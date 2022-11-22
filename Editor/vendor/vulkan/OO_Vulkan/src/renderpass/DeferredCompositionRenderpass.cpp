/************************************************************************************//*!
\file           DeferredCompositionRenderpass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a deferred lighting composition pass

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "DeferredCompositionRenderpass.h"

#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanUtils.h"

#include "GBufferRenderPass.h"
#include "SSAORenderPass.h"
#include "ShadowPass.h"

#include <array>
#include <iostream>

DECLARE_RENDERPASS(DeferredCompositionRenderpass);

void DeferredCompositionRenderpass::Init()
{

}

void DeferredCompositionRenderpass::CreatePSO()
{
	CreateDescriptors();
	CreatePipelineLayout();
	CreatePipeline(); // Dependency on GBuffer Init()
}

bool DeferredCompositionRenderpass::SetupDependencies()
{
	// TODO: If shadows are disabled, return false.

	// READ: Lighting buffer (all the visible lights intersecting the camera frustum)
	// READ: GBuffer Albedo
	// READ: GBuffer Normal
	// READ: GBuffer MAterial
	// READ: GBuffer Depth
	// WRITE: Color Output
	// etc
	
	return true;
}

void DeferredCompositionRenderpass::Draw()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainIdx = vr.swapchainIdx;
	auto* windowPtr = vr.windowPtr;

    const VkCommandBuffer cmdlist = vr.commandBuffers[swapchainIdx];
    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("DeferredComposition");

	std::array<VkClearValue, 2> clearValues{};
	//clearValues[0].color = { 0.6f,0.65f,0.4f,1.0f };
	clearValues[0].color = { 0.1f,0.1f,0.1f,0.0f };
	clearValues[1].depthStencil.depth = { 1.0f };

	//Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = vr.renderPass_HDR.pass;                  //render pass to begin
	renderPassBeginInfo.renderArea.offset = { 0,0 };                                     //start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = vr.m_swapchain.swapChainExtent; //size of region to run render pass on (Starting from offset)
	renderPassBeginInfo.pClearValues = clearValues.data();                               //list of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	VkFramebuffer currentFB;
	FramebufferBuilder::Begin(&vr.fbCache)
		.BindImage(&vr.renderTargets[vr.renderTargetInUseID].texture)
		.BindImage(&vr.renderTargets[vr.renderTargetInUseID].depth)
		.Build(currentFB,vr.renderPass_HDR);
	renderPassBeginInfo.framebuffer = currentFB;

	// transition depth buffer
	auto gbuffer = RenderPassDatabase::GetRenderPass<GBufferRenderPass>();
	assert(gbuffer->attachments[GBufferAttachmentIndex::DEPTH].currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		gbuffer->attachments[GBufferAttachmentIndex::DEPTH].image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });


	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	rhi::CommandList cmd{ cmdlist };
	cmd.SetDefaultViewportAndScissor();

	const auto& info = vr.globalLightBuffer.GetDescriptorBufferInfo();
	DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[swapchainIdx])
		.BindBuffer(4, &info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(vr.descriptorSet_lights,SetLayoutDB::lights);

	CreateDescriptors();

	LightPC pc{};
	pc.useSSAO = vr.useSSAO ? 1 : 0;
	
	pc.numLights = static_cast<uint32_t>(vr.currWorld->GetAllOmniLightInstances().size());

	// calculate shadowmap grid dims
	float gridSize = ceilf(sqrtf(vr.m_numShadowcastLights));
	gridSize = std::max<float>(0, gridSize);
	pc.shadowMapGridDim = glm::vec2{gridSize,gridSize};

	pc.ambient = vr.currWorld->lightSettings.ambient;
	pc.maxBias = vr.currWorld->lightSettings.maxBias;
	pc.mulBias = vr.currWorld->lightSettings.biasMultiplier;
	
	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(LightPC);
	cmd.SetPushConstant(PSOLayoutDB::deferredLightingCompositionPSOLayout,range,&pc);

	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
		vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	cmd.BindDescriptorSet(PSOLayoutDB::deferredLightingCompositionPSOLayout, 0,
		std::array<VkDescriptorSet, 3>
		{
			vr.descriptorSet_DeferredComposition,
			vr.descriptorSets_uniform[swapchainIdx],
			vr.descriptorSet_lights,
		},
		1,&dynamicOffset
	);
	cmd.BindPSO(pso_DeferredLightingComposition);

	cmd.DrawFullScreenQuad();

	vkCmdEndRenderPass(cmdlist);

}

void DeferredCompositionRenderpass::Shutdown()
{
	auto& device = VulkanRenderer::get()->m_device.logicalDevice;

	vkDestroyPipelineLayout(device, PSOLayoutDB::deferredLightingCompositionPSOLayout, nullptr);
	vkDestroyPipeline(device, pso_DeferredLightingComposition, nullptr);
}

void DeferredCompositionRenderpass::CreateDescriptors()
{
	if (m_log)
	{
		std::cout << __FUNCSIG__ << std::endl;
	}

	auto& vr = *VulkanRenderer::get();
	// At this point, all dependent resources (gbuffer etc) must be ready.
	auto gbuffer = RenderPassDatabase::GetRenderPass<GBufferRenderPass>();
	assert(gbuffer != nullptr);

	auto ssao = RenderPassDatabase::GetRenderPass<SSAORenderPass>();
	assert(ssao != nullptr);

    // Image descriptors for the offscreen color attachments
    // VkDescriptorImageInfo texDescriptorPosition = oGFX::vkutils::inits::descriptorImageInfo(
    //     GfxSamplerManager::GetSampler_Deferred(),
	// 	gbuffer->attachments[GBufferAttachmentIndex::POSITION].view,
    //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorNormal = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
		gbuffer->attachments[GBufferAttachmentIndex::NORMAL]  .view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorAlbedo = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
		gbuffer->attachments[GBufferAttachmentIndex::ALBEDO]  .view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorMaterial = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
        gbuffer->attachments[GBufferAttachmentIndex::MATERIAL].view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorDepth = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
        gbuffer->attachments[GBufferAttachmentIndex::DEPTH]   .view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	auto& shadowTex = RenderPassDatabase::GetRenderPass<ShadowPass>()->shadow_depth;
	VkDescriptorImageInfo texDescriptorShadow = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_ShowMapClamp(),
		shadowTex.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	auto& ssaoTex = ssao->SSAO_finalTarget;
	VkDescriptorImageInfo texDescriptorSSAO = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		ssaoTex.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// TODO: Proper light buffer
	// TODO: How to handle shadow map sampling?
	const auto& dbi = vr.globalLightBuffer.GetDescriptorBufferInfo();
    DescriptorBuilder::Begin(&vr.DescLayoutCache,&vr.descAllocs[vr.swapchainIdx])
        //.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
        .BindImage(1, &texDescriptorDepth, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // we construct world position using depth
        .BindImage(2, &texDescriptorNormal, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .BindImage(3, &texDescriptorAlbedo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .BindImage(4, &texDescriptorMaterial, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        //.BindImage(5, &texDescriptorDepth, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .BindImage(5, &texDescriptorShadow, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) 
        .BindImage(6, &texDescriptorSSAO, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) 
        .BindBuffer(7, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build(vr.descriptorSet_DeferredComposition, SetLayoutDB::DeferredLightingComposition);
}

void DeferredCompositionRenderpass::CreatePipelineLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	std::vector<VkDescriptorSetLayout> setLayouts
	{
		SetLayoutDB::DeferredLightingComposition, // (set = 0)
		SetLayoutDB::FrameUniform, // (set = 1)
		SetLayoutDB::lights // (set = 4)
	};

	VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));
	VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
	plci.pushConstantRangeCount = 1;
	plci.pPushConstantRanges = &pushConstantRange;

	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::deferredLightingCompositionPSOLayout));
	VK_NAME(m_device.logicalDevice, "deferredLightingCompositionPSOLayout", PSOLayoutDB::deferredLightingCompositionPSOLayout);
}

void DeferredCompositionRenderpass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/deferredlighting.vert.spv";
	const char* shaderPS = "Shaders/bin/deferredlighting.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::defaultPSOLayout, vr.renderPass_HDR.pass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

	// Final fullscreen composition pass pipeline
	rasterizationState.cullMode = VK_CULL_MODE_NONE;

	// Empty vertex input state, vertices are generated by the vertex shader
	VkPipelineVertexInputStateCreateInfo emptyInputState = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo();
	pipelineCI.pVertexInputState = &emptyInputState;
	pipelineCI.renderPass = vr.renderPass_HDR.pass;
	pipelineCI.layout = PSOLayoutDB::deferredLightingCompositionPSOLayout;
	colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_DeferredLightingComposition));
	VK_NAME(m_device.logicalDevice, "deferredLightingCompositionPSO", pso_DeferredLightingComposition);

	vkDestroyShaderModule(m_device.logicalDevice,shaderStages[0].module , nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}
