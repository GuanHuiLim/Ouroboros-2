/************************************************************************************//*!
\file           SSAORenderPass.cpp
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


struct SSAORenderPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(SSAORenderPass)

	void Init() override;
	void Draw(const VkCommandBuffer cmdlist) override;
	void Shutdown() override;

	void InitRandomFactors();

	bool SetupDependencies() override;

	void CreatePSO() override;
	void CreatePipelineLayout();
	void CreateDescriptors();

private:
	void SetupRenderpass();
	void CreatePipeline();

};

DECLARE_RENDERPASS(SSAORenderPass);



VulkanRenderpass renderpass_SSAO{};

//VkPushConstantRange pushConstantRange;
VkPipeline pso_SSAO{};
// TODO: compute i guess
VkPipeline pso_SSAO_blur{};

GpuVector<glm::vec3> randomVectorsSSBO;

std::vector<glm::vec4> ssaoNoise;
std::vector<glm::vec3> ssaoKernel;

void SSAORenderPass::Init()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainext = vr.m_swapchain.swapChainExtent;
	vr.attachments.SSAO_renderTarget.name = "SSAO_COL";
	vr.attachments.SSAO_renderTarget.forFrameBuffer(&vr.m_device, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		swapchainext.width, swapchainext.height, true, 0.5f);
	vr.fbCache.RegisterFramebuffer(vr.attachments.SSAO_renderTarget);

	vr.attachments.SSAO_finalTarget.name = "SSAO_FINAL";
	vr.attachments.SSAO_finalTarget.forFrameBuffer(&vr.m_device, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		swapchainext.width, swapchainext.height, true, 1.0f); // full scale image
	vr.fbCache.RegisterFramebuffer(vr.attachments.SSAO_finalTarget);

	auto cmd = vr.GetCommandBuffer();
	vkutils::SetImageInitialState(cmd, vr.attachments.SSAO_renderTarget);
	vkutils::SetImageInitialState(cmd, vr.attachments.SSAO_finalTarget);

	vr.SubmitSingleCommandAndWait(cmd);

	InitRandomFactors();

	SetupRenderpass();

}

void SSAORenderPass::CreatePSO()
{
	
	CreatePipeline(); // Dependency on GBuffer Init()
}

bool SSAORenderPass::SetupDependencies()
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

void SSAORenderPass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;


	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("SSAO");

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = { 0.0f,0.0f,0.0f,0.0f };


	// transition depth buffer
	auto& attachments = vr.attachments.gbuffer;

	
	rhi::CommandList cmd{ cmdlist, "SSAO Pass"};
	std::array<VkViewport, 1>viewports{ VkViewport{0,vr.attachments.SSAO_renderTarget.height * 1.0f,vr.attachments.SSAO_renderTarget.width * 1.0f,vr.attachments.SSAO_renderTarget.height * -1.0f} };

	CreateDescriptors();
	cmd.BindPSO(pso_SSAO, PSOLayoutDB::SSAOPSOLayout);
	cmd.BindAttachment(0, &vr.attachments.SSAO_renderTarget);
	cmd.SetDefaultViewportAndScissor();
	cmd.SetViewport(0, static_cast<uint32_t>(viewports.size()), viewports.data());

	SSAOPC pc{};
	pc.screenDim.x = static_cast<float>(vr.attachments.SSAO_renderTarget.width);
	pc.screenDim.y = static_cast<float>(vr.attachments.SSAO_renderTarget.height);
	pc.sampleDim.x = 4;
	pc.sampleDim.y = 4;
	pc.radius = vr.currWorld->ssaoSettings.radius;
	pc.bias = vr.currWorld->ssaoSettings.bias;
	pc.intensity = vr.currWorld->ssaoSettings.intensity;
	pc.numSamples = std::clamp<uint32_t>(vr.currWorld->ssaoSettings.samples, 1, 64);
	VkPushConstantRange range{};
	range.offset = 0;
	range.size = sizeof(SSAOPC);

	cmd.SetPushConstant(PSOLayoutDB::SSAOPSOLayout, range, &pc);

	const auto& ranvecBufer = randomVectorsSSBO.GetDescriptorBufferInfo();
	cmd.DescriptorSetBegin(0)
		.BindSampler(0, GfxSamplerManager::GetSampler_SSAOEdgeClamp()) // we construct world position using depth
		.BindImage(1, &attachments[GBufferAttachmentIndex::DEPTH], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) 
		.BindImage(2, &attachments[GBufferAttachmentIndex::NORMAL], VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(3, &vr.attachments.randomNoise_texture, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindBuffer(4, &ranvecBufer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO),
		vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	
	cmd.BindDescriptorSet(PSOLayoutDB::SSAOPSOLayout, 1, 1, &vr.descriptorSets_uniform[currFrame],VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &dynamicOffset);

	cmd.DrawFullScreenQuad();

	cmd.BindPSO(pso_SSAO_blur, PSOLayoutDB::SSAOBlurPSOLayout);
	cmd.BindAttachment(0, &vr.attachments.SSAO_finalTarget);
	cmd.SetDefaultViewportAndScissor();
	cmd.BindDescriptorSet(PSOLayoutDB::SSAOBlurPSOLayout, 1, 1, &vr.descriptorSets_uniform[currFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,1,&dynamicOffset);
	cmd.DescriptorSetBegin(0)
		.BindSampler(0, GfxSamplerManager::GetSampler_SSAOEdgeClamp())
		.BindImage(1, &vr.attachments.SSAO_renderTarget, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

	cmd.DrawFullScreenQuad();
	

	// wait for blurred image before next
	// vkutils::TransitionImage(cmdlist, vr.attachments.SSAO_finalTarget, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

void SSAORenderPass::Shutdown()
{

	auto& vr = *VulkanRenderer::get();
	auto& device = vr.m_device.logicalDevice;

	vkDestroyPipelineLayout(device, PSOLayoutDB::SSAOPSOLayout, nullptr);
	vkDestroyPipelineLayout(device, PSOLayoutDB::SSAOBlurPSOLayout, nullptr);
	renderpass_SSAO.destroy();
	vr.attachments.SSAO_renderTarget.destroy();
	vr.attachments.SSAO_finalTarget.destroy();
	vr.attachments.randomNoise_texture.destroy();
	randomVectorsSSBO.destroy();
	vkDestroyPipeline(device, pso_SSAO, nullptr);
	vkDestroyPipeline(device, pso_SSAO_blur, nullptr);
}

void SSAORenderPass::InitRandomFactors()
{
	auto& vr = *VulkanRenderer::get();

	auto lerp = [](float a, float b, float f)->float
	{
		return a + f * (b - a);
	};

	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	ssaoKernel.reserve(64);
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator)
		);
		sample  = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / 64.0f; 
		scale	= lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);  
	}
	randomVectorsSSBO.Init(&vr.m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	randomVectorsSSBO.reserve(ssaoKernel.size(),vr.m_device.graphicsQueue,vr.m_device.commandPoolManagers[vr.getFrame()].m_commandpool);
	// todo elegant way to do this
	randomVectorsSSBO.blockingWriteTo(ssaoKernel.size(), ssaoKernel.data(),vr.m_device.graphicsQueue,vr.m_device.commandPoolManagers[vr.getFrame()].m_commandpool);

	uint32_t width = 4;
	uint32_t height = 4;
	uint32_t numSamples = width*height;

	for (unsigned int i = 0; i < numSamples; i++)
	{
		glm::vec4 noise(
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator) * 2.0 - 1.0, 
			0.0f,
			0.0f); 
		ssaoNoise.push_back(noise);
	}  

	VkBufferImageCopy copyRegion{};
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.bufferOffset = 0;
	copyRegion.imageExtent.width = width;
	copyRegion.imageExtent.height = height;
	copyRegion.imageExtent.depth = 1;
	std::vector<VkBufferImageCopy> copies{copyRegion};

	vr.attachments.randomNoise_texture.fromBuffer(ssaoNoise.data(), ssaoNoise.size() * sizeof(glm::vec4), VK_FORMAT_R32G32B32A32_SFLOAT,
		width,height,copies,&vr.m_device,vr.m_device.graphicsQueue,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_FILTER_NEAREST);
}


void SSAORenderPass::CreateDescriptors()
{
	//if (m_log)
	//{
	//	std::cout << __FUNCSIG__ << std::endl;
	//}

	auto& vr = *VulkanRenderer::get();
	// At this point, all dependent resources (gbuffer etc) must be ready.
	auto& attachments = vr.attachments.gbuffer;
	
	VkDescriptorImageInfo texDescriptorDepth = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_SSAOEdgeClamp(),
		attachments[GBufferAttachmentIndex::DEPTH]   .view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorNormal = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		attachments[GBufferAttachmentIndex::NORMAL]  .view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorNoise = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		vr.attachments.randomNoise_texture.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo sampler = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		VK_NULL_HANDLE,
		VK_IMAGE_LAYOUT_GENERAL);

	const auto& ranvecBufer = randomVectorsSSBO.GetDescriptorBufferInfo();

	DescriptorBuilder::Begin()
		.BindImage(0, &sampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS) 
		.BindImage(1, &texDescriptorDepth, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(2, &texDescriptorNormal, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(3, &texDescriptorNoise, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindBuffer(4, &ranvecBufer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BuildLayout(SetLayoutDB::SSAO);

	VkDescriptorImageInfo texDescriptorSSAO = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_SSAOEdgeClamp(),
		vr.attachments.SSAO_renderTarget.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	DescriptorBuilder::Begin()
		.BindImage(0, &sampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(1, &texDescriptorSSAO, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BuildLayout(SetLayoutDB::SSAOBlur);

}

void SSAORenderPass::CreatePipelineLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	{
		std::vector<VkDescriptorSetLayout> setLayouts
		{
			SetLayoutDB::SSAO, // (set = 0)
			SetLayoutDB::FrameUniform, // (set = 1)
		};

		VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));
		VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &pushConstantRange;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::SSAOPSOLayout));
		VK_NAME(m_device.logicalDevice, "SSAO_PSOLayout", PSOLayoutDB::SSAOPSOLayout);
	}

	{

		std::vector<VkDescriptorSetLayout> setLayouts
		{
			SetLayoutDB::SSAOBlur, // (set = 0)
			SetLayoutDB::FrameUniform, // (set = 1)
		};

		VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));
		VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
		plci.pushConstantRangeCount = 1;
		plci.pPushConstantRanges = &pushConstantRange;

		VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::SSAOBlurPSOLayout));
		VK_NAME(m_device.logicalDevice, "SSAO_BlurLayout", PSOLayoutDB::SSAOBlurPSOLayout);
	}
}

void SSAORenderPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	// ATTACHMENTS
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = vr.attachments.SSAO_renderTarget.format; // R32_F 
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; 
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// REFERENCES 
	VkAttachmentReference  colourAttachmentReference = {};
	colourAttachmentReference.attachment = 0;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//information about a particular subpass the render pass is using
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; 
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentReference;
	subpass.pDepthStencilAttachment = VK_NULL_HANDLE; // no depth

	// Need to determine when layout transitions occur using subpass dependancies
	std::array<VkSubpassDependency, 1> subpassDependancies;

	//conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	subpassDependancies[0].srcSubpass = VK_SUBPASS_EXTERNAL; 
	subpassDependancies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; 
	subpassDependancies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; 
	subpassDependancies[0].dstSubpass = 0;
	subpassDependancies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependancies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependancies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::array<VkAttachmentDescription, 1> renderpassAttachments = { colourAttachment };

	//create info for render pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderpassAttachments.size());
	renderPassCreateInfo.pAttachments = renderpassAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependancies.size());
	renderPassCreateInfo.pDependencies = subpassDependancies.data();
	
	subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
	renderPassCreateInfo.dependencyCount = 0; // colour only

	renderpass_SSAO.name = "Renderpass_SSAO";
	renderpass_SSAO.Init(vr.m_device, renderPassCreateInfo);

	CreateDescriptors();
	CreatePipelineLayout();
}

void SSAORenderPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/genericFullscreen.vert.spv";
	const char* shaderPS = "Shaders/bin/SSAO.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT , VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, vr.G_DEPTH_COMPARISON);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::SSAOPSOLayout, renderpass_SSAO.pass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	// Empty vertex input state, vertices are generated by the vertex shader
	VkPipelineVertexInputStateCreateInfo emptyInputState = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo();
	pipelineCI.pVertexInputState = &emptyInputState;
	//pipelineCI.renderPass = renderpass_SSAO.pass;
	pipelineCI.renderPass = nullptr;
	pipelineCI.layout = PSOLayoutDB::SSAOPSOLayout;
	colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

	VkFormat format = vr.attachments.SSAO_renderTarget.format;
	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &format;
	renderingInfo.depthAttachmentFormat = {};
	renderingInfo.stencilAttachmentFormat = {};

	pipelineCI.pNext = &renderingInfo;

	if (pso_SSAO != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_SSAO, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_SSAO));
	VK_NAME(m_device.logicalDevice, "SSAO_PSO", pso_SSAO);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr); // destroy fragment

	shaderStages[1] = vr.LoadShader(m_device, "Shaders/bin/ssaoBlur.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineCI.layout = PSOLayoutDB::SSAOBlurPSOLayout;
	if (pso_SSAO_blur != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_SSAO_blur, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_SSAO_blur));
	VK_NAME(m_device.logicalDevice, "SSAO_PSO_blur", pso_SSAO_blur);

	vkDestroyShaderModule(m_device.logicalDevice,shaderStages[0].module , nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr); // destroy fragment
}
