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
#include "GfxRenderpass.h"

#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanUtils.h"

#include <array>
#include <random>

struct BloomPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(BloomPass)

	void Init() override;
	void Draw(const VkCommandBuffer cmdlist) override;
	void Shutdown() override;

	bool SetupDependencies() override;

	void CreatePSO() override;
	void CreatePipelineLayout();
	void CreateDescriptors();

private:

	vkutils::Texture2D* PerformBloom(rhi::CommandList& cmd, vkutils::Texture2D* target);
	void SetupRenderpass();
	void CreatePipeline();

};

DECLARE_RENDERPASS(BloomPass);

VulkanRenderpass renderpass_bright{};
VulkanRenderpass renderpass_bloomDownsample{};
VulkanRenderpass renderpass_bloomUpsample{};

//VkPushConstantRange pushConstantRange;
VkPipeline pso_bloom_bright{};
VkPipeline pso_bloom_down{};
VkPipeline pso_bloom_up{};
VkPipeline pso_additive_composite{};
VkPipeline pso_tone_mapping{};
VkPipeline pso_vignette{};
VkPipeline pso_fxaa{};

void BloomPass::Init()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainext = vr.m_swapchain.swapChainExtent;
	vr.attachments.Bloom_brightTarget.name = "bloom_bright";
	vr.attachments.Bloom_brightTarget.forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT_ALPHA, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, false, 1.0f);
	vr.fbCache.RegisterFramebuffer(vr.attachments.Bloom_brightTarget);
	float renderScale = 0.5f;
	for (size_t i = 0; i < vr.attachments.MAX_BLOOM_SAMPLES; i++)
	{
		// generate textures with half sizes
		vr.attachments.Bloom_downsampleTargets[i].name = "bloom_down_" + std::to_string(i);
		vr.attachments.Bloom_downsampleTargets[i].forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT_ALPHA, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, false, renderScale);
		vr.fbCache.RegisterFramebuffer(vr.attachments.Bloom_downsampleTargets[i]);

		renderScale /= 2.0f;
	}

	vr.attachments.SD_target[0].name = "SD_Target0";
	vr.attachments.SD_target[0].forFrameBuffer(&vr.m_device, vr.G_NON_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, 1.0f);
	vr.fbCache.RegisterFramebuffer(vr.attachments.SD_target[0]);

	vr.attachments.SD_target[1].name = "SD_Target1";
	vr.attachments.SD_target[1].forFrameBuffer(&vr.m_device, vr.G_NON_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, 1.0f);
	vr.fbCache.RegisterFramebuffer(vr.attachments.SD_target[1]);

	VkFramebufferCreateInfo blankInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	std::vector<VkImageView> dummyViews;
	std::vector<vkutils::Texture2D*> textures;

	textures.push_back(&vr.attachments.Bloom_brightTarget);
	dummyViews.push_back(vr.attachments.Bloom_brightTarget.view);
	for (size_t i = 0; i < vr.attachments.MAX_BLOOM_SAMPLES; i++)
	{
		dummyViews.push_back(vr.attachments.Bloom_downsampleTargets[i].view);
		textures.push_back(&vr.attachments.Bloom_downsampleTargets[i]);
	}

	blankInfo.attachmentCount = (uint32_t)dummyViews.size();
	blankInfo.pAttachments = dummyViews.data();
	// we add this to resize resource tracking
	const bool resourceTrackonly = true;
	vr.fbCache.CreateFramebuffer(&blankInfo, std::move(textures), textures.front()->useRenderscale, resourceTrackonly);

	auto cmd = vr.GetCommandBuffer();

	vkutils::SetImageInitialState(cmd, vr.attachments.Bloom_brightTarget);
	for (size_t i = 0; i < vr.attachments.MAX_BLOOM_SAMPLES; i++)
	{
		vkutils::SetImageInitialState(cmd, vr.attachments.Bloom_downsampleTargets[i]);
	}
	vkutils::SetImageInitialState(cmd, vr.attachments.SD_target[0]);
	vkutils::SetImageInitialState(cmd, vr.attachments.SD_target[1]);

	vr.SubmitSingleCommandAndWait(cmd);
	
	SetupRenderpass();

}

void BloomPass::CreatePSO()
{
	
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

void BloomPass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;
	lastCmd = cmdlist;
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("Bloom");
	rhi::CommandList cmd{ cmdlist, "Bloom"};
	cmd.BindPSO(pso_bloom_bright, PSOLayoutDB::doubleImageStoreLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
	
	vkutils::Texture2D* mainImage;
	if (vr.m_upscaleType == UPSCALING_TYPE::NONE) {
		mainImage = &vr.attachments.lighting_target;
	}
	else {
		mainImage = &vr.attachments.fullres_HDR;
	}

	glm::vec4 col = glm::vec4{ 1.0f,1.0f,1.0f,0.0f };
	auto regionBegin = VulkanRenderer::get()->pfnDebugMarkerRegionBegin;
	auto regionEnd = VulkanRenderer::get()->pfnDebugMarkerRegionEnd;
	
	VkDebugMarkerMarkerInfoEXT marker = {};
	marker.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
	memcpy(marker.color, &col[0], sizeof(float) * 4);	

	vkutils::Texture2D* previousBuffer{ mainImage };

	if (vr.currWorld->bloomSettings.enabled == true)
		previousBuffer = PerformBloom(cmd, mainImage);
	
	marker.pMarkerName = "TonemappingCOMP";
	if (regionBegin)
	{		
		regionBegin(cmdlist, &marker);
	}	
	// tone mapping 
	{// composite online main buffer
		cmd.BindPSO(pso_tone_mapping, PSOLayoutDB::tonemapPSOLayout,VK_PIPELINE_BIND_POINT_COMPUTE);
		vkutils::Texture2D * outputBuffer = (&vr.renderTargets[0].texture);
		vkutils::Texture2D * inputBuffer = previousBuffer;

		VkDescriptorBufferInfo dbi{};
		dbi.buffer = vr.LuminanceBuffer.buffer;
		dbi.range = VK_WHOLE_SIZE;

		cmd.DescriptorSetBegin(0)
			.BindSampler(0, GfxSamplerManager::GetSampler_BlackBorder())
			.BindImage(1, inputBuffer, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
			.BindImage(2, outputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			.BindBuffer(3, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		auto& colSettings = vr.currWorld->colourSettings;
		ColourCorrectPC pc;
		pc.threshold = glm::vec2{ colSettings.shadowThreshold ,colSettings.highlightThreshold };
		pc.shadowCol = colSettings.shadowColour;
		pc.midCol = colSettings.midtonesColour;
		pc.highCol = colSettings.highlightColour;
		pc.exposure = colSettings.exposure;

		pc.shadowCol.a /= 1000.0f;
		pc.midCol.a /= 1000.0f;
		pc.highCol.a /= 1000.0f;

		cmd.SetPushConstant(PSOLayoutDB::BloomPSOLayout, sizeof(ColourCorrectPC), &pc);

		cmd.Dispatch((outputBuffer->width - 1) / 16 + 1, (outputBuffer->height - 1) / 16 + 1);

		previousBuffer = outputBuffer;
	}
	if (regionEnd)
	{
		regionEnd(cmdlist);
	}

	// FXAA 
	if(0)
	{
		marker.pMarkerName = "FXAACOMP";
		if (regionBegin)
		{		
			regionBegin(cmdlist, &marker);
		}	
		cmd.BindPSO(pso_fxaa, PSOLayoutDB::BloomPSOLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
		{// composite online main buffer
			auto* outputBuffer = &vr.renderTargets[vr.renderTargetInUseID].texture;
			auto* inputBuffer = previousBuffer;

			cmd.DescriptorSetBegin(0)
				.BindSampler(0, GfxSamplerManager::GetSampler_BlackBorder())
				.BindImage(1, inputBuffer, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
				.BindImage(2, outputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);


			cmd.Dispatch((outputBuffer->width - 1) / 16 + 1, (outputBuffer->height - 1) / 16 + 1);

			previousBuffer = (vkutils::Texture2D*)outputBuffer;
		}
		if (regionEnd)
		{
			regionEnd(cmdlist);
		}
	}
	

	//  vigneette
	if (vr.currWorld->vignetteSettings.enabled == true)
	{
		marker.pMarkerName = "VignetteCOMP";
		if (regionBegin)
		{		
			regionBegin(cmdlist, &marker);
		}	
		cmd.BindPSO(pso_vignette, PSOLayoutDB::BloomPSOLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
		{// composite online main buffer
			auto* outputBuffer = (&vr.attachments.SD_target[0]);
			auto* inputBuffer = previousBuffer;

			cmd.DescriptorSetBegin(0)
				.BindSampler(0, GfxSamplerManager::GetSampler_BlackBorder())
				.BindImage(1, inputBuffer, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
				.BindImage(2, outputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			auto& vignette = vr.currWorld->vignetteSettings;
			VignettePC pc;
			pc.colour = vignette.colour;
			pc.vignetteValues = glm::vec4{vignette.innerRadius, vignette.outerRadius,0.0,0.0};

			cmd.SetPushConstant(PSOLayoutDB::BloomPSOLayout, sizeof(VignettePC), &pc);

			cmd.Dispatch((outputBuffer->width - 1) / 16 + 1, (outputBuffer->height - 1) / 16 + 1);

			previousBuffer = outputBuffer;
		}
		if (regionEnd)
		{
			regionEnd(cmdlist);
		}
	}
	if (previousBuffer != &vr.renderTargets[vr.renderTargetInUseID].texture) 
	{
		cmd.CopyImage(previousBuffer, &vr.renderTargets[vr.renderTargetInUseID].texture);
	}
}

void BloomPass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& device = vr.m_device.logicalDevice;
	
	vr.attachments.Bloom_brightTarget.destroy();
	for (size_t i = 0; i < vr.attachments.MAX_BLOOM_SAMPLES; i++)
	{
		// destroy
		vr.attachments.Bloom_downsampleTargets[i].destroy();
	}
	vr.attachments.SD_target[0].destroy();
	vr.attachments.SD_target[1].destroy();

	vkDestroyPipelineLayout(device, PSOLayoutDB::BloomPSOLayout, nullptr);
	vkDestroyPipelineLayout(device, PSOLayoutDB::tonemapPSOLayout, nullptr);
	vkDestroyPipelineLayout(device, PSOLayoutDB::doubleImageStoreLayout, nullptr);
	vkDestroyPipelineLayout(device, PSOLayoutDB::brightPixelsLayout, nullptr);
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
	auto currFrame = vr.getFrame();
	// At this point, all dependent resources (gbuffer etc) must be ready.

	VkDescriptorImageInfo texSrc = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_BlackBorder(),
		vr.attachments.Bloom_brightTarget.view,
		VK_IMAGE_LAYOUT_GENERAL);
	
	VkDescriptorImageInfo texOut = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		vr.attachments.Bloom_downsampleTargets[0]  .view,
		VK_IMAGE_LAYOUT_GENERAL);

	VkDescriptorImageInfo basicSampler = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_BlackBorder(),
		0,
		VK_IMAGE_LAYOUT_UNDEFINED);
	DescriptorBuilder::Begin()
		.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
		.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_singleTexture);

	if (SetLayoutDB::compute_doubleImageStore == VK_NULL_HANDLE)
	{
		VkDescriptorImageInfo basicSampler = oGFX::vkutils::inits::descriptorImageInfo(
			GfxSamplerManager::GetDefaultSampler(),
			0,
			VK_IMAGE_LAYOUT_UNDEFINED);
		DescriptorBuilder::Begin()
			.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
			.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.BuildLayout(SetLayoutDB::compute_doubleImageStore);
	}

	VkDescriptorBufferInfo dbi{};
	DescriptorBuilder::Begin()
		.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
		.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_brightPixels);
	
	DescriptorBuilder::Begin()
		.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, &texSrc, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT) // we construct world position using depth
		.BindImage(2, &texOut, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_tonemap);

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

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::BloomPSOLayout));
		VK_NAME(m_device.logicalDevice, "Bloom_PSOLayout", PSOLayoutDB::BloomPSOLayout);

		setLayouts[0] = SetLayoutDB::compute_doubleImageStore;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::doubleImageStoreLayout));
		VK_NAME(m_device.logicalDevice, "doubleImageStore_PSOLayout", PSOLayoutDB::doubleImageStoreLayout);	
		
		setLayouts[0] = SetLayoutDB::compute_brightPixels;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::brightPixelsLayout));
		VK_NAME(m_device.logicalDevice, "brightPixelsLayout", PSOLayoutDB::brightPixelsLayout);
		
		setLayouts[0] = SetLayoutDB::compute_tonemap;
		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::tonemapPSOLayout));
		VK_NAME(m_device.logicalDevice, "tonemapPSOLayout", PSOLayoutDB::tonemapPSOLayout);

	}
}

vkutils::Texture2D* BloomPass::PerformBloom(rhi::CommandList& cmd, vkutils::Texture2D* target)
{
	auto& vr = *VulkanRenderer::get();

	glm::vec4 col = glm::vec4{ 1.0f,1.0f,1.0f,0.0f };
	auto regionBegin = VulkanRenderer::get()->pfnDebugMarkerRegionBegin;
	auto regionEnd = VulkanRenderer::get()->pfnDebugMarkerRegionEnd;

	vkutils::Texture2D& mainImage = *target;

	VkDebugMarkerMarkerInfoEXT marker = {};
	marker.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
	memcpy(marker.color, &col[0], sizeof(float) * 4);	

	VkCommandBuffer cmdlist = cmd.getCommandBuffer();

	{// bright threshold pass
		marker.pMarkerName = "BrightCOMP";
		if (regionBegin)
		{		
			regionBegin(cmdlist, &marker);
		}
		cmd.BindPSO(pso_bloom_bright, PSOLayoutDB::brightPixelsLayout, VK_PIPELINE_BIND_POINT_COMPUTE);

		VkDescriptorBufferInfo dbi{};
		dbi.buffer = vr.LuminanceBuffer.buffer;
		dbi.range = VK_WHOLE_SIZE;
		cmd.DescriptorSetBegin(0)
			.BindImage(1, &mainImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			.BindImage(2, &vr.attachments.Bloom_brightTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
			.BindBuffer(3, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		BloomPC pc{};
		auto knee = vr.currWorld->bloomSettings.threshold * vr.currWorld->bloomSettings.softThreshold;
		pc.threshold.x = vr.currWorld->bloomSettings.threshold;
		pc.threshold.y = pc.threshold.x - knee;
		pc.threshold.z = 2.0f * knee;
		pc.threshold.w = 0.25f / (knee + 0.00001f);
		//pc.threshold = vr.m_ShaderDebugValues.vector4_values0;

		cmd.SetPushConstant(PSOLayoutDB::brightPixelsLayout, sizeof(BloomPC), &pc);

		cmd.Dispatch((vr.attachments.Bloom_brightTarget.width - 1) / 16 + 1, (vr.attachments.Bloom_brightTarget.height - 1) / 16 + 1);
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
		vkutils::Texture* prevImage = &vr.attachments.Bloom_brightTarget;
		vkutils::Texture* currImage;
		//downsample
		cmd.BindPSO(pso_bloom_down, PSOLayoutDB::BloomPSOLayout ,VK_PIPELINE_BIND_POINT_COMPUTE);
		for (size_t i = 0; i < vr.attachments.MAX_BLOOM_SAMPLES; i++)
		{
			currImage = &vr.attachments.Bloom_downsampleTargets[i];
			if (prevImage->width / 2 != currImage->width || prevImage->height / 2 != currImage->height)
			{
				// what do i do here?
				//currImage->Resize(prevImage->width / 2, prevImage->height / 2);
				//std::cout << "HOW?\n"; 
			}
			float mipLevel = float(i);

			cmd.SetPushConstant(PSOLayoutDB::BloomPSOLayout, sizeof(float), &mipLevel);

			cmd.DescriptorSetBegin(0)
				.BindSampler(0, GfxSamplerManager::GetSampler_BlackBorder())
				.BindImage(1, prevImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
				.BindImage(2, currImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			std::array<VkDescriptorSet, 1> decs{vr.descriptorSet_fullscreenBlit};
			cmd.Dispatch((currImage->width - 1) / 16 + 1, (currImage->height - 1) / 16 + 1);
			prevImage = currImage;
		} 
		if (regionEnd)
		{
			regionEnd(cmdlist);
		}
	}// end downsample scope


	 //6 pass iterative upsamping 9tap tent
	{
		marker.pMarkerName = "UpsampleCOMP";

		if (regionBegin)
		{		
			regionBegin(cmdlist, &marker);
		}
		cmd.BindPSO(pso_bloom_up, PSOLayoutDB::doubleImageStoreLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
		for (int i = static_cast<int>(vr.attachments.MAX_BLOOM_SAMPLES - 1ull); i > 0; --i)
		{
			auto* outputBuffer = (&vr.attachments.Bloom_downsampleTargets[i-1ull]);
			auto* inputBuffer = &vr.attachments.Bloom_downsampleTargets[i];

			cmd.DescriptorSetBegin(0)
				.BindSampler(0, GfxSamplerManager::GetSampler_BlackBorder())
				.BindImage(1, inputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
				.BindImage(2, outputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			cmd.Dispatch((outputBuffer->width - 1) / 16 + 1, (outputBuffer->height - 1) / 16 + 1);
		} 

		{ // we reuse the bright output to place the boom
			auto* outputBuffer = (&vr.attachments.Bloom_brightTarget);
			auto* inputBuffer = &vr.attachments.Bloom_downsampleTargets[0];


			cmd.DescriptorSetBegin(0)
				.BindSampler(0, GfxSamplerManager::GetSampler_BlackBorder())
				.BindImage(1, inputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
				.BindImage(2, outputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			cmd.Dispatch((outputBuffer->width - 1) / 16 + 1, (outputBuffer->height - 1) / 16 + 1);
		}
		if (regionEnd)
		{
			regionEnd(cmdlist);
		}
	}

	{
		marker.pMarkerName = "AdditiveCOMP";
		if (regionBegin)
		{
			regionBegin(cmdlist, &marker);
		}
		{// composite online main buffer
			cmd.BindPSO(pso_additive_composite, PSOLayoutDB::BloomPSOLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
			auto* outputBuffer = (&mainImage);
			auto* inputBuffer = &vr.attachments.Bloom_brightTarget;

			cmd.DescriptorSetBegin(0)
				.BindSampler(0, GfxSamplerManager::GetSampler_BlackBorder())
				.BindImage(1, inputBuffer, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
				.BindImage(2, outputBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

			cmd.Dispatch((outputBuffer->width - 1) / 16 + 1, (outputBuffer->height - 1) / 16 + 1);
		}
		if (regionEnd)
		{
			regionEnd(cmdlist);
		}
	}

	return &mainImage;
}

void BloomPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	CreateDescriptors();
	CreatePipelineLayout();
	CreatePSO();
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

	if (pso_bloom_bright != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_bloom_bright, nullptr);
	}
	VkComputePipelineCreateInfo computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::brightPixelsLayout);
	computeCI.stage = vr.LoadShader(m_device, shaderCS, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_bloom_bright));
	VK_NAME(m_device.logicalDevice, "pso_bloom_bright", pso_bloom_bright);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	if (pso_bloom_down != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_bloom_down, nullptr);
	}
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::BloomPSOLayout);
	computeCI.stage = vr.LoadShader(m_device, shaderDownsample, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_bloom_down));
	VK_NAME(m_device.logicalDevice, "pso_bloom_down", pso_bloom_down);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	if (pso_bloom_up != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_bloom_up, nullptr);
	}
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::doubleImageStoreLayout);
	computeCI.stage = vr.LoadShader(m_device, shaderUpample, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_bloom_up));
	VK_NAME(m_device.logicalDevice, "pso_bloom_up", pso_bloom_up);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	if (pso_additive_composite != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_additive_composite, nullptr);
	}
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::BloomPSOLayout);
	computeCI.stage = vr.LoadShader(m_device, compositeAdditive, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_additive_composite));
	VK_NAME(m_device.logicalDevice, "pso_additive_composite", pso_additive_composite);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	if (pso_vignette != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_vignette, nullptr);
	}
	computeCI.stage = vr.LoadShader(m_device, vignette, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_vignette));
	VK_NAME(m_device.logicalDevice, "pso_vignette", pso_vignette);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute

	if (pso_fxaa != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_fxaa, nullptr);
	}
	computeCI.stage = vr.LoadShader(m_device, fxaa, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_fxaa));
	VK_NAME(m_device.logicalDevice, "pso_fxaa", pso_fxaa);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute
	
	if (pso_tone_mapping != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_tone_mapping, nullptr);
	}
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::tonemapPSOLayout);
	computeCI.stage = vr.LoadShader(m_device, toneMap, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_tone_mapping));
	VK_NAME(m_device.logicalDevice, "pso_tone_mapping", pso_tone_mapping);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy compute
}
