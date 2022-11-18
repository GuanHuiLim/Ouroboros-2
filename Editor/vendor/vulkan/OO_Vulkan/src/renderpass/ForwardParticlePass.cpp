/************************************************************************************//*!
\file           ForwardParticlePass.cpp
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
#include "ForwardParticlePass.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "FramebufferCache.h"
#include "FramebufferBuilder.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"

#include "GraphicsWorld.h"
#include "DeferredCompositionRenderpass.h"
#include "GBufferRenderPass.h"

#include <array>

DECLARE_RENDERPASS(ForwardParticlePass);

void ForwardParticlePass::Init()
{
	SetupRenderpass();
	SetupFramebuffer();
}

void ForwardParticlePass::CreatePSO()
{
	CreatePipeline();
}

bool ForwardParticlePass::SetupDependencies()
{
	// TODO: If gbuffer rendering is disabled, return false.

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

void ForwardParticlePass::Draw()
{
	auto& vr = *VulkanRenderer::get();
	if (!vr.deferredRendering)
		return;

	auto& device = vr.m_device;
	auto& swapchain = vr.m_swapchain;
	auto& commandBuffers = vr.commandBuffers;
	auto& swapchainIdx = vr.swapchainIdx;
	auto* windowPtr = vr.windowPtr;

    const VkCommandBuffer cmdlist = commandBuffers[swapchainIdx];
    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("ForwardParticles");

	constexpr VkClearColorValue zeroFloat4 = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
	VkClearColorValue rMinusOne = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
	rMinusOne.int32[0] = -1;

	auto& attachments = RenderPassDatabase::GetRenderPass<GBufferRenderPass>()->attachments;

	assert(attachments[GBufferAttachmentIndex::DEPTH].currentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		attachments[GBufferAttachmentIndex::DEPTH].image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_WRITE_BIT|VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 });
	attachments[GBufferAttachmentIndex::DEPTH].currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	// Clear values for all attachments written in the fragment shader
	std::array<VkClearValue, GBufferAttachmentIndex::MAX_ATTACHMENTS> clearValues;
	//clearValues[GBufferAttachmentIndex::POSITION].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::NORMAL]  .color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::ALBEDO]  .color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::MATERIAL].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::ENTITY_ID].color = rMinusOne;
	clearValues[GBufferAttachmentIndex::DEPTH]   .depthStencil = { 1.0f, 0 };

	assert(vr.renderTargets[vr.renderTargetInUseID].texture.currentLayout == VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		vr.renderTargets[vr.renderTargetInUseID].texture.image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_WRITE_BIT|VK_ACCESS_MEMORY_READ_BIT,
		vr.renderTargets[vr.renderTargetInUseID].texture.currentLayout,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT ,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	vr.renderTargets[vr.renderTargetInUseID].texture.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkFramebuffer currentFB;
	FramebufferBuilder::Begin(&vr.fbCache)
		.BindImage(&vr.renderTargets[vr.renderTargetInUseID].texture)
		.BindImage(&attachments[GBufferAttachmentIndex::ENTITY_ID])
		.BindImage(&attachments[GBufferAttachmentIndex::DEPTH])
		.Build(currentFB,renderpass_GbufferSecondsPass);

	// Manually set layout for blit reason

	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass =  renderpass_GbufferSecondsPass.pass;
	renderPassBeginInfo.framebuffer = currentFB;
	renderPassBeginInfo.renderArea.extent.width = swapchain.swapChainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapchain.swapChainExtent.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// vr.ResizeSwapchain() destroys the depth attachment. This causes the renderpass to fail on resize
	// TODO: handle all framebuffer resizes gracefully
	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	rhi::CommandList cmd{ cmdlist };
	cmd.SetDefaultViewportAndScissor();

	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
																												vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	cmd.BindDescriptorSet(PSOLayoutDB::defaultPSOLayout, 0, 
		std::array<VkDescriptorSet, 3>{
		vr.descriptorSet_gpuscene,
			vr.descriptorSets_uniform[swapchainIdx],
			vr.descriptorSet_bindless,
	},
		1, & dynamicOffset
	);

	cmd.BindPSO(pso_GBufferParticles);
	// Bind merged mesh vertex & index buffers, instancing buffers.
	std::vector<VkBuffer> vtxBuffers{
		vr.g_GlobalMeshBuffers.VtxBuffer.getBuffer(),
		vr.skinningVertexBuffer.getBuffer(),
	};

	VkDeviceSize offsets[2]{
		0,
		0
	};
	cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, vr.g_GlobalMeshBuffers.VtxBuffer.getBufferPtr());
	cmd.BindIndexBuffer(vr.g_GlobalMeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	cmd.BindVertexBuffer(BIND_POINT_INSTANCE_BUFFER_ID, 1, vr.g_particleDatas[swapchainIdx].getBufferPtr());
	cmd.DrawIndexedIndirect(vr.g_particleCommandsBuffer.getBuffer(), 0, vr.g_particleCommandsBuffer.size());

	vkCmdEndRenderPass(cmdlist);
}

void ForwardParticlePass::Shutdown()
{
	auto& device = VulkanRenderer::get()->m_device.logicalDevice;

	renderpass_GbufferSecondsPass.destroy();
	vkDestroyPipeline(device, pso_GBufferParticles, nullptr);
}

void ForwardParticlePass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	// Set up separate renderpass with references to the color and depth attachments
	std::array<VkAttachmentDescription, 3> attachmentDescs = {};

	// Init attachment properties
		attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		
		attachmentDescs[2].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[2].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescs[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescs[2].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescs[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		auto& attachments = RenderPassDatabase::GetRenderPass<GBufferRenderPass>()->attachments;
	// Formats
	//attachmentDescs[GBufferAttachmentIndex::POSITION].format = attachments[GBufferAttachmentIndex::POSITION].format;
	attachmentDescs[0]  .format = m_swapchain.swapChainImageFormat;
	attachmentDescs[1]  .format = attachments[GBufferAttachmentIndex::ENTITY_ID].format;
	attachmentDescs[2]  .format = vr.G_DEPTH_FORMAT;
	

	std::vector<VkAttachmentReference> colorReferences;
	//colorReferences.push_back({ GBufferAttachmentIndex::POSITION, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 0,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back(	{ 1,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 2;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	renderpass_GbufferSecondsPass.name = "ForwardParticlePass";
	renderpass_GbufferSecondsPass.Init(m_device, renderPassInfo);
}

void ForwardParticlePass::SetupFramebuffer()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	auto& attachments = RenderPassDatabase::GetRenderPass<GBufferRenderPass>()->attachments;

	//FramebufferBuilder::Begin(&vr.fbCache)
	//	//.BindImage(&attachments[GBufferAttachmentIndex::POSITION])
	//	.BindImage(&m_swapchain.swapChainImages[0])
	//	.BindImage(&attachments[GBufferAttachmentIndex::ENTITY_ID])
	//	.BindImage(&m_swapchain.depthAttachment)
	//	.Build(framebuffer_GBufferSecondPass,renderpass_GbufferSecondsPass);

	//VkFramebufferCreateInfo fbufCreateInfo = {};
	//fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//fbufCreateInfo.pNext = NULL;
	//fbufCreateInfo.renderPass = renderpass_GBuffer;
	//fbufCreateInfo.pAttachments = attachments.data();
	//fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	//fbufCreateInfo.width = vr.m_swapchain.swapChainExtent.width;
	//fbufCreateInfo.height = vr.m_swapchain.swapChainExtent.height;
	//fbufCreateInfo.layers = 1;
	//VK_CHK(vkCreateFramebuffer(vr.m_device.logicalDevice, &fbufCreateInfo, nullptr, &framebuffer_GBuffer));
	//VK_NAME(vr.m_device.logicalDevice, "particlesFB", framebuffer_GBufferSecondPass);
}


void ForwardParticlePass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/forwardParticles.vert.spv";
	const char* shaderPS = "Shaders/bin/forwardParticles.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages =
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_TRUE); // we want blending 
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::defaultPSOLayout, renderpass_GbufferSecondsPass.pass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	const auto& bindingDescription = std::vector<VkVertexInputBindingDescription>{	
		oGFX::vkutils::inits::vertexInputBindingDescription(BIND_POINT_VERTEX_BUFFER_ID,sizeof(oGFX::Vertex),VK_VERTEX_INPUT_RATE_VERTEX),
		oGFX::vkutils::inits::vertexInputBindingDescription(BIND_POINT_INSTANCE_BUFFER_ID,sizeof(ParticleData),VK_VERTEX_INPUT_RATE_INSTANCE),
	};
	const auto& attributeDescriptions = std::vector<VkVertexInputAttributeDescription>{
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(oGFX::Vertex, pos)), //Position attribute
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,1,VK_FORMAT_R32G32B32_SFLOAT,offsetof(oGFX::Vertex, norm)),//normals attribute
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,2,VK_FORMAT_R32G32B32_SFLOAT,offsetof(oGFX::Vertex, col)), // colour attribute
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,3,VK_FORMAT_R32G32B32_SFLOAT,offsetof(oGFX::Vertex, tangent)),//tangent attribute
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,4,VK_FORMAT_R32G32_SFLOAT,offsetof(oGFX::Vertex, tex)),    //Texture attribute

		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,5,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(ParticleData, transform)+0*sizeof(glm::vec4)),    //xform
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,6,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(ParticleData, transform)+1*sizeof(glm::vec4)),    //xform
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,7,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(ParticleData, transform)+2*sizeof(glm::vec4)),    //xform
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,8,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(ParticleData, transform)+3*sizeof(glm::vec4)),    //xform
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,9,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(ParticleData, colour)),    //col
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,10,VK_FORMAT_R32G32B32A32_UINT,offsetof(ParticleData, instanceData)),    //texindex, entityID

	};
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo(bindingDescription, attributeDescriptions);
	pipelineCI.pVertexInputState = &vertexInputCreateInfo;

	// Separate render pass
	pipelineCI.renderPass = renderpass_GbufferSecondsPass.pass;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, 2> blendAttachmentStates =
	{
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE), // albedo blend
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};
	blendAttachmentStates[0].blendEnable = VK_TRUE; 
	blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // save background albedo as well
	blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;

	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_GBufferParticles));
	VK_NAME(m_device.logicalDevice, "forwardParticlesPSO", pso_GBufferParticles);

	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}
