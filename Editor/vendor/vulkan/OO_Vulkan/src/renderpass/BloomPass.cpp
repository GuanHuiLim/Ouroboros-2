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
	Bloom_brightTarget.forFrameBuffer(&vr.m_device, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, 1.0f);

	float renderScale = 0.5f;
	for (size_t i = 0; i < MAX_BLOOM_SAMPLES; i++)
	{
		// generate textures with half sizes
		Bloom_downsampleTargets[i].name = "bloom_down_" + std::to_string(i);
		Bloom_downsampleTargets[i].forFrameBuffer(&vr.m_device, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, true, renderScale);
		Bloom_upsampleTargets[i].name = "bloom_up_" + std::to_string(i);
		Bloom_upsampleTargets[i].forFrameBuffer(&vr.m_device, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, true, renderScale);

		renderScale /= 2.0f;
	}

	VkFramebufferCreateInfo blankInfo{};
	std::vector<VkImageView> dummyViews;
	std::vector<vkutils::Texture2D*> textures;

	textures.push_back(&Bloom_brightTarget);
	dummyViews.push_back(Bloom_brightTarget.view);
	for (size_t i = 0; i < MAX_BLOOM_SAMPLES; i++)
	{
		dummyViews.push_back(Bloom_downsampleTargets[i].view);
		textures.push_back(&Bloom_downsampleTargets[i]);
	}

	blankInfo.attachmentCount = dummyViews.size();
	blankInfo.pAttachments = dummyViews.data();
	// we add this to resize resource tracking
	vr.fbCache.CreateFramebuffer(&blankInfo, std::move(textures), textures.front()->targetSwapchain);

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
	
	auto& mainImage = vr.renderTargets[vr.renderTargetInUseID];
	auto lastLayout = mainImage.texture.currentLayout;

	glm::vec4 col = glm::vec4{ 1.0f,1.0f,1.0f,0.0f };
	auto regionBegin = VulkanRenderer::get()->pfnDebugMarkerRegionBegin;
	auto regionEnd = VulkanRenderer::get()->pfnDebugMarkerRegionEnd;
	
	VkDebugMarkerMarkerInfoEXT marker = {};
	marker.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
	memcpy(marker.color, &col[0], sizeof(float) * 4);
	
	

	{// bright threshold pass
		marker.pMarkerName = "BrightCOMP";
		if (regionBegin)
		{		
			regionBegin(cmdlist, &marker);
		}

		VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorderFloat(),
			mainImage.texture.view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::TransitionImage(cmdlist,mainImage.texture,VK_IMAGE_LAYOUT_GENERAL);

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

		BloomPC pc;
		auto knee = vr.currWorld->bloomSettings.threshold * vr.currWorld->bloomSettings.softThreshold;
		pc.threshold.x = vr.currWorld->bloomSettings.threshold;
		pc.threshold.y = pc.threshold.x - knee;
		pc.threshold.z = 2.0f * knee;
		pc.threshold.w = 0.25f / (knee + 0.00001f);
		//pc.threshold = vr.m_ShaderDebugValues.vector4_values0;

		vkCmdPushConstants(cmdlist, PSOLayoutDB::doubleImageStoreLayout, VK_SHADER_STAGE_ALL, 0, sizeof(BloomPC), &pc);
		vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::doubleImageStoreLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);

		vkCmdDispatch(cmdlist, (Bloom_brightTarget .width-1) / 16 + 1, (Bloom_brightTarget .height-1) / 16 + 1, 1);
		if (regionEnd)
		{
			regionEnd(cmdlist);
		}
	}
	
	{// downsample scope
		marker.pMarkerName = "DownsampleCOMP";
		if (regionBegin)
		{		
			regionBegin(cmdlist, &marker);
		}
		vkutils::Texture2D* prevImage = &Bloom_brightTarget;
		vkutils::Texture2D* currImage;
		//downsample
		vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_COMPUTE, pso_bloom_down);
		for (size_t i = 0; i < MAX_BLOOM_SAMPLES; i++)
		{
			currImage = &Bloom_downsampleTargets[i];
			if (prevImage->width / 2 != currImage->width || prevImage->height / 2 != currImage->height)
			{
				// what do i do here?
				//currImage->Resize(prevImage->width / 2, prevImage->height / 2);
				//std::cout << "HOW?\n"; 
			}

			VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
				GfxSamplerManager::GetSampler_BlackBorderFloat(),
				prevImage->view,
				VK_IMAGE_LAYOUT_GENERAL);
			vkutils::ComputeImageBarrier(cmdlist,*prevImage,VK_IMAGE_LAYOUT_GENERAL);

			VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
				GfxSamplerManager::GetSampler_Deferred(),  
				currImage  ->view,
				VK_IMAGE_LAYOUT_GENERAL);
			vkutils::ComputeImageBarrier(cmdlist,*currImage,VK_IMAGE_LAYOUT_GENERAL);
			DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
				//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
				.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
				.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);
			vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::BloomLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);
			vkCmdDispatch(cmdlist, (currImage ->width-1) / 16 + 1, (currImage ->height-1) / 16 + 1, 1);
			prevImage = currImage;
		} 
		if (regionEnd)
		{
			regionEnd(cmdlist);
		}
	}// end downsample scope
	

	//6 pass iterative upsamping 9tap tent
	marker.pMarkerName = "UpsampleCOMP";
	if (regionBegin)
	{		
		regionBegin(cmdlist, &marker);
	}
	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_COMPUTE, pso_bloom_up);
	for (int i = static_cast<int>(MAX_BLOOM_SAMPLES - 1ull); i > 0; --i)	
	{
		auto* outputBuffer = (&Bloom_downsampleTargets[i-1ull]);
		auto* inputBuffer = &Bloom_downsampleTargets[i];

		VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorderFloat(),
			inputBuffer->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*inputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_Deferred(),  
			outputBuffer  ->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*outputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
			//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
			.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
			.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);	
		
		vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::doubleImageStoreLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);
		vkCmdDispatch(cmdlist, (outputBuffer ->width-1) / 16 + 1, (outputBuffer ->height-1) / 16 + 1, 1);
	} 
	
		
	
	{ // we reuse the bright output to place the boom
		auto* outputBuffer = (&Bloom_brightTarget);
		auto* inputBuffer = &Bloom_downsampleTargets[0];

		VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorderFloat(),
			inputBuffer->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*inputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_Deferred(),  
			outputBuffer  ->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*outputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
			//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
			.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
			.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);	

		vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::doubleImageStoreLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);
		vkCmdDispatch(cmdlist, (outputBuffer ->width-1) / 16 + 1, (outputBuffer ->height-1) / 16 + 1, 1);
	}
	if (regionEnd)
	{
		regionEnd(cmdlist);
	}
	//carried over from last blit

	marker.pMarkerName = "AdditiveCOMP";
	if (regionBegin)
	{		
		regionBegin(cmdlist, &marker);
	}	
	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_COMPUTE, pso_additive_composite);
	{// composite online main buffer
		auto* outputBuffer = (&mainImage.texture);
		auto* inputBuffer = &Bloom_brightTarget;
	
		VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorderFloat(),
			inputBuffer->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*inputBuffer,VK_IMAGE_LAYOUT_GENERAL);
	
		VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_Deferred(),  
			outputBuffer  ->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*outputBuffer,VK_IMAGE_LAYOUT_GENERAL);
	
		DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
			//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
			.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
			.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);	
	
		vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::BloomLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);
		vkCmdDispatch(cmdlist, (outputBuffer ->width-1) / 16 + 1, (outputBuffer ->height-1) / 16 + 1, 1);
	}
	if (regionEnd)
	{
		regionEnd(cmdlist);
	}

	
	marker.pMarkerName = "TonemappingCOMP";
	if (regionBegin)
	{		
		regionBegin(cmdlist, &marker);
	}	
	// tone mapping 
	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_COMPUTE, pso_tone_mapping);
	{// composite online main buffer
		auto* outputBuffer = (&mainImage.texture);
		auto* inputBuffer = &Bloom_brightTarget;

		VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorder(),
			inputBuffer->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*inputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_Deferred(),  
			outputBuffer  ->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*outputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
			//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
			.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
			.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);	

		auto& colSettings = vr.currWorld->colourSettings;
		ColourCorrectPC pc;
		pc.threshold = glm::vec2{ colSettings.shadowThreshold ,colSettings.highlightThreshold };
		pc.shadowCol = colSettings.shadowColour;
		pc.midCol = colSettings.midtonesColour;
		pc.highCol = colSettings.highlightColour;

		pc.shadowCol.a /= 1000.0f;
		pc.midCol.a /= 1000.0f;
		pc.highCol.a /= 1000.0f;

		vkCmdPushConstants(cmdlist, PSOLayoutDB::BloomLayout, VK_SHADER_STAGE_ALL, 0, sizeof(ColourCorrectPC), &pc);

		vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::BloomLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);
		vkCmdDispatch(cmdlist, (outputBuffer ->width-1) / 16 + 1, (outputBuffer ->height-1) / 16 + 1, 1);
	}
	if (regionEnd)
	{
		regionEnd(cmdlist);
	}

	// FXAA 
	marker.pMarkerName = "FXAACOMP";
	if (regionBegin)
	{		
		regionBegin(cmdlist, &marker);
	}	
	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_COMPUTE, pso_fxaa);
	{// composite online main buffer
		auto* outputBuffer = &Bloom_brightTarget;
		auto* inputBuffer = (&mainImage.texture);

		VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorder(),
			inputBuffer->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*inputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_Deferred(),  
			outputBuffer  ->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist,*outputBuffer,VK_IMAGE_LAYOUT_GENERAL);

		DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
			//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
			.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
			.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);	

		vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::BloomLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);
		vkCmdDispatch(cmdlist, (outputBuffer ->width-1) / 16 + 1, (outputBuffer ->height-1) / 16 + 1, 1);
	}
	if (regionEnd)
	{
		regionEnd(cmdlist);
	}

	//  vigneette
	marker.pMarkerName = "VignetteCOMP";
	if (regionBegin)
	{		
		regionBegin(cmdlist, &marker);
	}	
	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_COMPUTE, pso_vignette);
	{// composite online main buffer
		auto* outputBuffer = (&mainImage.texture);
		auto* inputBuffer = &Bloom_brightTarget;

		VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_BlackBorder(),
			inputBuffer->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist, *inputBuffer, VK_IMAGE_LAYOUT_GENERAL);

		VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetSampler_Deferred(),
			outputBuffer->view,
			VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmdlist, *outputBuffer, VK_IMAGE_LAYOUT_GENERAL);

		DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
			//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
			.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
			.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(vr.descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);

		auto& vignette = vr.currWorld->vignetteSettings;
		VignettePC pc;
		pc.colour = vignette.colour;
		pc.vignetteValues = glm::vec4{vignette.innerRadius, vignette.outerRadius,0.0,0.0};


		vkCmdPushConstants(cmdlist, PSOLayoutDB::BloomLayout, VK_SHADER_STAGE_ALL, 0, sizeof(VignettePC), &pc);

		vkCmdBindDescriptorSets(cmdlist , VK_PIPELINE_BIND_POINT_COMPUTE, PSOLayoutDB::BloomLayout, 0, 1, &vr.descriptorSet_fullscreenBlit, 0, 0);
		vkCmdDispatch(cmdlist, (outputBuffer ->width-1) / 16 + 1, (outputBuffer ->height-1) / 16 + 1, 1);
	}
	if (regionEnd)
	{
		regionEnd(cmdlist);
	}


	vkutils::TransitionImage(cmdlist,mainImage.texture,lastLayout);

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
	vkDestroyPipelineLayout(device, PSOLayoutDB::doubleImageStoreLayout, nullptr);
	vkDestroyPipeline(device, pso_bloom_bright, nullptr);
	vkDestroyPipeline(device, pso_bloom_up, nullptr);
	vkDestroyPipeline(device, pso_bloom_down, nullptr);
	vkDestroyPipeline(device, pso_additive_composite, nullptr);
	vkDestroyPipeline(device, pso_tone_mapping, nullptr);
	vkDestroyPipeline(device, pso_vignette, nullptr);
	vkDestroyPipeline(device, pso_fxaa, nullptr);
}

void BloomPass::CreateDescriptors()
{

	auto& vr = *VulkanRenderer::get();
	auto& target = vr.renderTargets[vr.renderTargetInUseID].texture;
	// At this point, all dependent resources (gbuffer etc) must be ready.

	auto cmd = vr.beginSingleTimeCommands();
	VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_BlackBorder(),
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
		.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
		.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.Build(dummy, SetLayoutDB::compute_singleTexture);

	DescriptorBuilder::Begin(&vr.DescLayoutCache, &vr.descAllocs[vr.swapchainIdx])
		.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
		.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.Build(dummy, SetLayoutDB::compute_doubleImageStore);

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

		setLayouts[0] = SetLayoutDB::compute_doubleImageStore;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::doubleImageStoreLayout));
		VK_NAME(m_device.logicalDevice, "doubleImageStore_PSOLayout", PSOLayoutDB::doubleImageStoreLayout);

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

	const char* shaderCS = "Shaders/bin/BrightPixels.comp.spv";
	const char* shaderDownsample = "Shaders/bin/downsample.comp.spv";
	const char* shaderUpample = "Shaders/bin/upsample.comp.spv";
	const char* compositeAdditive = "Shaders/bin/additiveComposite.comp.spv";
	const char* toneMap = "Shaders/bin/tonemapping.comp.spv";
	const char* vignette = "Shaders/bin/vignette.comp.spv";
	const char* fxaa = "Shaders/bin/fxaa.comp.spv";

	VkComputePipelineCreateInfo computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::doubleImageStoreLayout);
	computeCI.stage = vr.LoadShader(m_device, shaderCS, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_bloom_bright));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::BloomLayout);
	computeCI.stage = vr.LoadShader(m_device, shaderDownsample, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_bloom_down));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::doubleImageStoreLayout);
	computeCI.stage = vr.LoadShader(m_device, shaderUpample, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_bloom_up));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::BloomLayout);
	computeCI.stage = vr.LoadShader(m_device, compositeAdditive, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_additive_composite));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	computeCI.stage = vr.LoadShader(m_device, toneMap, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_tone_mapping));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	computeCI.stage = vr.LoadShader(m_device, vignette, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_vignette));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	computeCI.stage = vr.LoadShader(m_device, fxaa, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_fxaa));
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute
	
}
