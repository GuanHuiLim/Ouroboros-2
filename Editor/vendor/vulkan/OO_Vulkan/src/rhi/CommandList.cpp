/************************************************************************************//*!
\file           CommandList.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief               Defines a command list RHI to keep track of state

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "CommandList.h"

#include "VulkanRenderer.h"
#include <cassert>

namespace rhi
{

void CommandList::BindPSO(const VkPipeline& pso,const VkPipelineBindPoint bindPoint)
{
	m_pipelineBindPoint = bindPoint;
	vkCmdBindPipeline(m_VkCommandBuffer, bindPoint, pso);
}

void CommandList::SetPushConstant(VkPipelineLayout layout, const VkPushConstantRange& pcr, const void* data)
{
	memset(m_push_constant, 0, 128);
	memcpy(m_push_constant, data, pcr.size);
	vkCmdPushConstants(m_VkCommandBuffer, layout, VK_SHADER_STAGE_ALL,pcr.offset,pcr.size,data);
}

CommandList::CommandList(const VkCommandBuffer& cmd, const char* name, const glm::vec4 col)
	: m_VkCommandBuffer{ cmd } 
{
	BeginNameRegion(name, col);

	for (auto& a : m_attachments)
	{
		a = VkRenderingAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	}
}
CommandList::~CommandList()
{
	EndNamedRegion();
}

void CommandList::EndNamedRegion()
{
	auto region = VulkanRenderer::get()->pfnDebugMarkerRegionEnd;
	
	if (region && m_regionNamed)
	{
		region(m_VkCommandBuffer);
	}
	m_regionNamed = false;	
}

void CommandList::BeginNameRegion(const char* name, const glm::vec4 col)
{
	auto region = VulkanRenderer::get()->pfnDebugMarkerRegionBegin;
	if (name && region)
	{
		EndNamedRegion();

		VkDebugMarkerMarkerInfoEXT marker = {};
		marker.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		memcpy(marker.color, &col[0], sizeof(float) * 4);
		marker.pMarkerName = name;
		region(m_VkCommandBuffer, &marker);
		m_regionNamed = true;
	}
}

void CommandList::BindAttachment(uint32_t bindPoint, vkutils::Texture2D* tex, bool clearOnDraw)
{
	//start tracking
	if (tex) {
		VkRenderingAttachmentInfo albedoInfo{};
		albedoInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		albedoInfo.pNext = NULL;
		albedoInfo.resolveMode = {};
		albedoInfo.resolveImageView = {};
		albedoInfo.resolveImageLayout = {};
		albedoInfo.imageView = tex->view;
		albedoInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		albedoInfo.loadOp = clearOnDraw ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		albedoInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		albedoInfo.clearValue = VkClearValue{ {} };

		m_attachments[bindPoint] = albedoInfo;
		m_highestAttachmentBound = std::max<int32_t>(bindPoint, m_highestAttachmentBound);
	}
	else 
	{
		m_attachments[bindPoint] = VkRenderingAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
	}
}

void CommandList::BindDepthAttachment(vkutils::Texture2D* tex, bool clearOnDraw)
{
	if (tex) 
	{	
		VkRenderingAttachmentInfo depthInfo{};
		depthInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		depthInfo.pNext = NULL;
		depthInfo.resolveMode = {};
		depthInfo.resolveImageView = {};
		depthInfo.resolveImageLayout = {};
		depthInfo.imageView = tex->view;
		depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthInfo.loadOp = clearOnDraw ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		depthInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthInfo.clearValue = VkClearValue{ {} };
		m_depth = depthInfo;
		m_depthBound = true;
	}
	else 
	{
		m_depthBound = false;
	}
}
void CommandList::BindVertexBuffer(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets /*= nullptr*/)
{
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(m_VkCommandBuffer, firstBinding, bindingCount, pBuffers, pOffsets ? pOffsets : offsets);
}

void CommandList::BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(m_VkCommandBuffer, buffer, offset, indexType);
}

void CommandList::BeginRendering(VkRect2D renderArea)
{
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = renderArea;
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = m_highestAttachmentBound + 1; // should be [0-8];
	renderingInfo.pColorAttachments = m_attachments.data();

	renderingInfo.pDepthAttachment = m_depthBound ? &m_depth : NULL;
	renderingInfo.pStencilAttachment = m_depthBound ? &m_depth : NULL;

	vkCmdBeginRendering(m_VkCommandBuffer, &renderingInfo);
}

void CommandList::EndRendering()
{
	vkCmdEndRendering(m_VkCommandBuffer);
}


void CommandList::DrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	//transition resource
	::DrawIndexedIndirect(m_VkCommandBuffer, buffer, offset, drawCount, stride);
}

void CommandList::BindDescriptorSet(VkPipelineLayout layout,
	uint32_t firstSet, uint32_t descriptorSetCount,
	const VkDescriptorSet* pDescriptorSets,
	VkPipelineBindPoint bindpoint,
	uint32_t dynamicOffsetCount /*= 1*/, const uint32_t* pDynamicOffsets /*= nullptr */)
{
	m_pipeLayout = layout;

	uint32_t dynamicOffset = 0;
	vkCmdBindDescriptorSets(
		m_VkCommandBuffer,
		bindpoint,
		layout,
		firstSet,
		descriptorSetCount,
		pDescriptorSets,
		dynamicOffsetCount,
		pDynamicOffsets ? pDynamicOffsets : &dynamicOffset);
}

void CommandList::DrawFullScreenQuad()
{
	vkCmdDraw(m_VkCommandBuffer, 3, 1, 0, 0);
}

void CommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
{
	vkCmdDispatch(m_VkCommandBuffer, x, y, z);
}

void CommandList::SetDefaultViewportAndScissor()
{
	::SetDefaultViewportAndScissor(m_VkCommandBuffer);
}

void CommandList::SetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	assert(viewportCount < 8);
	for (size_t i = 0; i < viewportCount; i++)
	{
		m_viewport[i] = pViewports[i];
	}
	vkCmdSetViewport(m_VkCommandBuffer, firstViewport, viewportCount, pViewports);
}

void CommandList::SetViewport(const VkViewport& viewport)
{
	const VkViewport vp{ viewport };
	this->SetViewport(0, 1, &vp);
}

void CommandList::SetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	assert(scissorCount < 8);
	for (size_t i = 0; i < scissorCount; i++)
	{
		m_scissor[i] = pScissors[i];
	}
	vkCmdSetScissor(m_VkCommandBuffer, firstScissor, scissorCount, pScissors);
}

void CommandList::SetScissor(const VkRect2D& scissor)
{
	const VkRect2D s{ scissor };
	this->SetScissor(0, 1, &s);
}

void CommandList::ClearImage(vkutils::Texture2D& texture,  VkClearValue clearval)
{
	auto oldformat = texture.currentLayout;
	vkutils::TransitionImage(m_VkCommandBuffer, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkImageSubresourceRange range{};
	if (texture.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		VkClearDepthStencilValue depth{};
		depth = clearval.depthStencil;
		range = VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
		vkCmdClearDepthStencilImage(m_VkCommandBuffer, texture.image.image, texture.currentLayout, &depth, 1, &range);
	}
	else
	{
		VkClearColorValue col{};
		col = clearval.color;
		range = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCmdClearColorImage(m_VkCommandBuffer, texture.image.image, texture.currentLayout, &col, 1, &range);
	}
	
vkutils::TransitionImage(m_VkCommandBuffer, texture, oldformat);
}

}
