#pragma once

#include <vulkan/vulkan.h>
#include "VulkanUtils.h"
#include "VulkanBuffer.h"

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
	

	friend class VulkanRenderer;
	VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
	VkDevice logicalDevice{VK_NULL_HANDLE};
	VulkanInstance* m_instancePtr{nullptr};

	VkQueue graphicsQueue{VK_NULL_HANDLE};
	oGFX::QueueFamilyIndices queueIndices{};
	VkQueue presentationQueue{VK_NULL_HANDLE};

	VkPhysicalDeviceFeatures enabledFeatures{};
	VkPhysicalDeviceProperties properties{};

	VkCommandPool commandPool{ VK_NULL_HANDLE };

	bool CheckDeviceSuitable(const oGFX::SetupInfo& si,VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(const oGFX::SetupInfo& si,VkPhysicalDevice device);	

	VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
		vkutils::Buffer* buffer, VkDeviceSize size,const void* data = nullptr);

	void CopyBuffer(vkutils::Buffer* src, vkutils::Buffer* dst, VkQueue queue,
			VkBufferCopy* copyRegion = nullptr);

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin = false);
	void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
	void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

};

