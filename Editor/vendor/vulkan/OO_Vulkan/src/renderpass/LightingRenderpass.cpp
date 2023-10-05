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
#include "GfxRenderpass.h"

#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanUtils.h"


#include <array>
#include <iostream>

struct LightingPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(DeferredCompositionRenderpass)

	void Init() override;
	void Draw(const VkCommandBuffer cmdlist) override;
	void Shutdown() override;

	bool SetupDependencies() override;
	void CreatePSO() override;

	void CreatePipeline();
private:
	void CreateResources();
	void CreateDescriptors();
	void CreatePipelineLayout();
};


DECLARE_RENDERPASS(LightingPass);

VkRenderPass renderpass_DeferredLightingComposition{};

VkPipeline pso_DeferredLightingComposition{};
VkPipeline pso_deferredBox{};

uint64_t uboDynamicAlignment{};

bool m_log{ false };

void LightingPass::Init()
{
	
	CreatePipelineLayout();

	CreateResources();
}

void LightingPass::CreatePSO()
{	
	CreatePipeline(); // Dependency on GBuffer Init()
}

bool LightingPass::SetupDependencies()
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

void LightingPass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;
	lastCmd = cmdlist;
    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("DeferredComposition");

	auto& attachments = vr.attachments.gbuffer;

	std::array<VkClearValue, 2> clearValues{};
	//clearValues[0].color = { 0.6f,0.65f,0.4f,1.0f };
	clearValues[0].color = { 0.1f,0.1f,0.1f,0.0f };
	clearValues[1].depthStencil.depth = { 1.0f };;

	auto tex = &vr.attachments.lighting_target; // layout undefined
	auto depth = &vr.renderTargets[vr.renderTargetInUseID].depth; // layout undefined
	rhi::CommandList cmd{ cmdlist, "Lighting Pass"};
	vkutils::ComputeImageBarrier(cmdlist, *depth, depth->referenceLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	vkutils::ComputeImageBarrier(cmdlist, attachments[GBufferAttachmentIndex::DEPTH],attachments[GBufferAttachmentIndex::DEPTH].referenceLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	VkImageCopy region{};
	region.srcSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT ,0,0,1 };
	region.srcOffset = {};
	region.dstSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT ,0,0,1 };
	region.dstOffset = {};
	region.extent = { depth->width,depth->height,1 };
	vkCmdCopyImage(cmdlist,attachments[GBufferAttachmentIndex::DEPTH].image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
		, depth->image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region);
	vkutils::ComputeImageBarrier(cmdlist, *depth, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, depth->referenceLayout);
	vkutils::ComputeImageBarrier(cmdlist, attachments[GBufferAttachmentIndex::DEPTH], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, attachments[GBufferAttachmentIndex::DEPTH].referenceLayout);

	vkutils::TransitionImage(cmdlist, *depth, depth->referenceLayout, depth->referenceLayout);
	vkutils::TransitionImage(cmdlist, attachments[GBufferAttachmentIndex::DEPTH], attachments[GBufferAttachmentIndex::DEPTH].referenceLayout, attachments[GBufferAttachmentIndex::DEPTH].referenceLayout);

	
	cmd.BindPSO(pso_DeferredLightingComposition, PSOLayoutDB::lightingPSOLayout);
	
	cmd.BindAttachment(0, tex);
	cmd.BindDepthAttachment(depth);

	cmd.DescriptorSetBegin(0)
		.BindSampler(0, GfxSamplerManager::GetDefaultSampler())
		.BindImage(1, &attachments[GBufferAttachmentIndex::DEPTH], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(2, &attachments[GBufferAttachmentIndex::NORMAL], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(3, &attachments[GBufferAttachmentIndex::ALBEDO], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(4, &attachments[GBufferAttachmentIndex::MATERIAL], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(5, &attachments[GBufferAttachmentIndex::EMISSIVE], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(6, &vr.attachments.shadow_depth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(7, &vr.attachments.SSAO_finalTarget, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindBuffer(8, &vr.globalLightBuffer[vr.getFrame()].GetDescriptorBufferInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		.BindSampler(9, GfxSamplerManager::GetSampler_Cube()) // cube sampler
		.BindImage(10, &vr.g_radianceMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) // cube map
		.BindImage(11, &vr.g_prefilterMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) // prefilter map
		.BindImage(12, &vr.g_brdfLUT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) // brdflut
		.BindSampler(13, GfxSamplerManager::GetSampler_ShowMapClamp()); // shadwosampler
	
	cmd.SetDefaultViewportAndScissor();

	const auto& info = vr.globalLightBuffer[currFrame].GetDescriptorBufferInfo();

	cmd.DescriptorSetBegin(2)
		.BindBuffer(4, &vr.globalLightBuffer[currFrame].GetDescriptorBufferInfo(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	//CreateDescriptors();

	LightPC pc{};
	pc.useSSAO = vr.useSSAO ? 1 : 0;
	pc.specularModifier = vr.currWorld->lightSettings.specularModifier;
	glm::vec3 normalizedDir = glm::normalize(vr.currWorld->lightSettings.directionalLight);
	pc.directionalLight = vec4{ normalizedDir, 0.0f };
	pc.lightColorInten = vr.currWorld->lightSettings.directionalLightColor;
	pc.resolution.x = (float)tex->width;
	pc.resolution.y = (float)tex->height;

	size_t lightCnt = 0;
	auto& lights = vr.batches.GetLocalLights();
	for(auto& l :lights) 
	{
		if (GetLightEnabled(l)== true)
		{
			++lightCnt;
		}
	}
	
	pc.numLights = static_cast<uint32_t>(lightCnt);

	// calculate shadowmap grid dims
	float gridSize = ceilf(sqrtf(static_cast<float>(vr.m_numShadowcastLights)));
	gridSize = std::max<float>(0, gridSize);
	pc.shadowMapGridDim = glm::vec2{gridSize,gridSize};

	pc.ambient = vr.currWorld->lightSettings.ambient;
	pc.maxBias = vr.currWorld->lightSettings.maxBias;
	pc.mulBias = vr.currWorld->lightSettings.biasMultiplier;
	
	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(LightPC);
	cmd.SetPushConstant(PSOLayoutDB::lightingPSOLayout,range,&pc);

	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
		vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	
	cmd.BindDescriptorSet(PSOLayoutDB::lightingPSOLayout, 1,
		std::array<VkDescriptorSet, 1>
		{
			vr.descriptorSets_uniform[currFrame],
		},
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		1,&dynamicOffset
	);
	
	cmd.DrawFullScreenQuad();

	const auto& cube = vr.g_globalModels[vr.GetDefaultCubeID()];
	cmd.BindPSO(pso_deferredBox, PSOLayoutDB::lightingPSOLayout);
	cmd.BindIndexBuffer(vr.g_GlobalMeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, vr.g_GlobalMeshBuffers.VtxBuffer.getBufferPtr());
	cmd.BindVertexBuffer(BIND_POINT_INSTANCE_BUFFER_ID, 1, vr.instanceBuffer[currFrame].getBufferPtr());

	cmd.DrawIndexed(cube.indicesCount, (uint32_t)lightCnt, cube.baseIndices, cube.baseVertex, 0);

}

void LightingPass::Shutdown()
{
	auto& device = VulkanRenderer::get()->m_device.logicalDevice;
	auto& vr = *VulkanRenderer::get();

	vr.attachments.lighting_target.destroy();

	vkDestroyPipelineLayout(device, PSOLayoutDB::lightingPSOLayout, nullptr);
	vkDestroyPipeline(device, pso_DeferredLightingComposition, nullptr);
	
	vkDestroyPipeline(device, pso_deferredBox, nullptr);

}

void LightingPass::CreateResources()
{

	auto& vr = *VulkanRenderer::get();
	auto swapchainext = vr.m_swapchain.swapChainExtent;
	vr.attachments.lighting_target.name = "lighting_buffer";
	vr.attachments.lighting_target.forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, 1.0f);
	vr.fbCache.RegisterFramebuffer(vr.attachments.lighting_target);

	auto cmd = vr.GetCommandBuffer();
	vkutils::SetImageInitialState(cmd, vr.attachments.lighting_target);
	vr.SubmitSingleCommandAndWait(cmd);

}

void LightingPass::CreateDescriptors()
{
	if (m_log)
	{
		std::cout << __FUNCSIG__ << std::endl;
	}

	auto& vr = *VulkanRenderer::get();
	// At this point, all dependent resources (gbuffer etc) must be ready.
	auto& attachments= vr.attachments.gbuffer;

    VkDescriptorImageInfo texDescriptorNormal = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
		attachments[GBufferAttachmentIndex::NORMAL]  .view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorAlbedo = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
		attachments[GBufferAttachmentIndex::ALBEDO]  .view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorMaterial = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
        attachments[GBufferAttachmentIndex::MATERIAL].view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorEmissive = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		attachments[GBufferAttachmentIndex::EMISSIVE].view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorDepth = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
        attachments[GBufferAttachmentIndex::DEPTH]   .view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	auto& shadowTex = vr.attachments.shadow_depth;
	auto& maskTex = vr.attachments.shadowMask;
	VkDescriptorImageInfo texDescriptorShadow = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_ShowMapClamp(),
		shadowTex.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	auto& ssaoTex = vr.attachments.SSAO_finalTarget;
	VkDescriptorImageInfo texDescriptorSSAO = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		ssaoTex.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); 

	VkDescriptorImageInfo sampler = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		VK_NULL_HANDLE,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// TODO: Proper light buffer
	// TODO: How to handle shadow map sampling?
	const auto& dbi = vr.globalLightBuffer[vr.getFrame()].GetDescriptorBufferInfo();
    DescriptorBuilder::Begin()
        .BindImage(0, &sampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BindImage(1, &texDescriptorDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS) // we construct world position using depth
        .BindImage(2, &texDescriptorNormal, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BindImage(3, &texDescriptorAlbedo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BindImage(4, &texDescriptorMaterial, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BindImage(5, &texDescriptorEmissive, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BindImage(6, &texDescriptorShadow, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BindImage(7, &texDescriptorSSAO, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BindBuffer(8, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .BuildLayout(SetLayoutDB::Lighting);
}

void LightingPass::CreatePipelineLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	
	VkDescriptorImageInfo dummy = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		VK_NULL_HANDLE,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
	const auto& dbi = vr.globalLightBuffer[vr.getFrame()].GetDescriptorBufferInfo();
	DescriptorBuilder::Begin()
		.BindImage(0, &dummy, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(1, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS) // we construct world position using depth
		.BindImage(2, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(3, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(4, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(5, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(6, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(7, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindBuffer(8, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(9, &dummy, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) // cube sampler
		.BindImage(10, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS) // cube map
		.BindImage(11, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,VK_SHADER_STAGE_ALL_GRAPHICS) // prefilter map
		.BindImage(12, &dummy, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,VK_SHADER_STAGE_ALL_GRAPHICS) // brdflut
		.BindImage(13, &dummy, VK_DESCRIPTOR_TYPE_SAMPLER,VK_SHADER_STAGE_ALL_GRAPHICS) // shadow sampler
		.BuildLayout(SetLayoutDB::Lighting);


	std::vector<VkDescriptorSetLayout> setLayouts
	{
		SetLayoutDB::Lighting, // (set = 0)
		SetLayoutDB::FrameUniform, // (set = 1)
		SetLayoutDB::lights // (set = 4)
	};

	VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));
	VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
	plci.pushConstantRangeCount = 1;
	plci.pPushConstantRanges = &pushConstantRange;

	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::lightingPSOLayout));
	VK_NAME(m_device.logicalDevice, "deferredLightingCompositionPSOLayout", PSOLayoutDB::lightingPSOLayout);
}

void LightingPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/genericFullscreen.vert.spv";
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
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, vr.G_DEPTH_COMPARISON);
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
    
    depthStencilState.depthTestEnable = VK_FALSE;

	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

	// Final fullscreen composition pass pipeline

	// Empty vertex input state, vertices are generated by the vertex shader
	VkPipelineVertexInputStateCreateInfo emptyInputState = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo();
	pipelineCI.pVertexInputState = &emptyInputState;
	// pipelineCI.renderPass = vr.renderPass_HDR.pass;
	pipelineCI.renderPass = VK_NULL_HANDLE;
	pipelineCI.layout = PSOLayoutDB::lightingPSOLayout;
	colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

	VkFormat colorFormat = vr.G_HDR_FORMAT;
	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &colorFormat;
	renderingInfo.depthAttachmentFormat = vr.G_DEPTH_FORMAT;
	renderingInfo.stencilAttachmentFormat = vr.G_DEPTH_FORMAT;

	depthStencilState.depthTestEnable = VK_FALSE;
	depthStencilState.stencilTestEnable = VK_TRUE;
	depthStencilState.front.compareOp = VK_COMPARE_OP_EQUAL;
	depthStencilState.front.passOp = VK_STENCIL_OP_REPLACE;
	depthStencilState.front.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.reference = 1;
	depthStencilState.front.compareMask = 0xff;
	depthStencilState.front.writeMask = 0x00;
	depthStencilState.back = depthStencilState.front;

	pipelineCI.pNext = &renderingInfo;
	if (pso_DeferredLightingComposition != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_DeferredLightingComposition, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_DeferredLightingComposition));
	VK_NAME(m_device.logicalDevice, "deferredLightingCompositionPSO", pso_DeferredLightingComposition);

	vkDestroyShaderModule(m_device.logicalDevice,shaderStages[0].module , nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);

	shaderStages[0] = vr.LoadShader(m_device,"Shaders/bin/deferredBoxLighting.vert.spv",VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vr.LoadShader(m_device,"Shaders/bin/deferredBoxLighting.frag.spv",VK_SHADER_STAGE_FRAGMENT_BIT);

	const auto& bindingDescription = oGFX::GetGFXVertexInputBindings();
	const auto& attributeDescriptions = oGFX::GetGFXVertexInputAttributes();
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo(bindingDescription, attributeDescriptions);
	pipelineCI.pVertexInputState = &vertexInputCreateInfo;
	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;

	VkPipelineColorBlendAttachmentState colourState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0x0000000F,VK_TRUE);
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;
	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1,&colourState);
	pipelineCI.pColorBlendState = &colourBlendingCreateInfo;

	depthStencilState.depthTestEnable = VK_FALSE;
	depthStencilState.stencilTestEnable = VK_TRUE;
	depthStencilState.front.compareOp = VK_COMPARE_OP_EQUAL;
	depthStencilState.front.passOp = VK_STENCIL_OP_REPLACE;
	depthStencilState.front.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.front.reference = 1;
	depthStencilState.front.compareMask = 0xff;
	depthStencilState.front.writeMask = 0x00;
	depthStencilState.back = depthStencilState.front;


	if (pso_deferredBox != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_deferredBox, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_deferredBox));
	VK_NAME(m_device.logicalDevice, "deferredBoxLights", pso_deferredBox);


	vkDestroyShaderModule(m_device.logicalDevice,shaderStages[0].module , nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);


}
