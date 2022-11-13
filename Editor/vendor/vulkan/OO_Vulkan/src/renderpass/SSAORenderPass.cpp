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
#include "SSAORenderPass.h"

#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanUtils.h"

#include "GBufferRenderPass.h"
#include "ShadowPass.h"

#include <array>
#include <random>

DECLARE_RENDERPASS(SSAORenderPass);

void SSAORenderPass::Init()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainext = vr.m_swapchain.swapChainExtent;
	SSAO_renderTarget.name = "SSAO_COL";
	SSAO_renderTarget.forFrameBuffer(&vr.m_device, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
		swapchainext.width, swapchainext.height);

	InitRandomFactors();

	SetupRenderpass();

}

void SSAORenderPass::CreatePSO()
{
	CreateDescriptors();
	CreatePipelineLayout();
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

void SSAORenderPass::Draw()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainIdx = vr.swapchainIdx;
	auto* windowPtr = vr.windowPtr;

	const VkCommandBuffer cmdlist = vr.commandBuffers[swapchainIdx];
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("SSAO");

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = {0.0f,0.0f,0.0f,0.0f };

	//Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderpass_SSAO;                  //render pass to begin
	renderPassBeginInfo.renderArea.offset = { 0,0 };                                     //start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = vr.m_swapchain.swapChainExtent; //size of region to run render pass on (Starting from offset)
	renderPassBeginInfo.pClearValues = clearValues.data();                               //list of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer =  vr.swapChainFramebuffers[swapchainIdx];

	VkFramebuffer currentFB;
	FramebufferBuilder::Begin(&vr.fbCache)
		.BindImage(&SSAO_renderTarget)
		//.BindImage(&vr.renderTargets[vr.renderTargetInUseID].depth) //no depth
		.Build(currentFB,renderpass_SSAO);
	renderPassBeginInfo.framebuffer = currentFB;

	// transition depth buffer
	auto gbuffer = RenderPassDatabase::GetRenderPass<GBufferRenderPass>();
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		gbuffer->attachments[GBufferAttachmentIndex::DEPTH].image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		gbuffer->attachments[GBufferAttachmentIndex::NORMAL].image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	rhi::CommandList cmd{ cmdlist };
	cmd.SetDefaultViewportAndScissor();

	CreateDescriptors();
	cmd.BindPSO(pso_SSAO);

	SSAOPC pc{};
	pc.screenDim.x = vr.m_swapchain.swapChainExtent.width;
	pc.screenDim.y = vr.m_swapchain.swapChainExtent.height;
	pc.sampleDim.x = 4;
	pc.sampleDim.y = 4;
	pc.radius = vr.currWorld->ssaoSettings.radius;
	pc.bias = vr.currWorld->ssaoSettings.bias;
	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(SSAOPC);

	cmd.SetPushConstant(PSOLayoutDB::SSAOPSOLayout,range,&pc);

	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
		vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	cmd.BindDescriptorSet(PSOLayoutDB::SSAOPSOLayout, 0,
		std::array<VkDescriptorSet, 2>
		{
			vr.descriptorSet_SSAO,
			vr.descriptorSets_uniform[swapchainIdx],
		},
		1,&dynamicOffset
	);

	cmd.DrawFullScreenQuad();

	vkCmdEndRenderPass(cmdlist);

	// wait for SSAO buffer before next pass
	//auto ssao = RenderPassDatabase::GetRenderPass<SSAORenderPass>();
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		SSAO_renderTarget.image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

}

void SSAORenderPass::Shutdown()
{
	auto& device = VulkanRenderer::get()->m_device.logicalDevice;

	vkDestroyPipelineLayout(device, PSOLayoutDB::SSAOPSOLayout, nullptr);
	vkDestroyRenderPass(device, renderpass_SSAO, nullptr);
	SSAO_renderTarget.destroy();
	randomNoise_texture.destroy();
	randomVectorsSSBO.destroy();
	vkDestroyPipeline(device, pso_SSAO, nullptr);
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
		float scale = (float)i / 64.0; 
		scale	= lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);  
	}
	randomVectorsSSBO.Init(&vr.m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	randomVectorsSSBO.reserve(ssaoKernel.size());
	randomVectorsSSBO.writeTo(ssaoKernel.size(), ssaoKernel.data());

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

	randomNoise_texture.fromBuffer(ssaoNoise.data(), ssaoNoise.size() * sizeof(glm::vec4), VK_FORMAT_R32G32B32A32_SFLOAT,
		width,height,copies,&vr.m_device,vr.m_device.graphicsQueue,VK_FILTER_NEAREST);
}


void SSAORenderPass::CreateDescriptors()
{
	//if (m_log)
	//{
	//	std::cout << __FUNCSIG__ << std::endl;
	//}

	auto& vr = *VulkanRenderer::get();
	// At this point, all dependent resources (gbuffer etc) must be ready.
	auto gbuffer = RenderPassDatabase::GetRenderPass<GBufferRenderPass>();
	assert(gbuffer != nullptr);
	// Image descriptors for the offscreen color attachments
	// VkDescriptorImageInfo texDescriptorPosition = oGFX::vkutils::inits::descriptorImageInfo(
	//     GfxSamplerManager::GetSampler_Deferred(),
	// 	gbuffer->attachments[GBufferAttachmentIndex::POSITION].view,
	//     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorDepth = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_SSAOEdgeClamp(),
		gbuffer->attachments[GBufferAttachmentIndex::DEPTH]   .view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorNormal = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_Deferred(),
		gbuffer->attachments[GBufferAttachmentIndex::NORMAL]  .view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	//VkDescriptorImageInfo texDescriptorSSAO = oGFX::vkutils::inits::descriptorImageInfo(
	//	GfxSamplerManager::GetSampler_Deferred(),
	//	SSAO_texture .view,
	//	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkDescriptorImageInfo texDescriptorNoise = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		randomNoise_texture.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	const auto& ranvecBufer = randomVectorsSSBO.GetDescriptorBufferInfo();

	DescriptorBuilder::Begin(&vr.DescLayoutCache,&vr.descAllocs[vr.swapchainIdx])
		//.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // to remove
		.BindImage(1, &texDescriptorDepth, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // we construct world position using depth
		.BindImage(2, &texDescriptorNormal, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.BindImage(3, &texDescriptorNoise, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.BindBuffer(4, &ranvecBufer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build(vr.descriptorSet_SSAO, SetLayoutDB::SSAO);
}

void SSAORenderPass::CreatePipelineLayout()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

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

void SSAORenderPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	// ATTACHMENTS
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = SSAO_renderTarget.format; // R32_F 
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; 
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

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
	VK_CHK(vkCreateRenderPass(vr.m_device.logicalDevice, &renderPassCreateInfo, nullptr, &renderpass_SSAO));
	VK_NAME(vr.m_device.logicalDevice, "Renderpass_SSAO",renderpass_SSAO);

}

void SSAORenderPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/SSAO.vert.spv";
	const char* shaderPS = "Shaders/bin/SSAO.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT , VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::SSAOPSOLayout, renderpass_SSAO);
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
	pipelineCI.renderPass = renderpass_SSAO;
	pipelineCI.layout = PSOLayoutDB::SSAOPSOLayout;
	colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_SSAO));
	VK_NAME(m_device.logicalDevice, "SSAO_PSO", pso_SSAO);

	vkDestroyShaderModule(m_device.logicalDevice,shaderStages[0].module , nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}
