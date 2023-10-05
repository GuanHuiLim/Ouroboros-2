/************************************************************************************//*!
\file           DebugRenderpass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a debug drawing pass

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "GfxRenderpass.h"

#include <array>

#include "imgui/imgui.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "FramebufferBuilder.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"


class ImguiRenderpass : public GfxRenderpass
{
public:

	void Init() override;
	void Draw(const VkCommandBuffer cmdlist) override;
	void Shutdown() override;

	bool SetupDependencies() override;

private:
	void CreatePipelineLayout();
	void CreatePipeline();
	void InitDebugBuffers();
};

DECLARE_RENDERPASS(ImguiRenderpass);

VkPipeline imguiPSO{};

void ImguiRenderpass::Init()
{
	CreatePipelineLayout();
	CreatePipeline();
	InitDebugBuffers();
}

bool ImguiRenderpass::SetupDependencies()
{
	// TODO: If debug drawing is disabled, return false.
	
	// READ: Scene Depth
	// WRITE: Color Output
	// etc

	return true;
}
#pragma optimize("", off)
void ImguiRenderpass::Draw(const VkCommandBuffer cmdlist)
{
	auto& vr = *VulkanRenderer::get();
	lastCmd = cmdlist;
	if (vr.m_imguiInitialized == false) return;

	std::scoped_lock lock{ vr.m_imguiShutdownGuard };
	auto currFrame = vr.getFrame();
	auto* windowPtr = vr.windowPtr;

	ImDrawData* draw_data = &vr.m_imguiDrawData;
	oGFX::AllocatedBuffer& vertexBuffer = vr.imguiVertexBuffer[currFrame];
	oGFX::AllocatedBuffer& indexBuffer = vr.imguiIndexBuffer[currFrame];
	oGFX::AllocatedBuffer& constantBuffer = vr.imguiConstantBuffer[currFrame];

	std::array<VkClearValue, 2> clearValues{};
	//clearValues[0].color = { 0.6f,0.65f,0.4f,1.0f };
	clearValues[0].color = { 0.1f,0.1f,0.1f,0.0f };
	clearValues[1].depthStencil.depth = {1.0f };

	auto& depthAtt = vr.attachments.gbuffer[GBufferAttachmentIndex::DEPTH];
	float width = vr.m_swapchain.swapChainImages[vr.swapchainIdx].width;
	float height = vr.m_swapchain.swapChainImages[vr.swapchainIdx].height;

	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("ImguiPass");
	rhi::CommandList cmd{ cmdlist, "Imgui Pass"};

	auto& target = vr.m_swapchain.swapChainImages[vr.swapchainIdx];

	// upload mtx
	const float L = draw_data->DisplayPos.x;
	const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
	const float T = draw_data->DisplayPos.y;
	const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
	glm::mat4 orthoProj = glm::ortho(L, R, B, T);
	//orthoProj[1][1] *= -1.0f;
	std::memcpy(constantBuffer.allocInfo.pMappedData, &orthoProj[0][0], sizeof(glm::mat4));
	vmaFlushAllocation(vr.m_device.m_allocator, constantBuffer.alloc, 0, VK_WHOLE_SIZE);
	
	cmd.BindPSO(imguiPSO, PSOLayoutDB::imguiPSOLayout);
	cmd.BindAttachment(0, &target);
	
	cmd.SetDefaultViewportAndScissor();
	
	cmd.DescriptorSetBegin(1)
		.BindBuffer(0, vr.imguiConstantBuffer[currFrame].getBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		//.BindSampler(1, GfxSamplerManager::GetDefaultSampler());

	size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
	size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
	oGFX::CreateOrResizeBuffer(vr.m_device.m_allocator, vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, vertexBuffer);
	oGFX::CreateOrResizeBuffer(vr.m_device.m_allocator, index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, indexBuffer);


	size_t idxOffset{};
	size_t vtxOffset{};

	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

		size_t vtxSize = cmd_list->VtxBuffer.Size;
		size_t idxSize = cmd_list->IdxBuffer.Size;
		// copy data over
		// beware offsets
		memcpy(reinterpret_cast<ImDrawVert*>(vertexBuffer.allocInfo.pMappedData) + vtxOffset, cmd_list->VtxBuffer.Data, vtxSize * sizeof(ImDrawVert));
		memcpy(reinterpret_cast<ImDrawIdx*>(indexBuffer.allocInfo.pMappedData) + idxOffset, cmd_list->IdxBuffer.Data, idxSize * sizeof(ImDrawIdx));

		vtxOffset += vtxSize;
		idxOffset += idxSize;
	}
	vmaFlushAllocation(vr.m_device.m_allocator, vertexBuffer.alloc, 0,VK_WHOLE_SIZE);
	vmaFlushAllocation(vr.m_device.m_allocator, indexBuffer.alloc, 0, VK_WHOLE_SIZE);

	cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, &vertexBuffer.buffer);
	cmd.BindIndexBuffer(indexBuffer.buffer, 0, sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	idxOffset = 0;
	vtxOffset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

		size_t vtxSize = cmd_list->VtxBuffer.Size;
		size_t idxSize = cmd_list->IdxBuffer.Size;
		// copy data over
		// beware offsets	

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {

			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

			// Project scissor/clipping rectangles into framebuffer space
			ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
			ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

			// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
			if (clip_min.x < 0.0f) { clip_min.x = 0.0f; }
			if (clip_min.y < 0.0f) { clip_min.y = 0.0f; }
			if (clip_max.x > width) { clip_max.x = (float)width; }
			if (clip_max.y > height) { clip_max.y = (float)height; }
			if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
				continue;

			// Apply scissor/clipping rectangle
			VkRect2D scissor{};
			scissor.offset.x = (int32_t)(clip_min.x);
			scissor.offset.y = (int32_t)(clip_min.y);
			scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
			scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);
			
			cmd.SetScissor(scissor);

			{
				std::scoped_lock l{ vr.g_mute_imguiTextureMap };
				vkutils::Texture* tex = vr.g_imguiToTexture[pcmd->TextureId];
				if (tex == nullptr) { tex = &vr.g_Textures[vr.whiteTextureID]; };

				cmd.DescriptorSetBegin(0)
					.BindImage(0, tex, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
					.BindSampler(1, GfxSamplerManager::GetDefaultSampler());
			}		

			cmd.DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + idxOffset, pcmd->VtxOffset + vtxOffset);
		}

		vtxOffset += vtxSize;
		idxOffset += idxSize;
	}	
	
}

void ImguiRenderpass::Shutdown()
{
	VulkanRenderer* vr = VulkanRenderer::get();
	auto& device = vr->m_device.logicalDevice;

	vkDestroyPipeline(device, imguiPSO, nullptr);

	vkDestroyPipelineLayout(device, PSOLayoutDB::imguiPSOLayout, nullptr);

}

void ImguiRenderpass::CreatePipelineLayout()
{

	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	VkSampler sampler = GfxSamplerManager::GetDefaultSampler();
	DescriptorBuilder::Begin()
		.DeclareDescriptor(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.DeclareDescriptor(1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BuildLayout(SetLayoutDB::imguiTexture);
	
	DescriptorBuilder::Begin()
		.DeclareDescriptor(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BuildLayout(SetLayoutDB::imguiCB);

	std::vector<VkDescriptorSetLayout> setLayouts
	{
		SetLayoutDB::imguiTexture, // (set = 0)
		SetLayoutDB::imguiCB, // (set = 1)
	};

	VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(
		setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));

	VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
	plci.pushConstantRangeCount = 1;
	plci.pPushConstantRanges = &pushConstantRange;


	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &PSOLayoutDB::imguiPSOLayout));
	VK_NAME(m_device.logicalDevice, "imguiPSOLayout", PSOLayoutDB::imguiPSOLayout);

}

void ImguiRenderpass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/imgui.vert.spv";
	const char* shaderPS = "Shaders/bin/imgui.frag.spv";

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages =
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	const std::vector<VkVertexInputBindingDescription> bindingDescription =
	{
		oGFX::vkutils::inits::vertexInputBindingDescription(BIND_POINT_VERTEX_BUFFER_ID,sizeof(ImDrawVert),VK_VERTEX_INPUT_RATE_VERTEX),
	};

	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
	{
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,0,VK_FORMAT_R32G32_SFLOAT, IM_OFFSETOF(ImDrawVert, pos)),
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,1,VK_FORMAT_R32G32_SFLOAT, IM_OFFSETOF(ImDrawVert, uv)),
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,2,VK_FORMAT_R8G8B8A8_UNORM, IM_OFFSETOF(ImDrawVert, col)),
	};

	using oGFX::vkutils::inits::Creator;
	auto vertexInputCreateInfo   = Creator<VkPipelineVertexInputStateCreateInfo>(bindingDescription, attributeDescriptions);
	auto inputAssembly           = Creator<VkPipelineInputAssemblyStateCreateInfo>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	auto viewportStateCreateInfo = Creator<VkPipelineViewportStateCreateInfo>();
	auto multisamplingCreateInfo = Creator<VkPipelineMultisampleStateCreateInfo>();
	auto rasterizerCreateInfo    = Creator<VkPipelineRasterizationStateCreateInfo>(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	const std::vector dynamicState{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	auto dynamicStateCreateInfo  = Creator<VkPipelineDynamicStateCreateInfo>(dynamicState);
	auto depthStencilCreateInfo  = Creator<VkPipelineDepthStencilStateCreateInfo>(VK_FALSE, VK_FALSE, vr.G_DEPTH_COMPARISON);

	VkPipelineColorBlendAttachmentState colourState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0x0000000F,VK_TRUE);
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;
	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1,&colourState);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::imguiPSOLayout,vr.renderPass_HDR.pass);
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	
	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &vr.m_swapchain.swapChainImageFormat;
	renderingInfo.depthAttachmentFormat = vr.G_DEPTH_FORMAT;
	renderingInfo.stencilAttachmentFormat = vr.G_DEPTH_FORMAT;

	pipelineCreateInfo.pNext = &renderingInfo;
	//pipelineCreateInfo.renderPass = debugRenderpass.pass;
	pipelineCreateInfo.renderPass = VK_NULL_HANDLE;

	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilCreateInfo.flags = {};
	depthStencilCreateInfo.pNext = NULL;
	depthStencilCreateInfo.front = { };
	depthStencilCreateInfo.back = { };

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &imguiPSO));
	VK_NAME(m_device.logicalDevice, "imguiPipe", imguiPSO);

	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}

void ImguiRenderpass::InitDebugBuffers()
{
}
