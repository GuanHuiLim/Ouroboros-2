/************************************************************************************//*!
\file           ShadowPass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a shadowpass

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "GfxRenderpass.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "VulkanTexture.h"
#include "FramebufferCache.h"
#include "FramebufferBuilder.h"
#include <iostream>
#include "DebugDraw.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"

#include <array>


struct ShadowPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(ShadowPass)

	void Init() override;
	void Draw(const VkCommandBuffer cmdlist) override;
	void Shutdown() override;

	bool SetupDependencies() override;
	void CreatePSO() override;


private:
	void SetupRenderpass();
	void SetupFramebuffer();
	void CreatePipeline();
};


DECLARE_RENDERPASS(ShadowPass);

VkExtent2D shadowmapSize = { 4096, 4096 };

VulkanRenderpass renderpass_Shadow{};

VkPipeline pso_ShadowDefault{};

void ShadowPass::Init()
{
	SetupRenderpass();
	SetupFramebuffer();
}

void ShadowPass::CreatePSO()
{
	CreatePipeline();
}

bool ShadowPass::SetupDependencies()
{
	// TODO: If shadows are disabled, return false.

	// READ: Scene data SSBO
	// READ: Instancing Data
	// WRITE: Shadow Depth Map
	// etc

	return true;
}

void ShadowPass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();

	lastCmd = cmdlist;
	if (!vr.deferredRendering)
		return;
	auto& device = vr.m_device;
	auto& swapchain = vr.m_swapchain;
	//auto& commandBuffers = vr.commandBuffers;
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;

    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("Shadow");

	constexpr VkClearColorValue zeroFloat4 = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear values for all attachments written in the fragment shader
	VkClearValue clearValues;
	clearValues.depthStencil = { 0.0f, 0 };
	
	const float vpHeight = (float)vr.attachments.shadow_depth.height;
	const float vpWidth = (float)vr.attachments.shadow_depth.width;
	rhi::CommandList cmd{ cmdlist, "Shadow Pass"};

	bool clearOnDraw = true;
	cmd.BindDepthAttachment(&vr.attachments.shadow_depth, clearOnDraw);

	cmd.BindPSO(pso_ShadowDefault, PSOLayoutDB::defaultPSOLayout);
	cmd.SetDefaultViewportAndScissor();

	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
		vr.m_device.properties.limits.minUniformBufferOffsetAlignment));

	cmd.DescriptorSetBegin(0)
		.BindSampler(0, GfxSamplerManager::GetDefaultSampler())
		.BindBuffer(1, vr.shadowCasterInstanceBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		.BindBuffer(3, vr.gpuShadowCasterTransformBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		.BindBuffer(4, vr.gpuBoneMatrixBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		.BindBuffer(5, vr.casterObjectInformationBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		.BindBuffer(6, vr.gpuSkinningWeightsBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	cmd.BindDescriptorSet(PSOLayoutDB::defaultPSOLayout, 1, 
		std::array<VkDescriptorSet, 2>
		{
			//vr.descriptorSet_gpuscene,
			vr.descriptorSets_uniform[currFrame],
			vr.descriptorSet_bindless
		},
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		1, &dynamicOffset
	);

	// Bind merged mesh vertex & index buffers, instancing buffers.
	cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, vr.g_GlobalMeshBuffers.VtxBuffer.getBufferPtr());
	//cmd.BindVertexBuffer(BIND_POINT_INSTANCE_BUFFER_ID, 1, vr.instanceBuffer.getBufferPtr());
	cmd.BindIndexBuffer(vr.g_GlobalMeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	// calculate shadowmap grid dim
	float smGridDim = ceilf(sqrtf(static_cast<float>(vr.m_numShadowcastLights)));
	
	glm::vec2 increment{ vpWidth, vpHeight};
	if (smGridDim)
	{
		increment /= smGridDim;
	}

	const auto& casterDatas = vr.batches.m_casterData;
	const auto& lights = vr.batches.GetShadowCasters();
	for (size_t i = 0; i < lights.size(); ++i)
	{
		auto& light = lights[i];
		OO_ASSERT(GetLightEnabled(light) == true);
		OO_ASSERT(GetCastsShadows(light) == true);

		// this is an omnilight
		if (light.info.x == 1)
		{
			// get the data for this light
			const GraphicsBatch::CastersData& casterData = casterDatas[i];
			constexpr size_t cubeFaces = 6;
			for (size_t face = 0; face < cubeFaces; face++)
			{
				int lightGrid = light.info.y + static_cast<int>(face);
				// set custom viewport for each view
				int ly = static_cast<int>(lightGrid / smGridDim);
				int lx = static_cast<int>(lightGrid - (ly * smGridDim));
				vec2 customVP = increment * glm::vec2{ lx,smGridDim - ly };

				//light.info.z = customVP.x; // this is actually wasted
				//light.info.w = customVP.y; // this is actually wasted

				//cmd.SetViewport(VkViewport{ 0.0f, vpHeight, vpWidth, -vpHeight, 0.0f, 1.0f });
				//cmd.SetScissor(VkRect2D{ {0, 0}, {(uint32_t)vpWidth , (uint32_t)vpHeight } });

				// calculate viewport for each light
				cmd.SetViewport(VkViewport{ customVP.x+1, customVP.y+1,increment.x-1, -(increment.y-1), 0.0f, 1.0f });
				// TODO: Set exact region for scissor
				cmd.SetScissor(VkRect2D{ {0, 0}, {(uint32_t)vpHeight, (uint32_t)vpWidth } });
			
				glm::mat4 mm(1.0f);
				mm = light.projection * light.view[face];
				cmd.SetPushConstant(PSOLayoutDB::defaultPSOLayout, sizeof(glm::mat4), glm::value_ptr(mm));					

				const std::vector<oGFX::IndirectCommand>& commands = casterData.m_commands[face];
				for (const oGFX::IndirectCommand& c : commands)
				{
					cmd.DrawIndexed(c.indexCount, c.instanceCount, c.firstIndex, c.vertexOffset, c.firstInstance);
				}
				//cmd.DrawIndexedIndirect(vr.shadowCasterCommandsBuffer.getBuffer(), 0, static_cast<uint32_t>(vr.shadowCasterCommandsBuffer.size()));

			}				
		}			
	}	
}

void ShadowPass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& device = vr.m_device.logicalDevice;

	vr.attachments.shadow_depth.destroy();
	renderpass_Shadow.destroy();
	vkDestroyPipeline(device, pso_ShadowDefault, nullptr);
}

void ShadowPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = shadowmapSize.width;
	const uint32_t height = shadowmapSize.height;

	vr.attachments.shadow_depth.name = "SHADOW_ATLAS";
	vr.attachments.shadow_depth.forFrameBuffer(&m_device, vr.G_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height, false);

	auto cmd = vr.GetCommandBuffer();
	vkutils::SetImageInitialState(cmd, vr.attachments.shadow_depth);
	vr.SubmitSingleCommandAndWait(cmd);

	// Set up separate renderpass with references to the color and depth attachments
	VkAttachmentDescription attachmentDescs = {};

	// Init attachment properties
	attachmentDescs.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescs.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//attachmentDescs.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	attachmentDescs.format = vr.attachments.shadow_depth.format;


	VkAttachmentReference depthReference = {};
	depthReference.attachment = 0;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = VK_NULL_HANDLE;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = &attachmentDescs;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	renderpass_Shadow.name = "ShadowPass";
	renderpass_Shadow.Init(m_device, renderPassInfo);
}

void ShadowPass::SetupFramebuffer()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	Attachments_imguiBinding::shadowImg = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), &vr.attachments.shadow_depth);
}

void ShadowPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	//VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, vr.G_DEPTH_COMPARISON);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::defaultPSOLayout, vr.renderPass_default.pass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();


	// Vertex input state from glTF model for pipeline rendering models
	//how the data for a single vertex (including infos such as pos, colour, texture, coords, normals etc) is as a whole
	auto& bindingDescription = oGFX::GetGFXVertexInputBindings();

	//how the data for an attirbute is define in the vertex
	auto& attributeDescriptions = oGFX::GetGFXVertexInputAttributes();

	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	// -- VERTEX INPUT -- 
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo(bindingDescription,attributeDescriptions);
	//vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	//vertexInputCreateInfo.vertexAttributeDescriptionCount = 5;

	vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

	pipelineCI.pVertexInputState = &vertexInputCreateInfo;

	// Offscreen pipeline
	shaderStages[0] = vr.LoadShader(m_device, "Shaders/bin/shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vr.LoadShader(m_device, "Shaders/bin/shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Separate render pass
	pipelineCI.renderPass = VK_NULL_HANDLE;

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = 0;
	renderingInfo.pColorAttachmentFormats = NULL;
	renderingInfo.depthAttachmentFormat = vr.G_DEPTH_FORMAT;
	renderingInfo.stencilAttachmentFormat =  vr.G_DEPTH_FORMAT;

	pipelineCI.pNext = &renderingInfo;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	
	if (pso_ShadowDefault != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_ShadowDefault, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_ShadowDefault));
	VK_NAME(m_device.logicalDevice, "ShadowPipline", pso_ShadowDefault);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);

}

