#include "CommandList.h"

#include "VulkanRenderer.h"

namespace rhi
{

void CommandList::BindPSO(const VkPipeline& pso)
{
	vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pso);
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

void CommandList::DrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	::DrawIndexedIndirect(m_VkCommandBuffer, buffer, offset, drawCount, stride);
}

void CommandList::BindDescriptorSet(VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount /*= 1*/, const uint32_t* pDynamicOffsets /*= nullptr */)
{
	uint32_t dynamicOffset = 0;
	vkCmdBindDescriptorSets(
		m_VkCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
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

void CommandList::SetDefaultViewportAndScissor()
{
	::SetDefaultViewportAndScissor(m_VkCommandBuffer);
}

void CommandList::SetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
	vkCmdSetViewport(m_VkCommandBuffer, firstViewport, viewportCount, pViewports);
}

void CommandList::SetViewport(const VkViewport& viewport)
{
	const VkViewport vp{ viewport };
	this->SetViewport(0, 1, &vp);
}

void CommandList::SetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
	vkCmdSetScissor(m_VkCommandBuffer, firstScissor, scissorCount, pScissors);
}

void CommandList::SetScissor(const VkRect2D& scissor)
{
	const VkRect2D s{ scissor };
	this->SetScissor(0, 1, &s);
}

}