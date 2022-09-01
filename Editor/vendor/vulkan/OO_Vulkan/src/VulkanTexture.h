#pragma once

#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "vulkan/vulkan.h"

//#include "VulkanBuffer.h"
#include "VulkanDevice.h"
//#include "VulkanTools.h"

namespace vkutils
{
	class Texture
	{
	public:
		std::string name{}; // maybe remove when not debug?
		VulkanDevice* device;
		VkImage image;
		VkFormat format;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;
		
		void updateDescriptor();
		void destroy();
	};

	class Texture2D : public Texture
	{
	public:
		void loadFromFile(
			std::string filename,
			VkFormat format,
			VulkanDevice* device,
			VkQueue copyQueue,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			bool forceLinear = false);

		void fromBuffer(
			void* buffer,
			VkDeviceSize bufferSize,
			VkFormat format,
			uint32_t texWidth,
			uint32_t texHeight, 
			std::vector<VkBufferImageCopy> mips,
			VulkanDevice* device,
			VkQueue copyQueue,
			VkFilter filter = VK_FILTER_LINEAR,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		void forFrameBuffer(VkFormat format,
			VkImageUsageFlags imageUsageFlags,
			uint32_t texWidth, uint32_t texHeight,
			VulkanDevice* device,
			uint32_t mipLevels = 1,
			VkMemoryPropertyFlags properties= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			VkFilter filter = VK_FILTER_LINEAR
		);
	};

}
