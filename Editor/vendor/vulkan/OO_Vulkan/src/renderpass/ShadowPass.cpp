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
#include "ShadowPass.h"

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

#include "renderpass/DeferredCompositionRenderpass.h"

#include <array>

DECLARE_RENDERPASS(ShadowPass);

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

void ShadowPass::Draw()
{
	auto& vr = *VulkanRenderer::get();

	if (!vr.deferredRendering)
		return;

	auto& device = vr.m_device;
	auto& swapchain = vr.m_swapchain;
	auto& commandBuffers = vr.commandBuffers;
	auto& swapchainIdx = vr.swapchainIdx;
	auto* windowPtr = vr.windowPtr;

    const VkCommandBuffer cmdlist = commandBuffers[swapchainIdx];
    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("Shadow");

	constexpr VkClearColorValue zeroFloat4 = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear values for all attachments written in the fragment shader
	VkClearValue clearValues;
	clearValues.depthStencil = { 1.0f, 0 };
	
	VkFramebuffer fb_shadow;
	FramebufferBuilder::Begin(&vr.fbCache)
		.BindImage(&shadow_depth)
		.Build(fb_shadow,renderpass_Shadow);

	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass =  renderpass_Shadow.pass;
	renderPassBeginInfo.framebuffer = fb_shadow;
	renderPassBeginInfo.renderArea.extent.width = shadowmapSize.width;
	renderPassBeginInfo.renderArea.extent.height = shadowmapSize.height;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearValues;

	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	const float vpHeight = (float)shadowmapSize.height;
	const float vpWidth = (float)shadowmapSize.width;
	rhi::CommandList cmd{ cmdlist };
	cmd.BindPSO(pso_ShadowDefault);

	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
		vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	cmd.BindDescriptorSet(PSOLayoutDB::defaultPSOLayout, 0, 
		std::array<VkDescriptorSet, 3>
		{
			vr.descriptorSet_gpuscene,
			vr.descriptorSets_uniform[swapchainIdx],
			vr.descriptorSet_bindless
		},
		1, &dynamicOffset
	);

	// Bind merged mesh vertex & index buffers, instancing buffers.
	cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, vr.g_GlobalMeshBuffers.VtxBuffer.getBufferPtr());
	cmd.BindVertexBuffer(BIND_POINT_WEIGHTS_BUFFER_ID, 1, vr.skinningVertexBuffer.getBufferPtr());
	cmd.BindVertexBuffer(BIND_POINT_INSTANCE_BUFFER_ID, 1, &vr.instanceBuffer.buffer);
	cmd.BindIndexBuffer(vr.g_GlobalMeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	// calculate shadowmap grid dim
	float smGridDim = ceilf(sqrtf(static_cast<float>(vr.m_numShadowcastLights)));
	
	glm::vec2 increment{ vpWidth, vpHeight};
	if (smGridDim)
	{
		increment /= smGridDim;
	}

	if (vr.m_numShadowcastLights > 0)
	{
		int i = 0;
		int viewIter = 0;
		for (auto& light: vr.currWorld->GetAllOmniLightInstances())
		{
			++viewIter;
			// not a shadow casting light skip
			if (light.info.x < 1) continue;

			// this is an omnilight
			if (light.info.x == 1)
			{
				constexpr size_t cubeFaces = 6;
				for (size_t face = 0; face < cubeFaces; face++)
				{
					int lightGrid = static_cast<uint32_t>(light.info.y + face);
					// set custom viewport for each view
					int ly = static_cast<int>(lightGrid / smGridDim);
					int lx = static_cast<int>(lightGrid - (ly * smGridDim));
					vec2 customVP = increment * glm::vec2{ lx,smGridDim - ly };

					light.info.z = static_cast<int>(customVP.x); // this is actually wasted
					light.info.w = static_cast<int>(customVP.y); // this is actually wasted

					//cmd.SetViewport(VkViewport{ 0.0f, vpHeight, vpWidth, -vpHeight, 0.0f, 1.0f });
					//cmd.SetScissor(VkRect2D{ {0, 0}, {(uint32_t)vpWidth , (uint32_t)vpHeight } });

					// calculate viewport for each light
					cmd.SetViewport(VkViewport{ customVP.x, customVP.y,increment.x, -vpHeight +(vpHeight-increment.y), 0.0f, 1.0f });
					// TODO: Set exact region for scissor
					cmd.SetScissor(VkRect2D{ {0, 0}, {(uint32_t)vpHeight, (uint32_t)vpWidth } });

					//constexpr glm::vec3 up{ 0.0f,1.0f,0.0f };
					//constexpr glm::vec3 right{ 1.0f,0.0f,0.0f };
					//constexpr glm::vec3 forward{ 0.0f,0.0f,-1.0f };
					//
					//std::array<glm::vec3, 6> dirs{
					//	glm::vec3(light.position) + -up ,
					//	glm::vec3(light.position) + up,
					//	glm::vec3(light.position) + -right,
					//	glm::vec3(light.position) + right,
					//	glm::vec3(light.position) + -forward,
					//	glm::vec3(light.position) + forward,
					//};
					//DebugDraw::AddArrow(light.position, dirs[face], oGFX::Colors::RED);


					glm::mat4 mm(1.0f);
					mm = light.projection * light.view[face];
					vkCmdPushConstants(cmdlist,
						PSOLayoutDB::defaultPSOLayout,
						VK_SHADER_STAGE_ALL,	    // stage to push constants to
						0,							// offset of push constants to update
						sizeof(glm::mat4),			// size of data being pushed
						glm::value_ptr(mm));		// actualy data being pushed (could be an array));

					cmd.DrawIndexedIndirect(vr.shadowCasterCommandsBuffer.m_buffer, 0, static_cast<uint32_t>(vr.shadowCasterCommandsBuffer.size()));
				}
				++i;
			}

			
		}		
	}

	vkCmdEndRenderPass(cmdlist);
}

void ShadowPass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& device = vr.m_device.logicalDevice;

	shadow_depth.destroy();
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

	shadow_depth.name = "SHADOW_ATLAS";
	shadow_depth.forFrameBuffer(&m_device, vr.G_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height, false);
	
	vkutils::Texture2D tex;
	tex.image = shadow_depth.image;

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
	attachmentDescs.format = shadow_depth.format;


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

	VkImageView depthView = shadow_depth.view;

	//VkFramebuffer fb;
	//FramebufferBuilder::Begin(&vr.fbCache)
	//	.BindImage(&shadow_depth)
	//	.Build(fb,renderpass_Shadow);

	// TODO: Fix imgui depth rendering
	//deferredImg[GBufferAttachmentIndex::DEPTH]    = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_depth.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	shadowImg = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), shadow_depth.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
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
	pipelineCI.renderPass = renderpass_Shadow.pass;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	
	//std::array<VkPipelineColorBlendAttachmentState, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> blendAttachmentStates =
	//{
	//	oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
	//	oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
	//	oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
	//	oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	//};
	//
	//colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	//colorBlendState.pAttachments = blendAttachmentStates.data();

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_ShadowDefault));
	VK_NAME(m_device.logicalDevice, "ShadowPipline", pso_ShadowDefault);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);

}

