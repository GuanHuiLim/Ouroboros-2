/************************************************************************************//*!
\file           FSR2Pass.cpp
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

// FFX defines
#ifndef FFX_CPU
#define FFX_CPU
#endif // !FFX_CPU
#ifndef FFX_GLSL
#define FFX_GLSL
#endif // !FFX_GLSL

//FFX values
#include "../shaders/shared_structs.h"
#include "../shaders/fidelity/include/FidelityFX/gpu/ffx_common_types.h"

#include "FSR2Helper.h"

// must match FSR2 enum
static const char* fsr_shaders[]{
	"Shaders/bin/ffx_fsr2_tcr_autogen_pass.glsl.spv",
	"Shaders/bin/ffx_fsr2_autogen_reactive_pass.glsl.spv",
	"Shaders/bin/ffx_fsr2_compute_luminance_pyramid_pass.glsl.spv",
	"Shaders/bin/ffx_fsr2_reconstruct_previous_depth_pass.glsl.spv",
	"Shaders/bin/ffx_fsr2_depth_clip_pass.glsl.spv",
	"Shaders/bin/ffx_fsr2_lock_pass.glsl.spv",
	"Shaders/bin/ffx_fsr2_accumulate_pass.glsl.spv",
	"Shaders/bin/ffx_fsr2_rcas_pass.glsl.spv",
};

static const char* fsr_shaders_names[]{
	"fsr2_tcr_autogen",
	"fsr2_autogen_reactive",
	"fsr2_compute_luminance_pyramid",
	"fsr2_reconstruct_previous_depth",
	"fsr2_depth_clip",
	"fsr2_lock",
	"fsr2_accumulate",
	"fsr2_rcas",
};



struct FSR2Pass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(FSR2Pass)

	void Init() override;
	void Draw(const VkCommandBuffer cmdlist) override;
	void Shutdown() override;

	bool SetupDependencies() override;

	void CreatePSO() override;
	void CreatePipelineLayout();
	void CreateDescriptors();

private:

	void SetupRenderpass();
	void CreatePipeline();

};

DECLARE_RENDERPASS(FSR2Pass);

VkPipeline pso_fsr2[FSR2::MAX_SIZE]{};

vkutils::Texture2D FSR2atomicBuffer;
vkutils::Texture2D FSR2AutoExposure;

void FSR2Pass::Init()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainext = vr.m_swapchain.swapChainExtent;

	SetupDependencies();

	float halfResolution = 0.5f;
	float fullResolution = 1.0f;
	vr.attachments.fsr_exposure_mips.name = "fsr_exposure_mips";
	vr.attachments.fsr_exposure_mips.forFrameBuffer(&vr.m_device, VK_FORMAT_R16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, halfResolution,1,VK_IMAGE_LAYOUT_GENERAL); // half resolution for mipmap
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_exposure_mips);
	
	vr.attachments.fsr_reconstructed_prev_depth.name = "fsr_reconstructed_prev_depth";
	vr.attachments.fsr_reconstructed_prev_depth.forFrameBuffer(&vr.m_device, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL); 
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_reconstructed_prev_depth);

	vr.attachments.fsr_dilated_depth.name = "fsr_dilated_depth";
	vr.attachments.fsr_dilated_depth.forFrameBuffer(&vr.m_device, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL);
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_dilated_depth);

	for (size_t i = 0; i < VulkanRenderer::MAX_FRAME_DRAWS; i++)
	{
		vr.attachments.fsr_dilated_velocity[i].name = "fsr_dilated_velocity_" + std::to_string(i+1);
		vr.attachments.fsr_dilated_velocity[i].forFrameBuffer(&vr.m_device, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, true, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL);
		vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_dilated_velocity[i]);

		vr.attachments.fsr_upscaled_color[i].name = "fsr_upscaled_color_" + std::to_string(i+1);
		vr.attachments.fsr_upscaled_color[i].forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT_ALPHA, VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, false, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL); 
		vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_upscaled_color[i]);

		vr.attachments.fsr_lock_status[i].name = "fsr_lock_status" + std::to_string(i+1);
		vr.attachments.fsr_lock_status[i].forFrameBuffer(&vr.m_device, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, false, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL);
		vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_lock_status[i]);

		vr.attachments.fsr_luma_history[i].name = "fsr_luma_history" + std::to_string(i+1);
		vr.attachments.fsr_luma_history[i].forFrameBuffer(&vr.m_device, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT,
			swapchainext.width, swapchainext.height, false, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL);
		vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_luma_history[i]);
	}

	

	vr.attachments.fsr_lock_input_luma.name = "fsr_lock_input_luma";
	vr.attachments.fsr_lock_input_luma.forFrameBuffer(&vr.m_device, VK_FORMAT_R16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL);
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_lock_input_luma);

	vr.attachments.fsr_dilated_reactive_masks.name = "fsr_dilated_reactive_masks";
	vr.attachments.fsr_dilated_reactive_masks.forFrameBuffer(&vr.m_device, VK_FORMAT_R8G8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL);
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_dilated_reactive_masks);

	vr.attachments.fsr_reactive_mask.name = "fsr_reactive_mask";
	vr.attachments.fsr_reactive_mask.forFrameBuffer(&vr.m_device, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL); 
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_reactive_mask);
	
	vr.attachments.fsr_prepared_input_color.name = "fsr_prepared_input_color";
	vr.attachments.fsr_prepared_input_color.forFrameBuffer(&vr.m_device, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, true, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL); 
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_prepared_input_color);
	
	vr.attachments.fsr_new_locks.name = "fsr_new_locks";
	vr.attachments.fsr_new_locks.forFrameBuffer(&vr.m_device, VK_FORMAT_R8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, false, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL); // half resolution for mipmap
	vr.fbCache.RegisterFramebuffer(vr.attachments.fsr_new_locks);
	
	vr.attachments.fullres_HDR.name = "fullres_HDR";
	vr.attachments.fullres_HDR.forFrameBuffer(&vr.m_device, vr.G_HDR_FORMAT, VK_IMAGE_USAGE_STORAGE_BIT,
		swapchainext.width, swapchainext.height, false, fullResolution, 1, VK_IMAGE_LAYOUT_GENERAL);
	vr.fbCache.RegisterFramebuffer(vr.attachments.fullres_HDR);

	FSR2atomicBuffer.name = "FSR2_atomic_tex";
	FSR2atomicBuffer.forFrameBuffer(&vr.m_device, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT,
		1, 1, false, 1.0f, 1, VK_IMAGE_LAYOUT_GENERAL);

	FSR2AutoExposure.name = "FSR2_AutoExposure_tex";
	FSR2AutoExposure.forFrameBuffer(&vr.m_device, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT,
		1, 1, false, 1.0f, 1, VK_IMAGE_LAYOUT_GENERAL);
	
	// for initializing textures etc
	auto cmd = vr.GetCommandBuffer();

	vkutils::SetImageInitialState(cmd, vr.attachments.fsr_exposure_mips);
	vkutils::SetImageInitialState(cmd, vr.attachments.fsr_reconstructed_prev_depth);
	for (size_t i = 0; i < VulkanRenderer::MAX_FRAME_DRAWS; i++)
	{
		vkutils::SetImageInitialState(cmd, vr.attachments.fsr_dilated_velocity[i]);
		vkutils::SetImageInitialState(cmd, vr.attachments.fsr_upscaled_color[i]);
		vkutils::SetImageInitialState(cmd, vr.attachments.fsr_lock_status[i]);
		vkutils::SetImageInitialState(cmd, vr.attachments.fsr_luma_history[i]);
	}
	
	vkutils::SetImageInitialState(cmd, vr.attachments.fsr_dilated_depth);
	vkutils::SetImageInitialState(cmd, vr.attachments.fsr_lock_input_luma);
	vkutils::SetImageInitialState(cmd, vr.attachments.fsr_dilated_reactive_masks);
	vkutils::SetImageInitialState(cmd, vr.attachments.fsr_prepared_input_color);
	vkutils::SetImageInitialState(cmd, vr.attachments.fsr_new_locks);

	vkutils::SetImageInitialState(cmd, FSR2atomicBuffer);
	vkutils::SetImageInitialState(cmd, FSR2AutoExposure);

	vkutils::SetImageInitialState(cmd, vr.attachments.fullres_HDR);

	VkClearColorValue clear{};
	clear.float32[0] = 0;
	clear.float32[1] = 0;
	VkImageSubresourceRange rng{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };	
	vkCmdClearColorImage(cmd, FSR2atomicBuffer.image.image, FSR2atomicBuffer.referenceLayout, &clear, 1, &rng); // clear to zero
	vkCmdClearColorImage(cmd, FSR2AutoExposure.image.image, FSR2AutoExposure.referenceLayout, &clear, 1, &rng); // clear to zero

	vr.SubmitSingleCommandAndWait(cmd);
	vr.GenerateMipmaps(vr.attachments.fsr_exposure_mips);
	
	SetupRenderpass();

	CreateDescriptors();
	CreatePipelineLayout();
	CreatePSO();

}

void FSR2Pass::CreatePSO()
{	
	CreatePipeline();
}

bool FSR2Pass::SetupDependencies()
{
	auto& vr = *VulkanRenderer::get();
	constexpr size_t MAX_FRAMES = 2;


	for (size_t i = 0; i < MAX_FRAMES; i++)
	{
		oGFX::CreateBuffer(vr.m_device.m_allocator, sizeof(FSR2_CB_DATA)
			, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
			, vr.FSR2constantBuffer[i]);
		VK_NAME(vr.m_device.logicalDevice, "FSR2 CB", vr.FSR2constantBuffer[i].buffer);
		
		oGFX::CreateBuffer(vr.m_device.m_allocator, sizeof(Fsr2SpdConstants)
			, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
			, vr.FSR2luminanceCB[i]);
		VK_NAME(vr.m_device.logicalDevice, "FSR2 lumCB", vr.FSR2luminanceCB[i].buffer);

		oGFX::CreateBuffer(vr.m_device.m_allocator, sizeof(Fsr2RcasConstants)
			, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
			, vr.FSR2rcasBuffer[i]);
		VK_NAME(vr.m_device.logicalDevice, "FSR2 RCAS_CB", vr.FSR2rcasBuffer[i].buffer);
		
		oGFX::CreateBuffer(vr.m_device.m_allocator, sizeof(FSR2AutogenConstants)
			, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
			, vr.FSR2autoGen[i]);
		VK_NAME(vr.m_device.logicalDevice, "FSR2 autoGen", vr.FSR2autoGen[i].buffer);
				
	}

	return true;
}

void SetupConstantBuffers() 
{
	VulkanRenderer& vr = *VulkanRenderer::get();
	size_t currFrame = vr.getFrame();

	Camera& cam = vr.currWorld->cameras[0];
	VkExtent2D resInfo = vr.m_swapchain.swapChainExtent;
	
	// Jitter handled in main

	constantBuffer.jitterOffset[0] = vr.jitterX * -1.0f;
	constantBuffer.jitterOffset[1] = vr.jitterY * -1.0f;

	if (constantBuffer.jitterPhaseCount == 0) {
		constantBuffer.jitterPhaseCount = vr.jitterPhaseCount;
	}
	else {
		const int32_t jitterPhaseCountDelta = (int32_t)(vr.jitterPhaseCount - constantBuffer.jitterPhaseCount);
		if (jitterPhaseCountDelta > 0) {
			constantBuffer.jitterPhaseCount++;
		}
		else if (jitterPhaseCountDelta < 0) {
			constantBuffer.jitterPhaseCount--;
		}
	}
	

	//printf("[%d] gpujitter [%1.4f,%1.4f], cameraJitter [%1.4f,%1.4f], previousJitter [%1.4f,%1.4f]\n",
	//	vr.m_JitterIndex
	//	, constantBuffer.jitterOffset[0]
	//	, constantBuffer.jitterOffset[1]
	//	, cam.jitterValues.x,cam.jitterValues.y
	//	, vr.prevjitterX,	 vr.prevjitterY);

	// compute the horizontal FOV for the shader from the vertical one.
	float fovYrad = glm::radians(cam.m_fovDegrees);
	const float aspectRatio = (float)vr.renderWidth / (float)vr.renderHeight;
	const float cameraAngleHorizontal = atan(tan(fovYrad / 2) * aspectRatio) * 2;
	constantBuffer.tanHalfFOV = tanf(cameraAngleHorizontal * 0.5f);
	constantBuffer.viewSpaceToMetersFactor = 1.0f; // wtf is this

	// compute params to enable device depth to view space depth computation in shader
	setupDeviceDepthToViewSpaceDepthParams(cam);

	constantBuffer.previousFramePreExposure = constantBuffer.preExposure;
	constantBuffer.preExposure = (constantBuffer.previousFramePreExposure != 0) ? constantBuffer.previousFramePreExposure : 1.0f;

	VkExtent2D renderSize = { vr.renderWidth ,vr.renderHeight};
	// motion vector data
	const uint32_t* motionVectorsTargetSize = (FFX_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS) ? (uint32_t*)&resInfo : (uint32_t*)&renderSize;

	 // compute jitter cancellation
	 if (FFX_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION) 
	 {	 
		 static glm::vec2 previousJitterOffset{};

	 	constantBuffer.motionVectorJitterCancellation[0] = (previousJitterOffset[0] - constantBuffer.jitterOffset[0]) / motionVectorsTargetSize[0];
	 	constantBuffer.motionVectorJitterCancellation[1] = (previousJitterOffset[1] - constantBuffer.jitterOffset[1]) / motionVectorsTargetSize[1];
	
		previousJitterOffset[0] = constantBuffer.jitterOffset[0];
		previousJitterOffset[1] = constantBuffer.jitterOffset[1];
	 }

	 constantBuffer.motionVectorScale.x = 1.0f;
	 constantBuffer.motionVectorScale.y = 1.0f;

	 // guarenteed OK

	 constantBuffer.displaySize[0] = resInfo.width;
	 constantBuffer.displaySize[1] = resInfo.height;

	 constantBuffer.frameIndex = vr.fsrFrameCount++;

	 constantBuffer.renderSize[0] = vr.renderWidth;
	 constantBuffer.renderSize[1] = vr.renderHeight;
	 constantBuffer.maxRenderSize[0] = vr.renderWidth;
	 constantBuffer.maxRenderSize[1] = vr.renderHeight;

	 // or colour .size
	 constantBuffer.inputColorResourceDimensions[0] = vr.renderWidth;
	 constantBuffer.inputColorResourceDimensions[1] = vr.renderHeight;

	// To be updated if resource is larger than the actual image size
	 constantBuffer.downscaleFactor[0] = float(vr.renderWidth) / resInfo.width;
	 constantBuffer.downscaleFactor[1] = float(vr.renderHeight) / resInfo.height;

	 float toMilliseconds = 1000.0f;
	 constantBuffer.deltaTime = glm::clamp(vr.deltaTime, 0.0f, 1.0f);
	 

	 constantBuffer.lumaMipLevelToUse = 4;
	 glm::uvec2 mipDims = vkutils::GetMipDims(vr.attachments.fsr_exposure_mips, constantBuffer.lumaMipLevelToUse);
	 constantBuffer.lumaMipDimensions[0] = mipDims.x;
	 constantBuffer.lumaMipDimensions[1] = mipDims.y;

	 memcpy(vr.FSR2constantBuffer[currFrame].allocInfo.pMappedData, &constantBuffer, sizeof(FSR2_CB_DATA));


	 // compute the constants.
	 rcasCB = {};
	 float sharpness = std::clamp(vr.rcas_sharpness, 0.0f, 1.0f);
	 const float sharpenessRemapped = (-2.0f * sharpness) + 2.0f;
	 FsrRcasCon(rcasCB.rcasConfig, sharpenessRemapped);

	 memcpy(vr.FSR2rcasBuffer[currFrame].allocInfo.pMappedData, &rcasCB, sizeof(Fsr2RcasConstants));

	 // autogen
	 autogenCB = {};
	 autogenCB.scale = 1.0f;
	 autogenCB.threshold =  0.2f;
	 autogenCB.binaryValue =  0.9f;
	 autogenCB.flags = FFX_FSR2_AUTOREACTIVEFLAGS_APPLY_TONEMAP |
		 FFX_FSR2_AUTOREACTIVEFLAGS_APPLY_THRESHOLD |
		 FFX_FSR2_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX;

	 memcpy(vr.FSR2autoGen[currFrame].allocInfo.pMappedData, &autogenCB, sizeof(FSR2AutogenConstants));
}

void FSR2Pass::Draw(const VkCommandBuffer cmdlist)
{
	VulkanRenderer& vr = *VulkanRenderer::get();
	uint32_t currFrame = vr.getFrame();
	uint32_t prevFrame = vr.getPreviousFrame();
	Window* windowPtr = vr.windowPtr;
	
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("FSR2");
	rhi::CommandList cmd{ cmdlist, "FSR2",{1,0,0,0.5} };
	lastCmd = cmdlist;

	SetupConstantBuffers();

	const int32_t threadGroupWorkRegionDim = 8;
	glm::uvec2 dispatchSrc{
		(vr.renderWidth-1)  / threadGroupWorkRegionDim + 1,
		(vr.renderHeight-1) / threadGroupWorkRegionDim + 1
	};
	
	glm::uvec2 dispatchDst{
		(vr.m_swapchain.swapChainExtent.width - 1) / threadGroupWorkRegionDim + 1,
		(vr.m_swapchain.swapChainExtent.height - 1) / threadGroupWorkRegionDim + 1
	};
	
	// Auto exposure
	uint32_t spdDispatchThreads[2];
	uint32_t workGroupOffset[2];
	uint32_t numWorkGroupsAndMips[2];
	uint32_t rectInfo[4] = { 0, 0, vr.renderWidth, vr.renderHeight };
	ffxSpdSetup(spdDispatchThreads, workGroupOffset, numWorkGroupsAndMips, rectInfo, -1);

	// downsample
	Fsr2SpdConstants luminancePyramidConstants;
	luminancePyramidConstants.numworkGroups = numWorkGroupsAndMips[0];
	luminancePyramidConstants.mips = numWorkGroupsAndMips[1];
	luminancePyramidConstants.workGroupOffset[0] = workGroupOffset[0];
	luminancePyramidConstants.workGroupOffset[1] = workGroupOffset[1];
	luminancePyramidConstants.renderSize[0] = vr.renderWidth;
	luminancePyramidConstants.renderSize[1] = vr.renderHeight;
	
	// copy over
	memcpy(vr.FSR2luminanceCB[currFrame].allocInfo.pMappedData, &luminancePyramidConstants, sizeof(Fsr2SpdConstants));

	//  FSR2_BIND_SRV_INPUT_COLOR                     0
	// 
	//  FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC            2001
	//  FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE     2002
	//  FSR2_BIND_UAV_EXPOSURE_MIP_5               2003
	//  FSR2_BIND_UAV_AUTO_EXPOSURE                2004
	// 
	//  FSR2_BIND_CB_FSR2                          3000
	//  FSR2_BIND_CB_SPD                           3001

	constexpr size_t maxNumMips = 13;
	std::array < VkImageView, maxNumMips> mipViews{};
	VkImageViewCreateInfo viewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	viewCreateInfo.pNext = NULL;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // for shader
	viewCreateInfo.format = vr.attachments.fsr_exposure_mips.format;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = vr.attachments.fsr_exposure_mips.layerCount;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;

	viewCreateInfo.image = vr.attachments.fsr_exposure_mips.image.image;
	for (size_t i = 0; i < vr.attachments.fsr_exposure_mips.mipLevels; i++)
	{
		viewCreateInfo.subresourceRange.baseMipLevel = (uint32_t)i;
		vkCreateImageView(vr.m_device.logicalDevice,&viewCreateInfo,nullptr, &mipViews[i]);
		//printf("Created %llu\n", size_t(mipViews[i]));
	}

	std::array<VkDescriptorImageInfo, maxNumMips> samplers{};
	for (size_t i = 0; i < samplers.size(); i++)
	{
		samplers[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplers[i].imageView = mipViews[0];
		samplers[i].sampler = nullptr;
	}
	for (size_t i = 0; i < vr.attachments.fsr_exposure_mips.mipLevels; i++)
	{		
		samplers[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplers[i].imageView = mipViews[i];
		samplers[i].sampler = nullptr;
	}

	// autogen reactive
	cmd.BindPSO(pso_fsr2[FSR2::AUTOGEN_REACTIVE], PSOLayoutDB::fsr2_PSOLayouts[FSR2::AUTOGEN_REACTIVE], VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(0, &vr.attachments.lighting_target, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(1, &vr.attachments.lighting_target, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)

		.BindSampler(1000, GfxSamplerManager::GetSampler_PointClamp()) // point clamp
		.BindSampler(1001, GfxSamplerManager::GetSampler_LinearClamp()) // linear clamp

		.BindImage(2002, &vr.attachments.fsr_reactive_mask, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)


		.BindBuffer(3000, vr.FSR2autoGen[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.BindBuffer(3001, vr.FSR2constantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.Dispatch(dispatchSrc[0], dispatchSrc[1]);


	// Luminance pyramid
	cmd.BindPSO(pso_fsr2[FSR2::COMPUTE_LUMINANCE_PYRAMID], PSOLayoutDB::fsr2_PSOLayouts[FSR2::COMPUTE_LUMINANCE_PYRAMID], VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(0, &vr.attachments.lighting_target, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)

		.BindSampler(1000, GfxSamplerManager::GetSampler_PointClamp()) // point clamp
		.BindSampler(1001, GfxSamplerManager::GetSampler_LinearClamp()) // linear clamp

		.BindImage(2001, &FSR2atomicBuffer, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindImage(2002, &vr.attachments.fsr_exposure_mips, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,mipViews[4])
		.BindImage(2003, &vr.attachments.fsr_exposure_mips, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,mipViews[5])
		.BindImage(2004, &FSR2AutoExposure, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)

		.BindBuffer(3000, vr.FSR2constantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.BindBuffer(3001, vr.FSR2luminanceCB[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	cmd.Dispatch(spdDispatchThreads[0], spdDispatchThreads[1]);

	// Reconstruct and dilate
	VkClearValue cv{};
	cv.depthStencil = {};
	//cmd.ClearImage(&vr.attachments.fsr_reconstructed_prev_depth,cv);
	cmd.BindPSO(pso_fsr2[FSR2::RECONSTRUCT_PREVIOUS_DEPTH], PSOLayoutDB::fsr2_PSOLayouts[FSR2::RECONSTRUCT_PREVIOUS_DEPTH], VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(0, &vr.attachments.gbuffer[GBufferAttachmentIndex::VELOCITY], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(1, &vr.attachments.gbuffer[GBufferAttachmentIndex::DEPTH], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(2, &vr.attachments.lighting_target, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(3, &FSR2AutoExposure, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(4, &FSR2AutoExposure, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)

		.BindSampler(1000, GfxSamplerManager::GetSampler_PointClamp()) // point clamp
		.BindSampler(1001, GfxSamplerManager::GetSampler_LinearClamp()) // linear clamp

		.BindImage(2005, &vr.attachments.fsr_reconstructed_prev_depth, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) // must clear
		.BindImage(2006, &vr.attachments.fsr_dilated_velocity[prevFrame], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindImage(2007, &vr.attachments.fsr_dilated_depth, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) // dont need clear..
		.BindImage(2011, &vr.attachments.fsr_lock_input_luma, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)

		.BindBuffer(3000, vr.FSR2constantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.Dispatch(dispatchSrc.x, dispatchSrc.y);

	// Depth clip
	cmd.BindPSO(pso_fsr2[FSR2::DEPTH_CLIP], PSOLayoutDB::fsr2_PSOLayouts[FSR2::DEPTH_CLIP], VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(0 , &vr.attachments.fsr_reconstructed_prev_depth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(1 , &vr.attachments.fsr_dilated_velocity[prevFrame], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(2 , &vr.attachments.fsr_dilated_depth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(3 , &vr.attachments.fsr_reactive_mask, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) // reactive
		.BindImage(4 , &vr.g_Textures[vr.blackTextureID], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) // transparent
		.BindImage(5 , &vr.attachments.lighting_target, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(6 , &vr.attachments.fsr_dilated_velocity[currFrame], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(7 , &vr.attachments.gbuffer[GBufferAttachmentIndex::VELOCITY], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(8 , &vr.attachments.lighting_target, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(9 , &vr.attachments.gbuffer[GBufferAttachmentIndex::DEPTH], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(10, &FSR2AutoExposure, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)

		.BindSampler(1000, GfxSamplerManager::GetSampler_PointClamp()) // point clamp
		.BindSampler(1001, GfxSamplerManager::GetSampler_LinearClamp()) // linear clamp

		.BindImage(2012, &vr.attachments.fsr_dilated_reactive_masks, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindImage(2013, &vr.attachments.fsr_prepared_input_color, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)

		.BindBuffer(3000, vr.FSR2constantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.Dispatch(dispatchSrc.x, dispatchSrc.y);

	// Generate Locks
	cmd.BindPSO(pso_fsr2[FSR2::LOCK], PSOLayoutDB::fsr2_PSOLayouts[FSR2::LOCK], VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(0, &vr.attachments.fsr_lock_input_luma, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		
		.BindSampler(1000, GfxSamplerManager::GetSampler_PointClamp()) // point clamp
		.BindSampler(1001, GfxSamplerManager::GetSampler_LinearClamp()) // linear clamp

		.BindImage(2001, &vr.attachments.fsr_new_locks, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindImage(2002, &vr.attachments.fsr_reconstructed_prev_depth, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)

		.BindBuffer(3000, vr.FSR2constantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.Dispatch(dispatchSrc.x, dispatchSrc.y);

	// Reproject & accumulate
	cmd.BindPSO(pso_fsr2[FSR2::ACCUMULATE], PSOLayoutDB::fsr2_PSOLayouts[FSR2::ACCUMULATE], VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(0, &FSR2AutoExposure, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(1, &vr.attachments.fsr_dilated_reactive_masks, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(2, &vr.attachments.fsr_dilated_velocity[prevFrame], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(3, &vr.attachments.fsr_upscaled_color[prevFrame], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(4, &vr.attachments.fsr_lock_status[prevFrame], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(6, &vr.attachments.fsr_prepared_input_color, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(10, &vr.attachments.fsr_exposure_mips, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(12, &vr.attachments.fsr_luma_history[prevFrame], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)

		.BindSampler(1000, GfxSamplerManager::GetSampler_PointClamp()) // point clamp
		.BindSampler(1001, GfxSamplerManager::GetSampler_LinearClamp()) // linear clamp

		.BindImage(2013, &vr.attachments.fsr_upscaled_color[currFrame], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindImage(2014, &vr.attachments.fsr_lock_status[currFrame], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
#ifdef FFX_FSR2_OPTION_APPLY_SHARPENING
		.BindImage(2015, &vr.attachments.fullres_HDR, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
#endif // DEBUG
		.BindImage(2016, &vr.attachments.fsr_new_locks, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindImage(2017, &vr.attachments.fsr_luma_history[currFrame], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)

		.BindBuffer(3000, vr.FSR2constantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.Dispatch(dispatchDst.x, dispatchDst.y);

	// RCAS
	cmd.BindPSO(pso_fsr2[FSR2::RCAS], PSOLayoutDB::fsr2_PSOLayouts[FSR2::RCAS], VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(0, &FSR2AutoExposure, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(1, &vr.attachments.fsr_upscaled_color[currFrame], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)

		.BindSampler(1000, GfxSamplerManager::GetSampler_PointClamp()) // point clamp
		.BindSampler(1001, GfxSamplerManager::GetSampler_LinearClamp()) // linear clamp

		.BindImage(2002, &vr.attachments.fullres_HDR, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)

		.BindBuffer(3000, vr.FSR2constantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		.BindBuffer(3001, vr.FSR2rcasBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	
	const int32_t threadGroupWorkRegionDimRCAS = 16;
	glm::uvec2 rcasDispatchDst = {
		(vr.m_swapchain.swapChainExtent.width - 1) / threadGroupWorkRegionDimRCAS + 1,
		(vr.m_swapchain.swapChainExtent.height - 1) / threadGroupWorkRegionDimRCAS + 1
	};
	//cmd.Dispatch(rcasDispatchDst.x, rcasDispatchDst.y);

	DelayedDeleter::get()->DeleteAfterFrames([views = mipViews, dev = vr.m_device.logicalDevice]() {
		for (size_t i = 0; i < views.size(); i++)
		{
			if (views[i] != VK_NULL_HANDLE)
			{
				//printf("Destroying %llu\n", size_t(views[i]));
				vkDestroyImageView(dev, views[i], nullptr);
			}
		}
	});	

}

void FSR2Pass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& device = vr.m_device.logicalDevice;

	vr.attachments.fsr_exposure_mips.destroy();

	vr.attachments.fsr_reconstructed_prev_depth.destroy();

	vr.attachments.fsr_dilated_depth.destroy();
	for (size_t i = 0; i < VulkanRenderer::MAX_FRAME_DRAWS; i++)
	{
		vr.attachments.fsr_dilated_velocity[i].destroy();
		vr.attachments.fsr_upscaled_color[i].destroy();
		vr.attachments.fsr_lock_status[i].destroy();
		vr.attachments.fsr_luma_history[i].destroy();
	}	

	vr.attachments.fsr_lock_input_luma.destroy();

	vr.attachments.fsr_dilated_reactive_masks.destroy();

	vr.attachments.fsr_reactive_mask.destroy();

	vr.attachments.fsr_prepared_input_color.destroy();

	vr.attachments.fsr_new_locks.destroy();

	FSR2atomicBuffer.destroy();
	FSR2AutoExposure.destroy();

	vr.attachments.fullres_HDR.destroy();

	constexpr size_t MAX_FRAMES = 2;
	for (size_t i = 0; i < MAX_FRAMES; i++)
	{
		vmaDestroyBuffer(vr.m_device.m_allocator, vr.FSR2constantBuffer[i].buffer, vr.FSR2constantBuffer[i].alloc);
		vmaDestroyBuffer(vr.m_device.m_allocator, vr.FSR2luminanceCB[i].buffer, vr.FSR2luminanceCB[i].alloc);
		vmaDestroyBuffer(vr.m_device.m_allocator, vr.FSR2rcasBuffer[i].buffer, vr.FSR2rcasBuffer[i].alloc);
		vmaDestroyBuffer(vr.m_device.m_allocator, vr.FSR2autoGen[i].buffer, vr.FSR2autoGen[i].alloc);
	}

	for (size_t i = 0; i < FSR2::MAX_SIZE; i++)
	{
		vkDestroyPipelineLayout(device, PSOLayoutDB::fsr2_PSOLayouts[i], nullptr);
		vkDestroyPipeline(device, pso_fsr2[i], nullptr);
	}
}

void FSR2Pass::CreateDescriptors()
{

	auto& vr = *VulkanRenderer::get();
	auto& target = vr.renderTargets[vr.renderTargetInUseID].texture;
	auto currFrame = vr.getFrame();


	// FSR2_BIND_SRV_INPUT_OPAQUE_ONLY                     0
	// FSR2_BIND_SRV_INPUT_COLOR                           1
	// FSR2_BIND_SRV_INPUT_MOTION_VECTORS                  2
	// FSR2_BIND_SRV_PREV_PRE_ALPHA_COLOR                  3
	// FSR2_BIND_SRV_PREV_POST_ALPHA_COLOR                 4
	// FSR2_BIND_SRV_REACTIVE_MASK                         5
	// FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK     6
	//
	// FSR2_BIND_UAV_AUTOREACTIVE                       2007
	// FSR2_BIND_UAV_AUTOCOMPOSITION                    2008
	// FSR2_BIND_UAV_PREV_PRE_ALPHA_COLOR               2009
	// FSR2_BIND_UAV_PREV_POST_ALPHA_COLOR              2010
	//
	// FSR2_BIND_CB_FSR2								 3000
	// FSR2_BIND_CB_AUTOREACTIVE                        3001
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(3, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(4, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(5, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(6, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		.BindImage(2007, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2008, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2009, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2010, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3001, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::TCR_AUTOGEN]);


	// FSR2_BIND_SRV_INPUT_OPAQUE_ONLY                     0
	// FSR2_BIND_SRV_INPUT_COLOR                           1
	//
	// FSR2_BIND_UAV_AUTOREACTIVE                       2002
	//
	// FSR2_BIND_CB_REACTIVE                            3000
	// FSR2_BIND_CB_FSR2                                3001
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		.BindImage(2002, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3001, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::AUTOGEN_REACTIVE]);


	//  FSR2_BIND_SRV_INPUT_COLOR                     0
	// 
	//  FSR2_BIND_UAV_SPD_GLOBAL_ATOMIC            2001
	//  FSR2_BIND_UAV_EXPOSURE_MIP_LUMA_CHANGE     2002
	//  FSR2_BIND_UAV_EXPOSURE_MIP_5               2003
	//  FSR2_BIND_UAV_AUTO_EXPOSURE                2004
	// 
	//  FSR2_BIND_CB_FSR2                          3000
	//  FSR2_BIND_CB_SPD                           3001
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr , VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr , VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr , VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		.BindImage(2001, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2002, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2003, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2004, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3001, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::COMPUTE_LUMINANCE_PYRAMID]);

	// FSR2_BIND_SRV_INPUT_MOTION_VECTORS                  0
	// FSR2_BIND_SRV_INPUT_DEPTH                           1
	// FSR2_BIND_SRV_INPUT_COLOR                           2
	// FSR2_BIND_SRV_INPUT_EXPOSURE                        3
	// FSR2_BIND_SRV_LUMA_HISTORY                          4
	// 
	// FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH   2005
	// FSR2_BIND_UAV_DILATED_MOTION_VECTORS             2006
	// FSR2_BIND_UAV_DILATED_DEPTH                      2007
	// FSR2_BIND_UAV_PREPARED_INPUT_COLOR               2008
	// FSR2_BIND_UAV_LUMA_HISTORY                       2009
	// FSR2_BIND_UAV_LUMA_INSTABILITY                   2010
	// FSR2_BIND_UAV_LOCK_INPUT_LUMA                    2011
	// 
	// FSR2_BIND_CB_FSR2                                3000
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(3, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(4, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		.BindImage(2005, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2006, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2007, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(2008, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(2009, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(2010, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2011, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::RECONSTRUCT_PREVIOUS_DEPTH]);

	// FSR2_BIND_SRV_RECONSTRUCTED_PREV_NEAREST_DEPTH      0
	// FSR2_BIND_SRV_DILATED_MOTION_VECTORS                1
	// FSR2_BIND_SRV_DILATED_DEPTH                         2
	// FSR2_BIND_SRV_REACTIVE_MASK                         3
	// FSR2_BIND_SRV_TRANSPARENCY_AND_COMPOSITION_MASK     4
	// FSR2_BIND_SRV_PREPARED_INPUT_COLOR                  5
	// FSR2_BIND_SRV_PREVIOUS_DILATED_MOTION_VECTORS       6
	// FSR2_BIND_SRV_INPUT_MOTION_VECTORS                  7
	// FSR2_BIND_SRV_INPUT_COLOR                           8
	// FSR2_BIND_SRV_INPUT_DEPTH                           9
	// FSR2_BIND_SRV_INPUT_EXPOSURE                        10
	//
	// FSR2_BIND_UAV_DEPTH_CLIP                          2011 // no need
	// FSR2_BIND_UAV_DILATED_REACTIVE_MASKS              2012
	// FSR2_BIND_UAV_PREPARED_INPUT_COLOR                2013
	//
	// FSR2_BIND_CB_FSR2                                 3000
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(3, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(4, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(5, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(6, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(7, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(8, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(9, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(10, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		//.BindImage(2011, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2012, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2013, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::DEPTH_CLIP]);


	// FSR2_BIND_SRV_LOCK_INPUT_LUMA                       0
	//
	// FSR2_BIND_UAV_NEW_LOCKS                          2001
	// FSR2_BIND_UAV_RECONSTRUCTED_PREV_NEAREST_DEPTH   2002
	//
	// FSR2_BIND_CB_FSR2                                3000
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		.BindImage(2001, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2002, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::LOCK]);
	
	//  FSR2_BIND_SRV_INPUT_EXPOSURE                         0
	//  FSR2_BIND_SRV_DILATED_REACTIVE_MASKS                 1
	///	#if FFX_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS
	//		FSR2_BIND_SRV_DILATED_MOTION_VECTORS             2
	///	#else
	//		FSR2_BIND_SRV_INPUT_MOTION_VECTORS               2
	///	#endif
	//  FSR2_BIND_SRV_INTERNAL_UPSCALED                      3
	//  FSR2_BIND_SRV_LOCK_STATUS                            4
	//  FSR2_BIND_SRV_INPUT_DEPTH_CLIP                       5
	//  FSR2_BIND_SRV_PREPARED_INPUT_COLOR                   6
	//  FSR2_BIND_SRV_LUMA_INSTABILITY                       7
	//  FSR2_BIND_SRV_LANCZOS_LUT                            8
	//  FSR2_BIND_SRV_UPSCALE_MAXIMUM_BIAS_LUT               9
	//  FSR2_BIND_SRV_SCENE_LUMINANCE_MIPS                   10
	//  FSR2_BIND_SRV_AUTO_EXPOSURE                          11
	//  FSR2_BIND_SRV_LUMA_HISTORY                           12
	// 
	//  FSR2_BIND_UAV_INTERNAL_UPSCALED                      2013
	//  FSR2_BIND_UAV_LOCK_STATUS                            2014
	//  FSR2_BIND_UAV_UPSCALED_OUTPUT                        2015
	//  FSR2_BIND_UAV_NEW_LOCKS                              2016
	//  FSR2_BIND_UAV_LUMA_HISTORY                           2017
	// 
	//  FSR2_BIND_CB_FSR2                                    3000
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(3, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(4, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(5, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(6, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(7, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(8, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(9, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(10, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		//.BindImage(11, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(12, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		.BindImage(2013, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2014, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
#ifdef FFX_FSR2_OPTION_APPLY_SHARPENING
		.BindImage(2015, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
#endif // DEBUG
		.BindImage(2016, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2017, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::ACCUMULATE]);


	// FSR2_BIND_SRV_INPUT_EXPOSURE        0
	// FSR2_BIND_SRV_RCAS_INPUT            1
	//
	// FSR2_BIND_UAV_UPSCALED_OUTPUT    2002
	//
	// FSR2_BIND_CB_FSR2                3000
	// FSR2_BIND_CB_RCAS                3001
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindImage(1000, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // point clamp
		.BindImage(1001, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT) // linear clamp

		.BindImage(2002, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)

		.BindBuffer(3000, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3001, nullptr, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_fsr2[FSR2::RCAS]);
	
	
}

void FSR2Pass::CreatePipelineLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	// setup all layouts
	for (size_t i = 0; i < FSR2::MAX_SIZE; i++)
	{
		std::vector<VkDescriptorSetLayout> setLayouts
		{
			SetLayoutDB::compute_fsr2[i],
		};

		VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(
			setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

		VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &pushConstantRange;
		std::string name(fsr_shaders_names[i]);
		name += "_PSOLayout";
		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::fsr2_PSOLayouts[i]));
		VK_NAME(m_device.logicalDevice, name.c_str(), PSOLayoutDB::fsr2_PSOLayouts[i]);
	}
}


void FSR2Pass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
}

void FSR2Pass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;


	VkComputePipelineCreateInfo computeCI;

	for (size_t i = 0; i < FSR2::MAX_SIZE; i++)
	{
		VkPipeline& pipe = pso_fsr2[i];
		if (pipe != VK_NULL_HANDLE) 
		{
			vkDestroyPipeline(m_device.logicalDevice, pipe, nullptr);
		}
		const char* shader = fsr_shaders[i];
		computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::fsr2_PSOLayouts[i]);
		computeCI.stage = vr.LoadShader(m_device, shader, VK_SHADER_STAGE_COMPUTE_BIT);
		VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pipe));
		std::string name(fsr_shaders_names[i]);
		name += "_PSO";		
		VK_NAME(m_device.logicalDevice, name.c_str(), &pipe);
		vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr); // destroy shader
	}
	
	
}
