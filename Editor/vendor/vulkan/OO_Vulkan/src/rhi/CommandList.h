/************************************************************************************//*!
\file           CommandList.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a command list RHI to keep track of state

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "MathCommon.h"
#include <array>
#include <vulkan/vulkan.h>
#include "VulkanTexture.h"
#include "DescriptorBuilder.h"

namespace rhi
{
	class CommandList;

	struct ImageStateTracking {
		VkImageLayout referenceLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageLayout currentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
		VkImageLayout expectedLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
	};

	enum ResourceUsage {
		UNKNOWN,
		SRV,
		UAV
	};

	struct BufferStateTracking {
		ResourceUsage referenceAccess{ SRV };
		ResourceUsage currentAccess{ SRV };
		ResourceUsage expectedAccess{ UNKNOWN };
		VkPipelineBindPoint previousStage{ VK_PIPELINE_BIND_POINT_GRAPHICS };
	};

	class DescriptorSetInfo {
	public:
		DescriptorBuilder builder;
		VkShaderStageFlags shaderStage;
		VkDescriptorSetLayout layout;
		VkDescriptorSet descriptor;

		bool built = false;
		bool expected = false;
		bool bound = false;

		bool hasDynamicOffset = false;
		uint32_t dynamicOffset;
		
		CommandList* m_cmdList;

		DescriptorSetInfo& BindImage(uint32_t binding, vkutils::Texture* texture, VkDescriptorType type);
		DescriptorSetInfo& BindImage(uint32_t binding, vkutils::Texture* texture, VkDescriptorType type, VkImageView viewOverride);
		DescriptorSetInfo& BindSampler(uint32_t binding, VkSampler sampler, VkShaderStageFlags stageFlagsInclude = 0);
		DescriptorSetInfo& BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type,ResourceUsage access = ResourceUsage::SRV, VkShaderStageFlags stageFlagsInclude = 0);
	};

// Another better alternative is to use Vulkan HPP.
class CommandList
{
public:
	

	CommandList(const VkCommandBuffer& cmd, const char* name = nullptr, const glm::vec4 col = glm::vec4{ 1.0f,1.0f,1.0f,0.0f });
	~CommandList();

	void BeginNameRegion(const char* name, const glm::vec4 col = glm::vec4{ 1.0f,1.0f,1.0f,0.0f });
	void EndNamedRegion();

	void BeginTrackingImage(vkutils::Texture* tex);
	ImageStateTracking* getTrackedImage(vkutils::Texture* tex);
	ImageStateTracking* ensureTrackedImage(vkutils::Texture* tex);

	void BeginTrackingBuffer(VkBuffer buffer);
	BufferStateTracking* getTrackedBuffer(VkBuffer buffer);
	BufferStateTracking* ensureTrackedBuffer(VkBuffer buffer);

	void VerifyResourceStates();
	void RestoreResourceStates();

	void VerifyImageResourceStates();
	void RestoreImageResourceStates();
	void VerifyBufferResourceStates();
	void RestoreBufferResourceStates();

	void CopyImage(vkutils::Texture* src, vkutils::Texture* dst);

	//----------------------------------------------------------------------------------------------------
	// Binding Commands
	//----------------------------------------------------------------------------------------------------
	void BindAttachment(uint32_t bindPoint, vkutils::Texture* tex, bool clearOnDraw = false);
	void BindDepthAttachment(vkutils::Texture* tex, bool clearOnDraw = false);

	void BindVertexBuffer(
		uint32_t firstBinding,
		uint32_t bindingCount,
		const VkBuffer* pBuffers,
		const VkDeviceSize* pOffsets = nullptr);

	void BindIndexBuffer(
		VkBuffer buffer,
		VkDeviceSize offset,
		VkIndexType indexType
	);

	void BeginRendering(VkRect2D renderArea);
	void EndRendering();

	void BindPSO(const VkPipeline& pso, VkPipelineLayout pipelay, const VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

	void SetPushConstant(VkPipelineLayout layout,
		const VkPushConstantRange& pcr,
		const void* data
	);

	DescriptorSetInfo& DescriptorSetBegin(uint32_t set);

	void BindDescriptorSet(uint32_t set, VkDescriptorSet descriptor);

	void BindDescriptorSet(
		VkPipelineLayout layout,
		uint32_t firstSet,
		uint32_t descriptorSetCount,
		const VkDescriptorSet* pDescriptorSets,
		VkPipelineBindPoint bindpoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		uint32_t dynamicOffsetCount = 0,
		const uint32_t* pDynamicOffsets = nullptr
	);

	template<typename T_ARRAY>
	void BindDescriptorSet(
		VkPipelineLayout layout,
		uint32_t firstSet,
		const T_ARRAY& array,
		VkPipelineBindPoint bindpoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		uint32_t dynamicOffsetCount = 1,
		const uint32_t* pDynamicOffsets = nullptr
	)
	{
		this->BindDescriptorSet(
			layout,
			firstSet,
			(uint32_t)array.size(),
			array.data(),
			bindpoint,
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
		uint32_t firstInstance = 0);

	void DrawIndexed(
		uint32_t indexCount,
		uint32_t instanceCount,
		uint32_t firstIndex = 0,
		int32_t vertexOffset = 0,
		uint32_t firstInstance = 0);
	

	void DrawIndexedIndirect(
			VkBuffer buffer,
			VkDeviceSize offset,
			uint32_t drawCount,
			uint32_t stride = sizeof(VkDrawIndexedIndirectCommand));
	
	// Helper function to draw a Full Screen Quad, without binding vertex and index buffers.
	void DrawFullScreenQuad();

	void Dispatch(uint32_t x, uint32_t y = 1 , uint32_t z = 1);

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

	VkCommandBuffer getCommandBuffer();
private:
	void CommitDescriptors();

	VkCommandBuffer m_VkCommandBuffer{};

	VkPipelineLayout m_pipeLayout{};
	VkPipelineBindPoint m_pipelineBindPoint{ VK_PIPELINE_BIND_POINT_MAX_ENUM };
	VkShaderStageFlags m_targetStage{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };

	std::array<VkRect2D, 8> m_scissor;
	std::array<VkViewport, 8> m_viewport;
	std::array<VkRenderingAttachmentInfo, 8> m_attachments{};
	std::array<bool, 8> m_shouldClearAttachment{};
	int32_t m_highestAttachmentBound{-1};
	bool m_depthBound = false;
	bool m_shouldClearDepth = false;
	VkRenderingAttachmentInfo m_depth;
	float m_push_constant[128 / sizeof(float)]{0.0f};
	bool m_regionNamed = false;

	VkRect2D m_renderArea{};

	std::unordered_map<vkutils::Texture*, ImageStateTracking> m_trackedTextures;
	std::unordered_map<VkBuffer, BufferStateTracking> m_trackedBuffers;

	std::array<DescriptorSetInfo, 4> descriptorSets; // only support 4 sets
	 
	// TODO: Handle VK_PIPELINE_BIND_POINT_GRAPHICS etc nicely next time.
	// TODO: Maybe we can cache the stuff that is bound, for easier debugging, else taking GPU captures is really unproductive.
};

}
