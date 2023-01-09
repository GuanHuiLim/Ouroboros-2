/************************************************************************************//*!
\file           VulkanTexture.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              A texture wrapper object for vulkan texture

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
		VulkanDevice* device{ nullptr };
		VkImage image{};
		VkFormat format{};
		VkImageLayout imageLayout{};
		VkImageLayout currentLayout{};
		VkDeviceMemory deviceMemory{};
		VkImageView view{};
		uint32_t width{}, height{};
		uint32_t mipLevels{};
		uint32_t layerCount{};
		VkDescriptorImageInfo descriptor{};
		VkSampler sampler{};
		VkImageUsageFlags usage{};
		VkImageAspectFlags aspectMask{};
		VkMemoryPropertyFlags MemProps{};
		bool targetSwapchain = true;
		float renderScale = 1.0f;
		
		void updateDescriptor();
		void destroy(bool delayed = false);
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D() : Texture() {}

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

		void forFrameBuffer(VulkanDevice* device,
			VkFormat format,
			VkImageUsageFlags imageUsageFlags,
			uint32_t texWidth, uint32_t texHeight,
			bool forFullscr = true,
			float renderscale = 1.0f,
			uint32_t mipLevels = 1,
			VkMemoryPropertyFlags properties= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			VkFilter filter = VK_FILTER_LINEAR
		);

		void Resize(uint32_t texWidth, uint32_t texHeight);

		void Update(void* buffer,
			VkDeviceSize bufferSize,
			VkFormat format,
			uint32_t texWidth,
			uint32_t texHeight,
			std::vector<VkBufferImageCopy> mips,
			VulkanDevice* device,
			VkQueue copyQueue,
			VkFilter filter = VK_FILTER_LINEAR,
			VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT);
	};


	void TransitionImage(VkCommandBuffer cmd,Texture2D& texture, VkImageLayout targetLayout);

}
