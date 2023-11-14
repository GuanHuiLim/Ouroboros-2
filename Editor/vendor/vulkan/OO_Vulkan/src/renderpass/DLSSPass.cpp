/************************************************************************************//*!
\file           DLSSPass.cpp
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

#include <array>
#include <random>

struct DLSSPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(DLSSPass)

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

DECLARE_RENDERPASS(DLSSPass);

void DLSSPass::Init()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainext = vr.m_swapchain.swapChainExtent;

	SetupDependencies();
	
	CreateDescriptors();
	CreatePipelineLayout();
	CreatePSO();

}

void DLSSPass::CreatePSO()
{	
	CreatePipeline();
}

bool DLSSPass::SetupDependencies()
{
	auto& vr = *VulkanRenderer::get();
	constexpr size_t MAX_FRAMES = 2;


	return true;
}

void DLSSPass::Draw(const VkCommandBuffer cmdlist)
{
	VulkanRenderer& vr = *VulkanRenderer::get();
	uint32_t currFrame = vr.getFrame();
	uint32_t prevFrame = vr.getPreviousFrame();
	Window* windowPtr = vr.windowPtr;
	
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("DLSS");
	rhi::CommandList cmd{ cmdlist, "DLSS",{1,0,0,0.5} };
	lastCmd = cmdlist;

	vr.PrepareDLSS();

	cmd.DescriptorSetBegin(0)
		.BindImage(0, &vr.attachments.lighting_target, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(1, &vr.attachments.fullres_HDR, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		.BindImage(2, &vr.attachments.gbuffer[VELOCITY], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(3, &vr.attachments.gbuffer[DEPTH], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(4, &vr.attachments.fsr_exposure_mips, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
		;

	// transition to layout
	cmd.VerifyImageResourceStates();
	
	vr.m_NGX.EvaluateSuperSampling(cmdlist
		, &vr.attachments.lighting_target
		, &vr.attachments.fullres_HDR
		, &vr.attachments.gbuffer[VELOCITY]
		, &vr.attachments.gbuffer[DEPTH]
		, &vr.attachments.fsr_exposure_mips
		, VkViewport{ 0,0,(float)vr.renderWidth,(float)vr.renderHeight,0,0 }
		,false,false
		, { vr.jitterX,vr.jitterY }
	, { vr.renderWidth ,vr.renderHeight});

}



void DLSSPass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& device = vr.m_device.logicalDevice;

	
}

void DLSSPass::CreateDescriptors()
{

	auto& vr = *VulkanRenderer::get();
	auto& target = vr.renderTargets[vr.renderTargetInUseID].texture;
	auto currFrame = vr.getFrame();

	
	
}

void DLSSPass::CreatePipelineLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	
}


void DLSSPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
}

void DLSSPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

}
