/************************************************************************************//*!
\file           BloomPass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date            Nov 8, 2022
\brief              Defines a SSAO pass

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "BloomPass.h"

#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanUtils.h"

#include "DeferredCompositionRenderpass.h"
#include "ShadowPass.h"

#include <array>
#include <random>

DECLARE_RENDERPASS(BloomPass);

void BloomPass::Init()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainext = vr.m_swapchain.swapChainExtent;
	Bloom_brightTarget.name = "bloom_bright";
	Bloom_brightTarget.forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, 1.0f);

	float renderScale = 1.0f;
	for (size_t i = 0; i < MAX_BLOOM_SAMPLES; i++)
	{
		// generate textures with half sizes
		Bloom_downsampleTargets[i].name = "bloom_down_" + std::to_string(i);
		Bloom_downsampleTargets[i].forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, true, renderScale);
		Bloom_upsampleTargets[i].name = "bloom_up_" + std::to_string(i);
		Bloom_upsampleTargets[i].forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, true, renderScale);

		renderScale /= 2.0f;
	}


	SetupRenderpass();

}

void BloomPass::CreatePSO()
{
	CreateDescriptors();
	CreatePipelineLayout();
	CreatePipeline(); // Dependency on GBuffer Init()
}

bool BloomPass::SetupDependencies()
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

void BloomPass::Draw()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainIdx = vr.swapchainIdx;
	auto* windowPtr = vr.windowPtr;

	const VkCommandBuffer cmdlist = vr.commandBuffers[swapchainIdx];
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("Bloom");
	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_COMPUTE, pso_bloom_bright);
	
	auto& target = vr.renderTargets[vr.renderTargetInUseID];
	auto lastLayout = target.texture.currentLayout;

	VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_SSAOEdgeClamp(),
		target.texture.view,
		VK_IMAGE_LAYOUT_GENERAL);
	vkutils::TransitionImage(cmdlist,target.texture,VK_IMAGE_LAYOUT_GENERAL);

	VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		Bloom_brightTarget  .view,
		VK_IMAGE_LAYOUT_GENERAL);
	vkutils::TransitionImage(cmdlist,Bloom_brightTarget,VK_IMAGE_LAYOUT_GENERAL);


	DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
		//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
		.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
		.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);

	vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::BloomLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);

	vkCmdDispatch(cmdlist, (Bloom_brightTarget .width-1) / 16 + 1, (Bloom_brightTarget .height-1) / 16 + 1, 1);
	vkutils::TransitionImage(cmdlist,target.texture,lastLayout);

	//rhi::CommandList cmd{ cmdlist };
	//cmd.BindPSO(pso_SSAO);
}

void BloomPass::Shutdown()
{
	auto& device = VulkanRenderer::get()->m_device.logicalDevice;

	
	Bloom_brightTarget.destroy();
	for (size_t i = 0; i < MAX_BLOOM_SAMPLES; i++)
	{
		// destroy
		Bloom_downsampleTargets[i].destroy();
		Bloom_upsampleTargets[i].destroy();
	}
	vkDestroyPipelineLayout(device, PSOLayoutDB::BloomLayout, nullptr);
	vkDestroyPipeline(device, pso_bloom_bright, nullptr);
}

void BloomPass::CreateDescriptors()
{

	auto& vr = *VulkanRenderer::get();
	auto& target = vr.renderTargets[vr.renderTargetInUseID].texture;
	// At this point, all dependent resources (gbuffer etc) must be ready.

	auto cmd = vr.beginSingleTimeCommands();
	VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_SSAOEdgeClamp(),
		Bloom_brightTarget.view,
		VK_IMAGE_LAYOUT_GENERAL);
	vkutils::TransitionImage(cmd,Bloom_brightTarget,VK_IMAGE_LAYOUT_GENERAL);

	VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		Bloom_downsampleTargets[0]  .view,
		VK_IMAGE_LAYOUT_GENERAL);
	vkutils::TransitionImage(cmd,Bloom_downsampleTargets[0],VK_IMAGE_LAYOUT_GENERAL);
	vr.endSingleTimeCommands(cmd);
	VkDescriptorSet dummy;
	DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
		//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
		.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
		.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.Build(dummy, SetLayoutDB::compute_singleTexture);

}

void BloomPass::CreatePipelineLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	{
		std::vector<VkDescriptorSetLayout> setLayouts
		{
			SetLayoutDB::compute_singleTexture, // (set = 0)
		};

		VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(
								setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

		VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &pushConstantRange;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::BloomLayout));
		VK_NAME(m_device.logicalDevice, "Bloom_PSOLayout", PSOLayoutDB::BloomLayout);
	}
}

void BloomPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	

}

void BloomPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/genericFullscreen.vert.spv";
	const char* shaderPS = "Shaders/bin/Bloom.frag.spv";
	const char* shaderCS = "Shaders/bin/BrightPixels.comp.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages
	{
		//vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		//vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkComputePipelineCreateInfo computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::BloomLayout);
	computeCI.stage = vr.LoadShader(m_device, shaderCS, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_bloom_bright));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	// VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	// VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	// VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT , VK_FALSE);
	// VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	// VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	// VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	// VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	// std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	// VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	// 
	// VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::SSAOPSOLayout, renderpass_SSAO.pass);
	// pipelineCI.pInputAssemblyState = &inputAssemblyState;
	// pipelineCI.pRasterizationState = &rasterizationState;
	// pipelineCI.pColorBlendState = &colorBlendState;
	// pipelineCI.pMultisampleState = &multisampleState;
	// pipelineCI.pViewportState = &viewportState;
	// pipelineCI.pDepthStencilState = &depthStencilState;
	// pipelineCI.pDynamicState = &dynamicState;
	// pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	// pipelineCI.pStages = shaderStages.data();
	// 
	// // Empty vertex input state, vertices are generated by the vertex shader
	// VkPipelineVertexInputStateCreateInfo emptyInputState = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo();
	// pipelineCI.pVertexInputState = &emptyInputState;
	// pipelineCI.renderPass = renderpass_SSAO.pass;
	// pipelineCI.layout = PSOLayoutDB::SSAOPSOLayout;
	// colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	// blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	// 
	// VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_SSAO));
	// VK_NAME(m_device.logicalDevice, "SSAO_PSO", pso_SSAO);
	// vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr); // destroy fragment
	// 
	// 
	// shaderStages[1] = vr.LoadShader(m_device, "Shaders/bin/ssaoBlur.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	// pipelineCI.layout = PSOLayoutDB::SSAOBlurLayout;
	// VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_SSAO_blur));
	// VK_NAME(m_device.logicalDevice, "SSAO_PSO_blur", pso_SSAO_blur);

	// vkDestroyShaderModule(m_device.logicalDevice,shaderStages[0].module , nullptr);
	// vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr); // destroy fragment
}
