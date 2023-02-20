/************************************************************************************//*!
\file           GBufferRenderPass.cpp
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
#include "GBufferRenderPass.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "FramebufferCache.h"
#include "FramebufferBuilder.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"

#include "DeferredCompositionRenderpass.h"

#include <array>

DECLARE_RENDERPASS(GBufferRenderPass);

void GBufferRenderPass::Init()
{
	SetupRenderpass();
	SetupFramebuffer();
}

void GBufferRenderPass::CreatePSO()
{
	CreatePipeline();
}

bool GBufferRenderPass::SetupDependencies()
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

void GBufferRenderPass::Draw()
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
    PROFILE_GPU_EVENT("GBuffer");

	constexpr VkClearColorValue zeroFloat4 = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
	constexpr VkClearColorValue tangentNormal = VkClearColorValue{ 0.5f,0.5f,1.0f,0.0f };
	VkClearColorValue rMinusOne = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
	rMinusOne.int32[0] = -1;

	// Clear values for all attachments written in the fragment shader
	std::array<VkClearValue, GBufferAttachmentIndex::MAX_ATTACHMENTS> clearValues;
	//clearValues[GBufferAttachmentIndex::POSITION].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::NORMAL]  .color = tangentNormal;
	clearValues[GBufferAttachmentIndex::ALBEDO].color =	  zeroFloat4;
	clearValues[GBufferAttachmentIndex::MATERIAL].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::ENTITY_ID].color = rMinusOne;
	clearValues[GBufferAttachmentIndex::DEPTH]   .depthStencil = { 1.0f, 0 };
	
	VkFramebuffer currentFB;
	FramebufferBuilder::Begin(&vr.fbCache)
		//.BindImage(&attachments[GBufferAttachmentIndex::POSITION])
		.BindImage(&attachments[GBufferAttachmentIndex::NORMAL  ])
		.BindImage(&attachments[GBufferAttachmentIndex::ALBEDO  ])
		.BindImage(&attachments[GBufferAttachmentIndex::MATERIAL])
		.BindImage(&attachments[GBufferAttachmentIndex::ENTITY_ID])
		.BindImage(&attachments[GBufferAttachmentIndex::DEPTH   ])
		.Build(currentFB,renderpass_GBuffer);

	// Manually set layout for blit reason
	attachments[GBufferAttachmentIndex::ENTITY_ID].currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass =  renderpass_GBuffer.pass;
	renderPassBeginInfo.framebuffer = currentFB;
	renderPassBeginInfo.renderArea.extent.width = swapchain.swapChainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapchain.swapChainExtent.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// vr.ResizeSwapchain() destroys the depth attachment. This causes the renderpass to fail on resize
	// TODO: handle all framebuffer resizes gracefully
	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	rhi::CommandList cmd{ cmdlist, "Gbuffer Pass"};
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

	cmd.BindPSO(pso_GBufferDefault);
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
	cmd.BindVertexBuffer(BIND_POINT_WEIGHTS_BUFFER_ID, 1, vr.skinningVertexBuffer.getBufferPtr());
	cmd.BindVertexBuffer(BIND_POINT_INSTANCE_BUFFER_ID, 1, vr.instanceBuffer.getBufferPtr());
	cmd.BindIndexBuffer(vr.g_GlobalMeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	cmd.DrawIndexedIndirect(vr.indirectCommandsBuffer.getBuffer(), 0, vr.objectCount);

	vkCmdEndRenderPass(cmdlist);
}

void GBufferRenderPass::Shutdown()
{
	auto& device = VulkanRenderer::get()->m_device.logicalDevice;
	
	for (auto& att : attachments)
	{
		att.destroy();
	}

	renderpass_GBuffer.destroy();
	vkDestroyPipeline(device, pso_GBufferDefault, nullptr);
}

void GBufferRenderPass::SetupRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	//attachments[GBufferAttachmentIndex::POSITION].name = "GB_Position";
	//attachments[GBufferAttachmentIndex::POSITION].forFrameBuffer(&m_device, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	attachments[GBufferAttachmentIndex::NORMAL	].name = "GB_Normal";
	attachments[GBufferAttachmentIndex::NORMAL	].forFrameBuffer(&m_device, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	// linear texture
	attachments[GBufferAttachmentIndex::ALBEDO	].name = "GB_Albedo";
	attachments[GBufferAttachmentIndex::ALBEDO	].forFrameBuffer(&m_device, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	attachments[GBufferAttachmentIndex::MATERIAL].name = "GB_Material";
	attachments[GBufferAttachmentIndex::MATERIAL].forFrameBuffer(&m_device, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	attachments[GBufferAttachmentIndex::ENTITY_ID].name = "GB_Entity";
	attachments[GBufferAttachmentIndex::ENTITY_ID].forFrameBuffer(&m_device, VK_FORMAT_R32_SINT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, width, height);
	attachments[GBufferAttachmentIndex::DEPTH	].name = "GB_DEPTH";
	attachments[GBufferAttachmentIndex::DEPTH	].forFrameBuffer(&m_device, vr.G_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, width, height);

	// Set up separate renderpass with references to the color and depth attachments
	std::array<VkAttachmentDescription, GBufferAttachmentIndex::MAX_ATTACHMENTS> attachmentDescs = {};

	// Init attachment properties
	for (uint32_t i = 0; i < GBufferAttachmentIndex::MAX_ATTACHMENTS; ++i)
	{
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (i == GBufferAttachmentIndex::DEPTH)
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// Formats
	//attachmentDescs[GBufferAttachmentIndex::POSITION].format = attachments[GBufferAttachmentIndex::POSITION].format;
	attachmentDescs[GBufferAttachmentIndex::NORMAL]  .format = attachments[GBufferAttachmentIndex::NORMAL]  .format;
	attachmentDescs[GBufferAttachmentIndex::ALBEDO]  .format = attachments[GBufferAttachmentIndex::ALBEDO]  .format;
	attachmentDescs[GBufferAttachmentIndex::MATERIAL].format = attachments[GBufferAttachmentIndex::MATERIAL].format;
	attachmentDescs[GBufferAttachmentIndex::ENTITY_ID].format = attachments[GBufferAttachmentIndex::ENTITY_ID].format;
	attachmentDescs[GBufferAttachmentIndex::DEPTH]   .format = attachments[GBufferAttachmentIndex::DEPTH]   .format;
	

	std::vector<VkAttachmentReference> colorReferences;
	//colorReferences.push_back({ GBufferAttachmentIndex::POSITION, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::NORMAL,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::ALBEDO,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::MATERIAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::ENTITY_ID, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = GBufferAttachmentIndex::DEPTH;
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

	renderpass_GBuffer.name = "deferredPass";
	renderpass_GBuffer.Init(m_device, renderPassInfo);
}

void GBufferRenderPass::SetupFramebuffer()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;
	
	//FramebufferBuilder::Begin(&vr.fbCache)
	//	//.BindImage(&attachments[GBufferAttachmentIndex::POSITION])
	//	.BindImage(&attachments[GBufferAttachmentIndex::NORMAL  ])
	//	.BindImage(&attachments[GBufferAttachmentIndex::ALBEDO  ])
	//	.BindImage(&attachments[GBufferAttachmentIndex::MATERIAL])
	//	.BindImage(&attachments[GBufferAttachmentIndex::ENTITY_ID])
	//	.BindImage(&attachments[GBufferAttachmentIndex::DEPTH   ])
	//	.Build(framebuffer_GBuffer,renderpass_GBuffer);

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
	//VK_NAME(vr.m_device.logicalDevice, "deferredFB", framebuffer_GBuffer);

	//deferredImg[GBufferAttachmentIndex::POSITION] = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), attachments[GBufferAttachmentIndex::POSITION].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredImg[GBufferAttachmentIndex::NORMAL]   = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), attachments[GBufferAttachmentIndex::NORMAL  ].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredImg[GBufferAttachmentIndex::ALBEDO]   = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), attachments[GBufferAttachmentIndex::ALBEDO  ].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredImg[GBufferAttachmentIndex::MATERIAL] = vr.CreateImguiBinding(GfxSamplerManager::GetSampler_Deferred(), attachments[GBufferAttachmentIndex::MATERIAL].view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	//deferredImg[GBufferAttachmentIndex::DEPTH]    = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_depth.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}


void GBufferRenderPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/gbuffer.vert.spv";
	const char* shaderPS = "Shaders/bin/gbuffer.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages =
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::defaultPSOLayout, vr.renderPass_default.pass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	const auto& bindingDescription = oGFX::GetGFXVertexInputBindings();
	const auto& attributeDescriptions = oGFX::GetGFXVertexInputAttributes();
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo(bindingDescription, attributeDescriptions);
	pipelineCI.pVertexInputState = &vertexInputCreateInfo;

	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

	// Separate render pass
	pipelineCI.renderPass = renderpass_GBuffer.pass;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> blendAttachmentStates =
	{
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};

	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_GBufferDefault));
	VK_NAME(m_device.logicalDevice, "GBufferDefaultPSO", pso_GBufferDefault);

	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}
