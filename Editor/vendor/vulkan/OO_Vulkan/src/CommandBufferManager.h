/************************************************************************************//*!
\file           CommandBufferManager.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Aug 24, 2023
\brief              Defines command list allocator

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "vulkan/vulkan.h"
#include <vector>

namespace oGFX
{

	enum eRECSTATUS : int8_t {
		INVALID = -1,
		RECORDING = 0,
		SUBMITTED = 1,
};

class CommandBufferManager
{
public:
	VkResult InitPool(VkDevice device, uint32_t queueIndex);
	VkCommandBuffer GetNextCommandBuffer(bool begin = false);
	void ResetPool();
	void DestroyPool();
	void SubmitCommandBuffer(VkQueue queue, VkCommandBuffer cmd);
	void SubmitAll(VkQueue queue, VkSubmitInfo submitInfo = {}, VkFence signalFence = VK_NULL_HANDLE);

	VkCommandPool m_commandpool{};
private:
	void AllocateCommandBuffer();
	size_t counter{ };

	VkDevice m_device{};
	uint32_t nextIdx{};
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<int8_t>submitted{};
};


}
