/************************************************************************************//*!
\file           GBufferRenderPass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a gbuffer pass

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
#include "FramebufferCache.h"
#include "FramebufferBuilder.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"

#include <array>

struct GBufferRenderPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(GBufferRenderPass)

	void Init() override;
	void Draw(const VkCommandBuffer cmdlist) override;
	void Shutdown() override;

	bool SetupDependencies() override;

	void CreatePSO() override;


private:
	void SetupRenderpass();
	void SetupFramebuffer();
	void CreatePipeline();
	void CreatePSOLayout();
	void SetupResources();

};

DECLARE_RENDERPASS(GBufferRenderPass);

VulkanRenderpass renderpass_GBuffer{};
VkFramebuffer framebuffer_GBuffer{};

//VkPushConstantRange pushConstantRange;
VkPipeline pso_GBufferDefault{};

VkPipeline pso_ComputeCull{};
VkPipeline pso_ComputeShadowPrepass{};

void GBufferRenderPass::Init()
{
	SetupResources();
	CreatePSOLayout();
	SetupRenderpass();
	SetupFramebuffer();
}

void GBufferRenderPass::CreatePSO()
{
	CreatePipeline();
}

bool GBufferRenderPass::SetupDependencies()
{
	// TODO: If gbuffer rendering sis disabled, return false.

	// READ: Scene data SSBO
	// READ: Instancing Data
	// READ: Bindless stuff
	// WRITE: GBuffer Albedo
	// WRITE: GBuffer Normal
	// WRITE: GBuffer Material
	// WRITE: GBuffer Depth
	// etc
	
	return true;
}

void GBufferRenderPass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();
	lastCmd = cmdlist;
	if (!vr.deferredRendering)
		return;
	auto& device = vr.m_device;
	auto& swapchain = vr.m_swapchain;
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;

    PROFILE_GPU_CONTEXT(cmdlist);

	rhi::CommandList cmd{ cmdlist, "CullCOMP" };
	{ // culling stage
		cmd.BindPSO(pso_ComputeCull, PSOLayoutDB::singleSSBOlayout, VK_PIPELINE_BIND_POINT_COMPUTE);

		cmd.DescriptorSetBegin(0)
			.BindBuffer(1, vr.indirectCommandsBuffer[currFrame].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rhi::UAV)
			.BindBuffer(2, vr.instanceBuffer[currFrame].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
			.BindBuffer(3, vr.gpuTransformBuffer[currFrame].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		
		oGFX::Frustum frust = vr.currWorld->cameras[vr.renderIteration].GetFrustum();
		struct CullingPC pc;
		pc. top = frust.top.normal;
		pc. bottom = frust.bottom.normal;
		pc. right = frust.right.normal;
		pc. left = frust.left.normal;
		pc. pFar = frust.planeFar.normal;
		pc. pNear = frust.planeNear.normal;
		pc.numItems = vr.indirectCommandsBuffer[currFrame].size();

		VkPushConstantRange pcr{};
		pcr.size = sizeof(CullingPC);
		pcr.stageFlags = VK_SHADER_STAGE_ALL;

		cmd.SetPushConstant(PSOLayoutDB::singleSSBOlayout, pcr, &pc);
		
		cmd.Dispatch((vr.indirectCommandsBuffer[currFrame].size() - 1) / 128 + 128);

	} // end culling stage

	PROFILE_GPU_EVENT("GBuffer");	
	cmd.BeginNameRegion("Gbuffer Pass");
	VK_NAME(vr.m_device.logicalDevice, "GBufferCmd", cmdlist);

	constexpr VkClearColorValue zeroFloat4 = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
	constexpr VkClearColorValue tangentNormal = VkClearColorValue{ 0.5f,0.5f,1.0f,0.0f };
	VkClearColorValue rMinusOne = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
	rMinusOne.int32[0] = -1;

	// Clear values for all attachments written in the fragment shader
	std::array<VkClearValue, GBufferAttachmentIndex::MAX_ATTACHMENTS> clearValues;
	//clearValues[GBufferAttachmentIndex::POSITION].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::NORMAL]  .color = tangentNormal;
	clearValues[GBufferAttachmentIndex::ALBEDO].color =	  zeroFloat4;
	clearValues[GBufferAttachmentIndex::MATERIAL].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::EMISSIVE].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::ENTITY_ID].color = rMinusOne;
	clearValues[GBufferAttachmentIndex::DEPTH]   .depthStencil = { 0.0f, 0 };

	auto& attachments = vr.attachments.gbuffer;
	
	const float vpHeight = (float)vr.m_swapchain.swapChainExtent.height;
	const float vpWidth = (float)vr.m_swapchain.swapChainExtent.width;

	constexpr bool clearOnDraw = true;
	for (size_t i = 0; i < GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS; i++)
	{
		if (i == GBufferAttachmentIndex::NORMAL) 
		{
			cmd.BindAttachment(i, &attachments[i],clearOnDraw);
		}
		else 
		{
			cmd.BindAttachment(i, &attachments[i]);
		}
		
	}
	
	cmd.BindDepthAttachment(&attachments[GBufferAttachmentIndex::DEPTH]);
	
	cmd.BindPSO(pso_GBufferDefault, PSOLayoutDB::defaultPSOLayout);
	cmd.SetDefaultViewportAndScissor();
	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
																												vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	
	cmd.BindDescriptorSet(PSOLayoutDB::defaultPSOLayout, 0, 
		std::array<VkDescriptorSet, 3>{
		vr.descriptorSet_gpuscene,
			vr.descriptorSets_uniform[currFrame],
			vr.descriptorSet_bindless,
	},
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		1, & dynamicOffset
	);

	// Bind merged mesh vertex & index buffers, instancing buffers.
	std::vector<VkBuffer> vtxBuffers{
		vr.g_GlobalMeshBuffers.VtxBuffer.getBuffer(),
	};

	VkDeviceSize offsets[2]{
		0,
		0
	};
	cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, vr.g_GlobalMeshBuffers.VtxBuffer.getBufferPtr());
	cmd.BindVertexBuffer(BIND_POINT_INSTANCE_BUFFER_ID, 1, vr.instanceBuffer[currFrame].getBufferPtr());
	cmd.BindIndexBuffer(vr.g_GlobalMeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

	cmd.DrawIndexedIndirect(vr.indirectCommandsBuffer[currFrame].getBuffer(), 0, vr.objectCount);

	

	if(0){
		// cmd.BeginNameRegion("ShadowMaskCOMP");
		// 
		// VkDescriptorImageInfo depthInput = oGFX::vkutils::inits::descriptorImageInfo(
		// 	GfxSamplerManager::GetSampler_Deferred(),
		// 	attachments[GBufferAttachmentIndex::DEPTH	].view,
		// 	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		// 
		// auto oldDepth = attachments[GBufferAttachmentIndex::DEPTH].currentLayout;
		// vkutils::TransitionImage(cmdlist,attachments[GBufferAttachmentIndex::DEPTH	],VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		// 
		// VkDescriptorImageInfo shadowinput = oGFX::vkutils::inits::descriptorImageInfo(
		// 	GfxSamplerManager::GetSampler_BlackBorder(),
		// 	vr.attachments.shadow_depth.view,
		// 	VK_IMAGE_LAYOUT_GENERAL);
		// auto oldShadow = vr.attachments.shadow_depth.currentLayout;
		// vkutils::TransitionImage(cmdlist, vr.attachments.shadow_depth,VK_IMAGE_LAYOUT_GENERAL);
		// 
		// VkDescriptorImageInfo normalInput = oGFX::vkutils::inits::descriptorImageInfo(
		// 	GfxSamplerManager::GetSampler_BlackBorder(),
		// 	attachments[GBufferAttachmentIndex::NORMAL	].view,
		// 	VK_IMAGE_LAYOUT_GENERAL);
		// auto oldNormal = attachments[GBufferAttachmentIndex::NORMAL].currentLayout;
		// vkutils::TransitionImage(cmdlist,attachments[GBufferAttachmentIndex::NORMAL	],VK_IMAGE_LAYOUT_GENERAL);
		// 
		// VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
		// 	GfxSamplerManager::GetSampler_Deferred(),
		// 	vr.attachments.shadowMask.view,
		// 	VK_IMAGE_LAYOUT_GENERAL);
		// vkutils::TransitionImage(cmdlist,vr.attachments.shadowMask,VK_IMAGE_LAYOUT_GENERAL);
		// 
		// VkDescriptorImageInfo basicSampler = oGFX::vkutils::inits::descriptorImageInfo(
		// 	GfxSamplerManager::GetSampler_Deferred(),
		// 	VK_NULL_HANDLE,
		// 	VK_IMAGE_LAYOUT_GENERAL);
		// 
		// vkutils::TransitionImage(cmdlist, vr.attachments.shadowMask, VK_IMAGE_LAYOUT_GENERAL);
		// const auto& dbi = vr.globalLightBuffer[currFrame].GetBufferInfoPtr();
		// 
		// cmd.BindPSO(pso_ComputeShadowPrepass, PSOLayoutDB::shadowPrepassPSOLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
		// 
		// cmd.DescriptorSetBegin(0)
		// 	.BindSampler(0, GfxSamplerManager::GetSampler_Deferred())
		// 	.BindImage(1, &attachments[GBufferAttachmentIndex::DEPTH], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		// 	.BindImage(2, &vr.attachments.shadow_depth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		// 	.BindImage(3, &attachments[GBufferAttachmentIndex::NORMAL], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		// 
		// 	.BindImage(6, &vr.attachments.shadowMask, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		// 	.BindBuffer(8, vr.globalLightBuffer[currFrame].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		// 	//.Build(shadowprepassDS, SetLayoutDB::compute_shadowPrepass);
		// 
		// uint32_t offset = 1;
		// cmd.BindDescriptorSet(PSOLayoutDB::shadowPrepassPSOLayout, offset,
		// 	std::array<VkDescriptorSet, 2>{
		// 		//shadowprepassDS,
		// 		vr.descriptorSets_uniform[currFrame],
		// 		vr.descriptorSet_bindless,
		// },
		// 	VK_PIPELINE_BIND_POINT_COMPUTE,
		// 	1, & dynamicOffset
		// 		);
		// 
		// LightPC pc{};
		// pc.useSSAO = vr.useSSAO ? 1 : 0;
		// pc.specularModifier = vr.currWorld->lightSettings.specularModifier;
		// 
		// size_t lightCnt = 0;
		// auto& lights = vr.batches.GetLocalLights();
		// for(auto& l :lights) 
		// {
		// 	if (GetLightEnabled(l)== true)
		// 	{
		// 		++lightCnt;
		// 	}
		// }
		// 
		// pc.numLights = static_cast<uint32_t>(lightCnt);
		// 
		// // calculate shadowmap grid dims
		// float gridSize = ceilf(sqrtf(static_cast<float>(vr.m_numShadowcastLights)));
		// gridSize = std::max<float>(0, gridSize);
		// pc.shadowMapGridDim = glm::vec2{gridSize,gridSize};
		// 
		// pc.ambient = vr.currWorld->lightSettings.ambient;
		// pc.maxBias = vr.currWorld->lightSettings.maxBias;
		// pc.mulBias = vr.currWorld->lightSettings.biasMultiplier;
		// 
		// VkPushConstantRange range;
		// range.offset = 0;
		// range.size = sizeof(LightPC);
		// //cmd.SetPushConstant(PSOLayoutDB::shadowPrepassLayout,range,&pc);
		// vkCmdPushConstants(cmdlist, PSOLayoutDB::shadowPrepassPSOLayout, VK_SHADER_STAGE_ALL,range.offset,range.size,&pc);
		// vkCmdDispatch(cmdlist, (vr.attachments.shadowMask.width - 1) / 16 + 1, (vr.attachments.shadowMask.height - 1) / 16 + 1, 1);

		

	}

}

void GBufferRenderPass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& device = m_device.logicalDevice;
	
	auto& attachments = vr.attachments.gbuffer;
	for (auto& att : attachments)
	{
		att.destroy();
	}

	vr.attachments.shadowMask.destroy();

	vkDestroyPipelineLayout(device, PSOLayoutDB::singleSSBOlayout, nullptr);
	vkDestroyPipeline(device, pso_ComputeCull, nullptr);

	renderpass_GBuffer.destroy();
	vkDestroyPipeline(device, pso_GBufferDefault, nullptr);

	vkDestroyPipeline(device, pso_ComputeShadowPrepass, nullptr);
	vkDestroyPipelineLayout(device, PSOLayoutDB::shadowPrepassPSOLayout, nullptr);
}

void GBufferRenderPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	
	// Set up separate renderpass with references to the color and depth attachments
	std::array<VkAttachmentDescription, GBufferAttachmentIndex::MAX_ATTACHMENTS> attachmentDescs = {};

	// Init attachment properties
	for (uint32_t i = 0; i < GBufferAttachmentIndex::MAX_ATTACHMENTS; ++i)
	{
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (i == GBufferAttachmentIndex::DEPTH)
		{
			attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	auto& attachments = vr.attachments.gbuffer;
	// Formats
	//attachmentDescs[GBufferAttachmentIndex::POSITION].format = attachments[GBufferAttachmentIndex::POSITION].format;
	attachmentDescs[GBufferAttachmentIndex::NORMAL]  .format = attachments[GBufferAttachmentIndex::NORMAL]  .format;
	attachmentDescs[GBufferAttachmentIndex::ALBEDO]  .format = attachments[GBufferAttachmentIndex::ALBEDO]  .format;
	attachmentDescs[GBufferAttachmentIndex::MATERIAL].format = attachments[GBufferAttachmentIndex::MATERIAL].format;
	attachmentDescs[GBufferAttachmentIndex::EMISSIVE].format = attachments[GBufferAttachmentIndex::EMISSIVE].format;
	attachmentDescs[GBufferAttachmentIndex::ENTITY_ID].format = attachments[GBufferAttachmentIndex::ENTITY_ID].format;
	attachmentDescs[GBufferAttachmentIndex::DEPTH]   .format = attachments[GBufferAttachmentIndex::DEPTH]   .format;
	

	std::vector<VkAttachmentReference> colorReferences;
	//colorReferences.push_back({ GBufferAttachmentIndex::POSITION, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::NORMAL,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::ALBEDO,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::MATERIAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::EMISSIVE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::ENTITY_ID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = GBufferAttachmentIndex::DEPTH;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
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
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	renderpass_GBuffer.name = "deferredPass";
	renderpass_GBuffer.Init(m_device, renderPassInfo);
}

void GBufferRenderPass::SetupFramebuffer()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;
	
	// deferredImg[GBufferAttachmentIndex::NORMAL]   = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), attachments[GBufferAttachmentIndex::NORMAL  ].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	// deferredImg[GBufferAttachmentIndex::ALBEDO]   = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), attachments[GBufferAttachmentIndex::ALBEDO  ].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	// deferredImg[GBufferAttachmentIndex::MATERIAL] = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), attachments[GBufferAttachmentIndex::MATERIAL].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	//deferredImg[GBufferAttachmentIndex::DEPTH]    = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_depth.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}


void GBufferRenderPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/gbuffer.vert.spv";
	const char* shaderPS = "Shaders/bin/gbuffer.frag.spv";
	const char* compute = "Shaders/bin/computeCull.comp.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages =
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, vr.G_DEPTH_COMPARISON);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

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

	depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
	depthStencilState.depthWriteEnable = VK_FALSE;

	// write to stencil buffer
	depthStencilState.stencilTestEnable = VK_TRUE;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilState.back.failOp = VK_STENCIL_OP_REPLACE;
	depthStencilState.back.depthFailOp = VK_STENCIL_OP_REPLACE;
	depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;
	depthStencilState.back.compareMask = 0xff;
	depthStencilState.back.writeMask = 0xff;
	depthStencilState.back.reference = 1;
	depthStencilState.front = depthStencilState.back;

	const auto& bindingDescription = oGFX::GetGFXVertexInputBindings();
	const auto& attributeDescriptions = oGFX::GetGFXVertexInputAttributes();
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo(bindingDescription, attributeDescriptions);
	pipelineCI.pVertexInputState = &vertexInputCreateInfo;


	// Separate render pass
	pipelineCI.renderPass = renderpass_GBuffer.pass;
	pipelineCI.renderPass = NULL;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> blendAttachmentStates =
	{
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};

	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	std::vector<VkFormat> colourFormats;
	colourFormats.resize(GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS);
	for (size_t i = 0; i < GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS; i++)
	{
		colourFormats[i] = vr.attachments.gbuffer[i].format;
	}

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS;
	renderingInfo.pColorAttachmentFormats = colourFormats.data();
	renderingInfo.depthAttachmentFormat = vr.G_DEPTH_FORMAT;
	renderingInfo.stencilAttachmentFormat =  vr.G_DEPTH_FORMAT;
	
	pipelineCI.pNext = &renderingInfo;

	if (pso_GBufferDefault != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_GBufferDefault, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_GBufferDefault));
	VK_NAME(m_device.logicalDevice, "GBufferDefaultPSO", pso_GBufferDefault);

	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);

	if (pso_ComputeCull != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_ComputeCull, nullptr);
	}
	VkComputePipelineCreateInfo computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::singleSSBOlayout);
	computeCI.stage = vr.LoadShader(m_device, compute, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_ComputeCull));
	VK_NAME(m_device.logicalDevice, "pso_ComputeCull", pso_ComputeCull);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute


	{// shadow prepass moveout one day

		const char* shaderCS = "Shaders/bin/shadowPrepass.comp.spv";
		if (pso_ComputeShadowPrepass != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_device.logicalDevice, pso_ComputeShadowPrepass, nullptr);
		}
		VkComputePipelineCreateInfo computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::shadowPrepassPSOLayout);
		computeCI.stage = vr.LoadShader(m_device, shaderCS, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_ComputeShadowPrepass));
		VK_NAME(m_device.logicalDevice, "pso_ComputeShadowPrepass", pso_ComputeShadowPrepass);
		vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute
	}

}

void GBufferRenderPass::CreatePSOLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	DescriptorBuilder::Begin()
		.BindBuffer(1, vr.indirectCommandsBuffer[0].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) 
		.BindBuffer(2, vr.instanceBuffer[0].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) 
		.BindBuffer(3, vr.gpuTransformBuffer[0].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT) 
		.BuildLayout(SetLayoutDB::compute_singleSSBO);

	{
		std::vector<VkDescriptorSetLayout> setLayouts
		{
			SetLayoutDB::compute_singleSSBO, // (set = 0)
		};

		VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(
			setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

		VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &pushConstantRange;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::singleSSBOlayout));
		VK_NAME(m_device.logicalDevice, "SingleSSBO_PSOLayout", PSOLayoutDB::singleSSBOlayout);
	}	


	{

		auto& attachments = vr.attachments.gbuffer;
		VkDescriptorImageInfo depthInput = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorder(),
			attachments[GBufferAttachmentIndex::DEPTH	].view,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkDescriptorImageInfo shadowinput = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorder(),
			vr.attachments.shadow_depth.view,
			VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo normalInput = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorder(),
			attachments[GBufferAttachmentIndex::NORMAL	].view,
			VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_Deferred(),
			vr.attachments.shadowMask.view,
			VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo sampler = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetDefaultSampler(),
			VK_NULL_HANDLE,
			VK_IMAGE_LAYOUT_GENERAL);
		
		VkDescriptorSet dummy;	
		const auto& dbi = vr.globalLightBuffer[0].GetDescriptorBufferInfo();
		DescriptorBuilder::Begin()
			.BindImage(0, &sampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindImage(1, &depthInput, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindImage(2, &shadowinput, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindImage(3, &normalInput, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

			.BindImage(6, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

			.BindBuffer(8, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.BuildLayout(SetLayoutDB::compute_shadowPrepass);

		std::array<VkDescriptorSetLayout,4> descriptorSetLayouts = 
		{
			SetLayoutDB::compute_shadowPrepass, // (set = 0)
			SetLayoutDB::FrameUniform,  // (set = 1)
			SetLayoutDB::bindless,  // (set = 2)
			SetLayoutDB::lights, // (set = 3)
		};

		VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(
			descriptorSetLayouts.data(), static_cast<uint32_t>(descriptorSetLayouts.size()));

		VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &pushConstantRange;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::shadowPrepassPSOLayout));
		VK_NAME(m_device.logicalDevice, "ShadowPrepass_PSOLayout", PSOLayoutDB::shadowPrepassPSOLayout);

	}

	
}

void GBufferRenderPass::SetupResources() {

	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;
	printf("%s\n", __FUNCTION__);
	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	auto& attachments = vr.attachments.gbuffer;
	attachments[GBufferAttachmentIndex::NORMAL	].name = "GB_Normal";
	attachments[GBufferAttachmentIndex::NORMAL	].forFrameBuffer(&m_device, vr.G_NORMALS_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	vr.fbCache.RegisterFramebuffer(attachments[GBufferAttachmentIndex::NORMAL]);
	// linear texture
	attachments[GBufferAttachmentIndex::ALBEDO	].name = "GB_Albedo";
	attachments[GBufferAttachmentIndex::ALBEDO	].forFrameBuffer(&m_device, vr.G_NON_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	vr.fbCache.RegisterFramebuffer(attachments[GBufferAttachmentIndex::ALBEDO]);
	attachments[GBufferAttachmentIndex::MATERIAL].name = "GB_Material";
	attachments[GBufferAttachmentIndex::MATERIAL].forFrameBuffer(&m_device, vr.G_NON_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	vr.fbCache.RegisterFramebuffer(attachments[GBufferAttachmentIndex::MATERIAL]);
	attachments[GBufferAttachmentIndex::ENTITY_ID].name = "GB_Entity";
	attachments[GBufferAttachmentIndex::ENTITY_ID].forFrameBuffer(&m_device, VK_FORMAT_R32_SINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	vr.fbCache.RegisterFramebuffer(attachments[GBufferAttachmentIndex::ENTITY_ID]);
	attachments[GBufferAttachmentIndex::DEPTH	].name = "GB_DEPTH";
	attachments[GBufferAttachmentIndex::DEPTH	].forFrameBuffer(&m_device, vr.G_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);
	vr.fbCache.RegisterFramebuffer(attachments[GBufferAttachmentIndex::DEPTH]);
	attachments[GBufferAttachmentIndex::EMISSIVE	].name = "GB_Emissive";
	attachments[GBufferAttachmentIndex::EMISSIVE	].forFrameBuffer(&m_device, vr.G_HDR_FORMAT_ALPHA, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	vr.fbCache.RegisterFramebuffer(attachments[GBufferAttachmentIndex::EMISSIVE]);
	
	vr.attachments.shadowMask.name = "GB_ShadowMask";
	vr.attachments.shadowMask.forFrameBuffer(&m_device, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	vr.fbCache.RegisterFramebuffer(vr.attachments.shadowMask);

	auto cmd = vr.GetCommandBuffer();
	for (size_t i = 0; i < GBufferAttachmentIndex::MAX_ATTACHMENTS; i++)
	{
		vkutils::SetImageInitialState(cmd, attachments[i]);
	}		
	vkutils::SetImageInitialState(cmd, vr.attachments.shadowMask);

	vr.SubmitSingleCommandAndWait(cmd);

}