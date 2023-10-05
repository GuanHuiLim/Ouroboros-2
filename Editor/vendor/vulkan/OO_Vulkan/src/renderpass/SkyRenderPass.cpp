/************************************************************************************//*!
\file           SkyRenderPass.cpp
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

#include "MathCommon.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"

#include "../shaders/shared_structs.h"

#include <array>

struct SkyRenderPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(SkyRenderPass)

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

DECLARE_RENDERPASS(SkyRenderPass);


//VkPushConstantRange pushConstantRange;
VkPipeline pso_skyPass{};

void SkyRenderPass::Init()
{
	SetupResources();
	CreatePSOLayout();
	SetupRenderpass();
	SetupFramebuffer();
}

void SkyRenderPass::CreatePSO()
{
	CreatePipeline();
}

bool SkyRenderPass::SetupDependencies()
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

void SkyRenderPass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();
	lastCmd = cmdlist;
	if (!vr.deferredRendering)
		return;
	auto& device = vr.m_device;
	auto currFrame = vr.getFrame();

    PROFILE_GPU_CONTEXT(cmdlist);

	rhi::CommandList cmd{ cmdlist, "SkyRenderPass" };

	PROFILE_GPU_EVENT("SkyPass");	



	auto& attachments = vr.attachments.gbuffer;

	constexpr bool clearOnDraw = true;
	cmd.BindAttachment(0, &vr.attachments.lighting_target);
	cmd.BindDepthAttachment(&attachments[GBufferAttachmentIndex::DEPTH]);
	
	cmd.BindPSO(pso_skyPass, PSOLayoutDB::skypassPSOLayout);
	cmd.SetDefaultViewportAndScissor();
	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
												vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	cmd.DescriptorSetBegin(0)
		.BindSampler(0, GfxSamplerManager::GetSampler_Cube())
		.BindImage(1, &vr.g_cubeMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

	LightPC pc{};
	pc.ambient = vr.currWorld->lightSettings.ambient;
	VkPushConstantRange pcr{};
	pcr.size = sizeof(LightPC);
	pcr.stageFlags = VK_SHADER_STAGE_ALL;

	cmd.SetPushConstant(PSOLayoutDB::skypassPSOLayout, pcr, &pc);

	cmd.BindDescriptorSet(PSOLayoutDB::skypassPSOLayout, 1, 1, &vr.descriptorSets_uniform[currFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &dynamicOffset);

	cmd.DrawFullScreenQuad();

}

void SkyRenderPass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& device = m_device.logicalDevice;

	vkDestroyPipeline(device, pso_skyPass, nullptr);
	vkDestroyPipelineLayout(device, PSOLayoutDB::skypassPSOLayout, nullptr);
}

void SkyRenderPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;


}

void SkyRenderPass::SetupFramebuffer()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;
	
}


void SkyRenderPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/farplaneFullscreen.vert.spv";
	const char* shaderPS = "Shaders/bin/sky.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages =
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_GREATER_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::skypassPSOLayout, VK_NULL_HANDLE);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	// write to stencil buffer
	depthStencilState.stencilTestEnable = VK_TRUE;
	depthStencilState.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;
	depthStencilState.back.compareMask = 0xff;
	depthStencilState.back.writeMask = 0x00;
	depthStencilState.back.reference = 1; // check for anything greater than zero
	depthStencilState.front = depthStencilState.back;
	// maybe disable depth test
	depthStencilState.depthWriteEnable = VK_FALSE;

	const auto& bindingDescription = oGFX::GetGFXVertexInputBindings();
	const auto& attributeDescriptions = oGFX::GetGFXVertexInputAttributes();

	VkPipelineVertexInputStateCreateInfo vertexInput = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo();
	pipelineCI.pVertexInputState = &vertexInput; // fullscreen empty vertex imput


	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, 1> blendAttachmentStates =
	{
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};

	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &vr.G_HDR_FORMAT;
	renderingInfo.depthAttachmentFormat = vr.G_DEPTH_FORMAT;
	renderingInfo.stencilAttachmentFormat =  vr.G_DEPTH_FORMAT;
	
	pipelineCI.pNext = &renderingInfo;

	if (pso_skyPass != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_skyPass, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_skyPass));
	VK_NAME(m_device.logicalDevice, "Skypass_PSO", pso_skyPass);

	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);

}

void SkyRenderPass::CreatePSOLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	VkDescriptorImageInfo sampler{};
	sampler.sampler = GfxSamplerManager::GetSampler_Cube(); // MAYBE CHANGE THIS??

	VkDescriptorImageInfo cubeMap{};
	cubeMap.imageLayout = vr.g_cubeMap.referenceLayout;
	cubeMap.imageView = vr.g_cubeMap.view;

	DescriptorBuilder::Begin()
		.BindImage(0, &sampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(1, &cubeMap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BuildLayout(SetLayoutDB::skypass);

	{
		std::vector<VkDescriptorSetLayout> setLayouts
		{
			SetLayoutDB::skypass,
			SetLayoutDB::FrameUniform,
		};

		VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(
			setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

		VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &pushConstantRange;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::skypassPSOLayout));
		VK_NAME(m_device.logicalDevice, "skypassPSOLayout", PSOLayoutDB::skypassPSOLayout);
	}	
	
}

void SkyRenderPass::SetupResources() {

	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;


}