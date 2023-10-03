/************************************************************************************//*!
\file           ForwardUIPass.cpp
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

#include <array>


struct ForwardUIPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(ForwardUIPass)

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

DECLARE_RENDERPASS(ForwardUIPass);

VulkanRenderpass renderpass_ForwardUI{};

//VkPushConstantRange pushConstantRange;
VkPipeline pso_Forward_UI{};
VkPipeline pso_Forward_UI_NO_DEPTH{};


void ForwardUIPass::Init()
{
	SetupRenderpass();
	SetupFramebuffer();
}

void ForwardUIPass::CreatePSO()
{
	CreatePipeline();
}

bool ForwardUIPass::SetupDependencies()
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

void ForwardUIPass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();
	if (!vr.deferredRendering)
		return;

	auto& device = vr.m_device;
	auto& swapchain = vr.m_swapchain;
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;

	PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("ForwardUI");

	rhi::CommandList cmd{ cmdlist, "Forward UI Pass"};
	
	auto& attachments = vr.attachments.gbuffer;

	cmd.BindAttachment(0, &vr.renderTargets[vr.renderTargetInUseID].texture);
	//cmd.BindAttachment(1, &attachments[GBufferAttachmentIndex::ENTITY_ID]);
	cmd.BindDepthAttachment(&attachments[GBufferAttachmentIndex::DEPTH]);


	cmd.BindPSO(pso_Forward_UI, PSOLayoutDB::defaultPSOLayout);
	cmd.SetDefaultViewportAndScissor();
	uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
																												vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
	cmd.BindDescriptorSet(PSOLayoutDB::defaultPSOLayout, 0, 
		std::array<VkDescriptorSet, 3>{
		vr.descriptorSet_gpuscene,
			vr.descriptorSets_uniform[currFrame],
			vr.descriptorSet_bindless,
	},
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		1, & dynamicOffset
	);

	// Bind merged mesh vertex & index buffers, instancing buffers.
	std::vector<VkBuffer> vtxBuffers{
		vr.g_UIVertexBufferGPU[currFrame].getBuffer(),
	};

	VkDeviceSize offsets[2]{
		0,
		0
	};
	cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, vr.g_UIVertexBufferGPU[currFrame].getBufferPtr());
	cmd.BindIndexBuffer(vr.g_UIIndexBufferGPU[currFrame].getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	
	//cmd.BindVertexBuffer(BIND_POINT_INSTANCE_BUFFER_ID, 1, vr.g_particleDatas[currFrame].getBufferPtr());
	
	auto &uivert = vr.batches.GetUIVertices();
	const auto ScreenSpaceVtxOffset = vr.batches.GetScreenSpaceUIOffset();

	const auto instanceCnt = uivert.size() / 4;
	const auto indicesCnt =  instanceCnt* 6;

	const auto ScreenSpaceCnt = instanceCnt - (ScreenSpaceVtxOffset / 4);
	const auto ScreenSpaceIndices = ScreenSpaceCnt * 6;
	const auto WorldSpaceCnt = instanceCnt - ScreenSpaceCnt;
	const auto WorldSpaceIndices = WorldSpaceCnt * 6;
	const auto ScreenSpaceIdxOffset = WorldSpaceIndices;
	const auto instanceOffset = instanceCnt - WorldSpaceCnt;
	// do draw command here
	// batch draw from the big buffer
	cmd.DrawIndexed(static_cast<uint32_t>(WorldSpaceIndices), 1);// draw worldspace
	
	// bind depth ignore pass
	cmd.BindPSO(pso_Forward_UI_NO_DEPTH, PSOLayoutDB::defaultPSOLayout);
	cmd.DrawIndexed(static_cast<uint32_t>(ScreenSpaceIndices), 1
					,ScreenSpaceIdxOffset, 0, 0);  // draw screenspace

}

void ForwardUIPass::Shutdown()
{
	auto& device = VulkanRenderer::get()->m_device.logicalDevice;

	renderpass_ForwardUI.destroy();
	vkDestroyPipeline(device, pso_Forward_UI, nullptr);
	vkDestroyPipeline(device, pso_Forward_UI_NO_DEPTH, nullptr);
}

void ForwardUIPass::SetupRenderpass()
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

		auto& attachments = vr.attachments.gbuffer;
	// Formats
	//attachmentDescs[GBufferAttachmentIndex::POSITION].format = attachments[GBufferAttachmentIndex::POSITION].format;
	attachmentDescs[0]  .format = vr.G_NON_HDR_FORMAT;
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

	renderpass_ForwardUI.name = "ForwardUIPass";
	renderpass_ForwardUI.Init(m_device, renderPassInfo);
}

void ForwardUIPass::SetupFramebuffer()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;
	auto& m_swapchain = vr.m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	// maybe dont use this??
	auto& attachments = vr.attachments.gbuffer;

}


void ForwardUIPass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/forwardUI.vert.spv";
	const char* shaderPS = "Shaders/bin/forwardUI.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages =
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_TRUE); // we want blending 
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, vr.G_DEPTH_COMPARISON);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::defaultPSOLayout, renderpass_ForwardUI.pass);
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
		oGFX::vkutils::inits::vertexInputBindingDescription(BIND_POINT_VERTEX_BUFFER_ID,sizeof(oGFX::UIVertex),VK_VERTEX_INPUT_RATE_VERTEX),
		//oGFX::vkutils::inits::vertexInputBindingDescription(BIND_POINT_INSTANCE_BUFFER_ID,sizeof(UIData),VK_VERTEX_INPUT_RATE_INSTANCE),
	};
	const auto& attributeDescriptions = std::vector<VkVertexInputAttributeDescription>{
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,0,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(oGFX::UIVertex, pos)), //Position attribute
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,1,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(oGFX::UIVertex, tex)),    //Texture attribute
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,2,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(oGFX::UIVertex, col)),    //Texture attribute


		//oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,2,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(UIData, transform)+0*sizeof(glm::vec4)), //xform
		//oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,3,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(UIData, transform)+1*sizeof(glm::vec4)), //xform
		//oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,4,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(UIData, transform)+2*sizeof(glm::vec4)), //xform
		//oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,5,VK_FORMAT_R32G32B32A32_SFLOAT,offsetof(UIData, transform)+3*sizeof(glm::vec4)), //xform
		//oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_INSTANCE_BUFFER_ID,6,VK_FORMAT_R32G32B32A32_UINT,offsetof(UIData, instanceData)),    //texindex, entityID

	};
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo(bindingDescription, attributeDescriptions);
	pipelineCI.pVertexInputState = &vertexInputCreateInfo;

	// Separate render pass
	// pipelineCI.renderPass = renderpass_ForwardUI.pass;
	pipelineCI.renderPass = VK_NULL_HANDLE;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, 2> blendAttachmentStates =
	{
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xF, VK_FALSE),
		oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xF, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE), // albedo blend
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		//oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};
	blendAttachmentStates[0].blendEnable = VK_TRUE; 
	blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // save background albedo as well
	blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;

	colorBlendState.attachmentCount =1;
	colorBlendState.pAttachments = blendAttachmentStates.data();

	std::array<VkFormat, 2> formats{
		vr.G_NON_HDR_FORMAT,
		VK_FORMAT_R32_SINT
	};
	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = formats.data();
	renderingInfo.depthAttachmentFormat = vr.G_DEPTH_FORMAT;
	renderingInfo.stencilAttachmentFormat = vr.G_DEPTH_FORMAT;

	pipelineCI.pNext = &renderingInfo;

	if (pso_Forward_UI != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_Forward_UI, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_Forward_UI));
	VK_NAME(m_device.logicalDevice, "forwardUIPSO", pso_Forward_UI);

	if (pso_Forward_UI_NO_DEPTH != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_Forward_UI_NO_DEPTH, nullptr);
	}
	depthStencilState.depthTestEnable = VK_FALSE;
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_Forward_UI_NO_DEPTH));
	VK_NAME(m_device.logicalDevice, "forwardUIPSO_NO_DEPTH", pso_Forward_UI_NO_DEPTH);

	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}
