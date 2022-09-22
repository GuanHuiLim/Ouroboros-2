#pragma once

#include <vulkan/vulkan.h>

namespace rhi
{

// Another better alternative is to use Vulkan HPP.
class CommandList
{
public:

	CommandList(const VkCommandBuffer& cmd)
		: m_VkCommandBuffer{ cmd } 
	{
	}

	//----------------------------------------------------------------------------------------------------
	// Binding Commands
	//----------------------------------------------------------------------------------------------------

	void BindVertexBuffer(
		uint32_t firstBinding,
		uint32_t bindingCount,
		const VkBuffer* pBuffers,
		const VkDeviceSize* pOffsets = nullptr);

	void BindIndexBuffer(
		VkBuffer buffer,
		VkDeviceSize offset,
		VkIndexType indexType);

	void BindPSO(const VkPipeline& pso);

	void BindDescriptorSet(
		VkPipelineLayout layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const VkDescriptorSet* pDescriptorSets,
		uint32_t dynamicOffsetCount = 0,
		const uint32_t* pDynamicOffsets = nullptr
	);

	template<typename T_ARRAY>
	void BindDescriptorSet(
		VkPipelineLayout layout,
		uint32_t firstSet,
		const T_ARRAY& array,
		uint32_t dynamicOffsetCount = 1,
		const uint32_t* pDynamicOffsets = nullptr
	)
	{
		this->BindDescriptorSet(
			layout,
			firstSet,
			(uint32_t)array.size(),
			array.data(),
			dynamicOffsetCount,
			pDynamicOffsets);
	}

	//----------------------------------------------------------------------------------------------------
	// Drawing Commands
	//----------------------------------------------------------------------------------------------------

	void Draw(
		uint32_t vertexCount,
		uint32_t instanceCount,
		uint32_t firstVertex = 0,
		uint32_t firstInstance = 0)
	{
		vkCmdDraw(m_VkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void DrawIndexed(
		uint32_t indexCount,
		uint32_t instanceCount,
		uint32_t firstIndex = 0,
		int32_t vertexOffset = 0 ,
		uint32_t firstInstance = 0)
	{
		vkCmdDrawIndexed(m_VkCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void DrawIndexedIndirect(
			VkBuffer buffer,
			VkDeviceSize offset,
			uint32_t drawCount,
			uint32_t stride = sizeof(VkDrawIndexedIndirectCommand));
	
	// Helper function to draw a Full Screen Quad, without binding vertex and index buffers.
	void DrawFullScreenQuad();

	//----------------------------------------------------------------------------------------------------
	// Pipeline State Commands
	//----------------------------------------------------------------------------------------------------

	void SetDefaultViewportAndScissor();

	void SetViewport(uint32_t firstViewport,
		uint32_t viewportCount,
		const VkViewport* pViewports);

	void SetViewport(const VkViewport& viewport);

	void SetScissor(uint32_t firstScissor,
		uint32_t scissorCount,
		const VkRect2D* pScissors);

	void SetScissor(const VkRect2D& scissor);

	// TODO: Function not here? Add it on demand...

private:
	VkCommandBuffer m_VkCommandBuffer;
	// TODO: Handle VK_PIPELINE_BIND_POINT_GRAPHICS etc nicely next time.
	// TODO: Maybe we can cache the stuff that is bound, for easier debugging, else taking GPU captures is really unproductive.
};

}
