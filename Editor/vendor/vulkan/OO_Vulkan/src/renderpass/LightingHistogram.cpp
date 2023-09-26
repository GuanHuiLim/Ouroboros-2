/************************************************************************************//*!
\file           LightingHistogram.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a LightingHistogram

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "GfxRenderpass.h"

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


struct LightingHistogram : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(LightingHistogram)

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


DECLARE_RENDERPASS(LightingHistogram);


VkPipeline pso_LightingHistogram{};
VkPipeline pso_lightingCDFScan{};

void LightingHistogram::Init()
{
	SetupRenderpass();
	SetupFramebuffer();
	SetupDependencies();
}

void LightingHistogram::CreatePSO()
{
	CreatePipeline();
}

bool LightingHistogram::SetupDependencies()
{
	auto& vr = *VulkanRenderer::get();
	VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	oGFX::CreateBuffer(vr.m_device.m_allocator, sizeof(HistoStruct)
						, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, flags
						, vr.lightingHistogram);

	oGFX::CreateBuffer(vr.m_device.m_allocator, sizeof(LuminenceData)
		, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT|VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, flags
		, vr.LuminanceBuffer);
	
	oGFX::CreateBuffer(vr.m_device.m_allocator, sizeof(LuminenceData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, vr.LuminanceMonitor);
	VK_CHK(vmaMapMemory(vr.m_device.m_allocator,vr.LuminanceMonitor.alloc, &vr.monitorData));


	return true;
}

void LightingHistogram::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();

	
	auto& device = vr.m_device;
	auto& swapchain = vr.m_swapchain;
	//auto& commandBuffers = vr.commandBuffers;
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;

    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("LightingHistogram");
	
	rhi::CommandList cmd{ cmdlist, "LightingHistogram"};

	VkDescriptorBufferInfo dbi{};
	dbi.buffer = vr.lightingHistogram.buffer;
	dbi.offset = 0;
	dbi.range = VK_WHOLE_SIZE;

	VkPushConstantRange pcr{};
	pcr.size = sizeof(LuminencePC);
	pcr.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	


	vkCmdFillBuffer(cmdlist, vr.lightingHistogram.buffer, 0, VK_WHOLE_SIZE, 0);

	auto& target = vr.attachments.lighting_target;

	cmd.BindPSO(pso_LightingHistogram, PSOLayoutDB::histogramPSOLayout,VK_PIPELINE_BIND_POINT_COMPUTE);

	float minLogLum = -8.0f;
	float maxLogLum = 3.5f;
	float histogramParams[4] = {
		minLogLum,
		1.0f / (maxLogLum - minLogLum),
		target.width,
		target.height
	};
	cmd.SetPushConstant(PSOLayoutDB::histogramPSOLayout, pcr, &histogramParams);
	cmd.DescriptorSetBegin(0)
		.BindImage(0, &target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindBuffer(1, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rhi::UAV);
	cmd.Dispatch(target.width / 16 + 1, target.height / 16 + 1);

	float tau = 1.1f;
	float frameTime = vr.deltaTime;
	float timeCoeff = glm::clamp<float>(1.0f - glm::exp(-frameTime * tau), 0.0, 1.0);
	float avgParams[4] = {
		minLogLum,
		maxLogLum - minLogLum,
		timeCoeff,
		static_cast<float>(target.width * target.height),
	};

	VkDescriptorBufferInfo lumBufferInfo{};
	lumBufferInfo.buffer = vr.LuminanceBuffer.buffer;
	lumBufferInfo.offset = 0;
	lumBufferInfo.range = VK_WHOLE_SIZE;
	cmd.BindPSO(pso_lightingCDFScan, PSOLayoutDB::luminancePSOLayout,VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindBuffer(0, &lumBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rhi::UAV)
		.BindBuffer(1, &dbi, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rhi::UAV);
	cmd.SetPushConstant(PSOLayoutDB::luminancePSOLayout, pcr, &avgParams);
	cmd.Dispatch(1);
}

void LightingHistogram::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& device = vr.m_device.logicalDevice;

	vmaUnmapMemory(vr.m_device.m_allocator, vr.LuminanceMonitor.alloc);

	vmaDestroyBuffer(vr.m_device.m_allocator, vr.lightingHistogram.buffer, vr.lightingHistogram.alloc);
	vmaDestroyBuffer(vr.m_device.m_allocator, vr.LuminanceBuffer.buffer, vr.LuminanceBuffer.alloc);
	vmaDestroyBuffer(vr.m_device.m_allocator, vr.LuminanceMonitor.buffer, vr.LuminanceMonitor.alloc);

	vkDestroyPipelineLayout(device, PSOLayoutDB::histogramPSOLayout, nullptr);
	vkDestroyPipelineLayout(device, PSOLayoutDB::luminancePSOLayout, nullptr);
	vkDestroyPipeline(device, pso_LightingHistogram, nullptr);
	vkDestroyPipeline(device, pso_lightingCDFScan, nullptr);
}

void LightingHistogram::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	VkDescriptorImageInfo dummy{};
	VkDescriptorBufferInfo dummybuf{};

	DescriptorBuilder::Begin()
		.BindImage(0, &dummy, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(1, &dummybuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_histogram);
	
	VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
	
	VkPipelineLayoutCreateInfo plci{};
	plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(&SetLayoutDB::compute_histogram, 1);
	plci.pushConstantRangeCount = 1;
	plci.pPushConstantRanges = &pushConstantRange;
	vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::histogramPSOLayout);
	VK_NAME(m_device.logicalDevice, "histogramPSOLayout", PSOLayoutDB::histogramPSOLayout);

	DescriptorBuilder::Begin()
		.BindBuffer(0, &dummybuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(1, &dummybuf, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_luminance);

	plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(&SetLayoutDB::compute_luminance, 1);
	plci.pushConstantRangeCount = 1;
	plci.pPushConstantRanges = &pushConstantRange;
	vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::luminancePSOLayout);
	VK_NAME(m_device.logicalDevice, "luminancePSOLayout", PSOLayoutDB::luminancePSOLayout);
}

void LightingHistogram::SetupFramebuffer()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
}

void LightingHistogram::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	VkComputePipelineCreateInfo computeCI{};
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::histogramPSOLayout);
	const char* histoShader=  "Shaders/bin/histogram.comp.spv";
	computeCI.stage = vr.LoadShader(m_device, histoShader, VK_SHADER_STAGE_COMPUTE_BIT);
	
	if (pso_LightingHistogram != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_LightingHistogram, nullptr);
	}
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_LightingHistogram));
	VK_NAME(m_device.logicalDevice, "pso_LightingHistogram", pso_LightingHistogram);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr);

	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::luminancePSOLayout);
	const char* cdfShader=  "Shaders/bin/cdfscan.comp.spv";
	computeCI.stage = vr.LoadShader(m_device, cdfShader, VK_SHADER_STAGE_COMPUTE_BIT);
	if (pso_lightingCDFScan != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_lightingCDFScan, nullptr);
	}
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_lightingCDFScan));
	VK_NAME(m_device.logicalDevice, "pso_lightingCDFScan", pso_lightingCDFScan);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr);

}

