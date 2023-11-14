/************************************************************************************//*!
\file           VulkanDevice.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a vulkan device wrapper

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <vulkan/vulkan.h>
#include "VulkanUtils.h"
#include "VulkanBuffer.h"
#include "CommandBufferManager.h"

#include "VmaUsage.h"
#include "gpuCommon.h"

struct Window;
struct VulkanInstance;
struct VulkanDevice
{
	VulkanDevice() = default;
	~VulkanDevice();
	VulkanDevice(const VulkanDevice&) = delete;

	void InitPhysicalDevice(const oGFX::SetupInfo& si,VulkanInstance& instance);
	void InitLogicalDevice(const oGFX::SetupInfo& si,VulkanInstance& instance);
	void InitAllocator(const oGFX::SetupInfo& si,VulkanInstance& instance);
	

	friend class VulkanRenderer;
	VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
	VkDevice logicalDevice{VK_NULL_HANDLE};
	VulkanInstance* m_instancePtr{nullptr};

	VmaAllocator m_allocator{};

	VkQueue graphicsQueue{VK_NULL_HANDLE};
	oGFX::QueueFamilyIndices queueIndices{};

	VkPhysicalDeviceFeatures2 enabledFeatures{};
	VkPhysicalDeviceProperties properties{};

	std::vector<oGFX::CommandBufferManager> commandPoolManagers;

	bool CheckDeviceSuitable(const oGFX::SetupInfo& si,VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(const oGFX::SetupInfo& si,VkPhysicalDevice device);	

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
	//VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
	void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
	//void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

};

