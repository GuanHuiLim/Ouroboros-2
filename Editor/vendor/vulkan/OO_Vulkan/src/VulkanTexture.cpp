/************************************************************************************//*!
\file           VulkanTexture.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines the texture wrapper object

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "VulkanTexture.h"
#include "DelayedDeleter.h"

#pragma warning( push )
#pragma warning( disable : 26451 ) // arithmetic overflow
#include "loader/stb_image.h"
#pragma warning( pop )

#include <cassert>

namespace vkutils
{
	void Texture::updateDescriptor()
	{
		descriptor.imageView = view;
		descriptor.imageLayout = imageLayout;
	}

	void Texture::destroy(bool delayed)
	{
		auto viewCpy = view;
		auto imageCpy = image.image;
		auto allocationCpy = image.allocation;
		auto deviceCpy = device->logicalDevice;
		auto allocCpy = device->m_allocator;

		// deletion func
		auto delFunctor = [=](){
			vkDestroyImageView(deviceCpy, viewCpy, nullptr);

			vmaDestroyImage(allocCpy, imageCpy, allocationCpy);
		};

		if (delayed)
		{
			auto* dd = DelayedDeleter::get();
			dd->DeleteAfterFrames(delFunctor);
		}
		else
		{
			delFunctor();
		}
		
		view = VK_NULL_HANDLE;		
		image.image = VK_NULL_HANDLE;
		image.allocation = VK_NULL_HANDLE;
	}

	/**
	* Load a 2D texture including all mip levels
	*
	* @param filename File to load (supports .ktx)
	* @param format Vulkan format of the image.image data stored in the file
	* @param device Vulkan device to create the texture on
	* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
	* @param (Optional) imageUsageFlags Usage flags for the texture's image.image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
	* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	* @param (Optional) forceLinear Force linear tiling (not advised, defaults to false)
	*
	*/
	void Texture2D::loadFromFile(std::string filename, VkFormat _format, VulkanDevice *device, VkQueue copyQueue, VkImageLayout imageLayout, VkImageUsageFlags imageUsageFlags,  bool forceLinear)
	{
		stbi_uc* ktxTextureData;
		int _width, _height, _channels;
		ktxTextureData = stbi_load(filename.c_str(),&_width,&_height, &_channels, STBI_rgb_alpha);

		this->device = device;
		width = _width;
		height = _height;
		format = _format;
		usage = imageUsageFlags;
		int ktxTextureSize = (width * height * STBI_rgb_alpha);

		//TODO mips
		mipLevels = 0;

		// Get device properties for the requested texture _format
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(device->physicalDevice, _format, &formatProperties);

		// Only use linear tiling if requested (and supported by the device)
		// Support for linear tiling is mostly limited, so prefer to use
		// optimal tiling instead
		// On most implementations linear tiling will only support a very
		// limited amount of _formats and features (mip maps, cubemaps, arrays, etc.)
		VkBool32 useStaging = !forceLinear;

		VkMemoryAllocateInfo memAllocInfo =oGFX::vkutils::inits::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		// Use a separate command buffer for texture loading
		VkCommandBuffer copyCmd = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->commandPoolManagers[0].m_commandpool, true);

		if (useStaging)
		{

			oGFX::AllocatedBuffer stagingBuffer{};
			oGFX::CreateBuffer(device->m_allocator, ktxTextureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

			void* mappedData = nullptr;
			auto result = vmaMapMemory(device->m_allocator, stagingBuffer.alloc, &mappedData);
			if (result != VK_SUCCESS)
			{
				assert(false);
			}
			memcpy(mappedData, ktxTextureData, (size_t)ktxTextureSize);
			vmaUnmapMemory(device->m_allocator, stagingBuffer.alloc);

			// Create optimal tiled target image.image
			VkImageCreateInfo imageCreateInfo = oGFX::vkutils::inits::imageCreateInfo();
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = _format;
			imageCreateInfo.mipLevels = mipLevels;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.extent = { width, height, 1 };
			imageCreateInfo.usage = imageUsageFlags;
			// Ensure that the TRANSFER_DST bit is set for staging
			//if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
			{
				imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			}


			VmaAllocationCreateInfo allocCI{};
			allocCI.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
			allocCI.usage = VMA_MEMORY_USAGE_AUTO;
			allocCI.priority = 1.0f;

			result = vmaCreateImage(device->m_allocator, &imageCreateInfo, &allocCI, &image.image, &image.allocation, &image.allocationInfo);
			if (result != VK_SUCCESS)
			{
				std::cerr << "Failed to create a image!" << std::endl;
				__debugbreak();
			}
			VK_NAME(device->logicalDevice, "loadFromFile::image", image.image);

			aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = aspectMask;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = mipLevels;
			subresourceRange.layerCount = 1;

			// Image barrier for optimal image.image (target)
			// Optimal image.image will be used as destination for the copy
			oGFX::vkutils::tools::setImageLayout(
				copyCmd,
				image.image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange);

			//// Copy mip levels from staging buffer
			//vkCmdCopyBufferToImage(
			//	copyCmd,
			//	stagingBuffer,
			//	image.image,
			//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			//	static_cast<uint32_t>(bufferCopyRegions.size()),
			//	bufferCopyRegions.data()
			//);

			// Change texture image.image layout to shader read after all mip levels have been copied
			this->imageLayout = imageLayout;
			oGFX::vkutils::tools::setImageLayout(
				copyCmd,
				image.image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				imageLayout,
				subresourceRange);

			// Clean up staging resources
			vmaDestroyBuffer(device->m_allocator, stagingBuffer.buffer, stagingBuffer.alloc);
		}
		else
		{
			// Prefer using optimal tiling, as linear tiling 
			// may support only a small set of features 
			// depending on implementation (e.g. no mip maps, only one layer, etc.)

			// Check if this support is supported for linear tiling
			assert(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

			VkImageCreateInfo imageCreateInfo = oGFX::vkutils::inits::imageCreateInfo();
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = _format;
			imageCreateInfo.extent = { width, height, 1 };
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageCreateInfo.usage = imageUsageFlags;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			// Load mip map level 0 to linear tiling image.image

			VmaAllocationCreateInfo allocCI{};
			allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; // this is absolute dogwater dont use this
			__debugbreak(); // it only exists here for reference
			allocCI.usage = VMA_MEMORY_USAGE_AUTO;
			allocCI.priority = 1.0f;
			VkResult result = vmaCreateImage(device->m_allocator, &imageCreateInfo, &allocCI, &image.image, &image.allocation, &image.allocationInfo);
			if (result != VK_SUCCESS)
			{
				std::cerr << "Failed to create a image!" << std::endl;
				__debugbreak();
			}
				

			// Get sub resource layout
			// Mip map count, array layer, etc.
			VkImageSubresource subRes = {};
			subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subRes.mipLevel = 0;

			VkSubresourceLayout subResLayout;
			void *data;

			// Get sub resources layout 
			// Includes row pitch, size offsets, etc.
			vkGetImageSubresourceLayout(device->logicalDevice, image.image, &subRes, &subResLayout);

			// Map image.image memory
			vmaMapMemory(device->m_allocator, image.allocation, &data);

			// Copy image.image data into memory
			memcpy(data, ktxTextureData, memReqs.size);

			vmaUnmapMemory(device->m_allocator, image.allocation);

			// Linear tiled images don't need to be staged
			// and can be directly used as textures
			this->imageLayout = imageLayout;

			// Setup image.image memory barrier
			oGFX::vkutils::tools::setImageLayout(copyCmd, image.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);
			
		}
		device->FlushCommandBuffer(copyCmd, copyQueue,device->commandPoolManagers[0].m_commandpool);

		stbi_image_free(ktxTextureData);

		filter = VK_FILTER_LINEAR;

		

		// Create image.image view
		// Textures are not directly accessed by the shaders and
		// are abstracted by image.image views containing additional
		// in_formation and sub resource ranges
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = _format;
		viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		// Linear tiling usually won't support mip maps
		// Only set mip map count if optimal tiling is used
		viewCreateInfo.subresourceRange.levelCount = (useStaging) ? mipLevels : 1;
		viewCreateInfo.image = image.image;
		vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &view);
		VK_NAME(device->logicalDevice, "loadFromFile::view", view);

		// Update descriptor image.image info member that can be used for setting up descriptor sets
		updateDescriptor();
	}

	/**
	* Creates a 2D texture from a buffer
	*
	* @param buffer Buffer containing texture data to upload
	* @param bufferSize Size of the buffer in machine units
	* @param width Width of the texture to create
	* @param height Height of the texture to create
	* @param format Vulkan format of the image.image data stored in the file
	* @param device Vulkan device to create the texture on
	* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
	* @param (Optional) filter Texture filtering for the sampler (defaults to VK_FILTER_LINEAR)
	* @param (Optional) imageUsageFlags Usage flags for the texture's image.image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
	* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	*/
	void Texture2D::fromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat _format,
		uint32_t texWidth, uint32_t texHeight, std::vector<VkBufferImageCopy> mipInfo,VulkanDevice* device, VkQueue copyQueue, VkImageLayout _imageLayout, VkFilter _filter, VkImageUsageFlags imageUsageFlags)
	{
		assert(buffer);

		this->device = device;
		width = texWidth;
		height = texHeight;
		format = _format;
		usage = imageUsageFlags;
		mipLevels =static_cast<uint32_t>(mipInfo.size());
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		this->imageLayout = _imageLayout;

		VkCommandBuffer copyCmd = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, device->commandPoolManagers[0].m_commandpool, true);

		oGFX::AllocatedBuffer stagingBuffer{};
		oGFX::CreateBuffer(device->m_allocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

		void* mappedData = nullptr;
		auto result = vmaMapMemory(device->m_allocator, stagingBuffer.alloc, &mappedData);
		if (result != VK_SUCCESS)
		{
			assert(false);
		}
		memcpy(mappedData, buffer, (size_t)bufferSize);
		vmaUnmapMemory(device->m_allocator, stagingBuffer.alloc);	

		std::vector<VkBufferImageCopy>bufferCopyRegion = mipInfo;

		AllocateImageMemory(device,imageUsageFlags);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 1;

		// Image barrier for optimal image.image (target)
		// Optimal image.image will be used as destination for the copy
		oGFX::vkutils::tools::setImageLayout(
			copyCmd,
			image.image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer.buffer,
			image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			bufferCopyRegion.size(), // copy over as many mips as have
			bufferCopyRegion.data()
		);

		// Change texture image.image layout to shader read after all mip levels have been copied		
		oGFX::vkutils::tools::setImageLayout(
			copyCmd,
			image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			imageLayout,
			subresourceRange);
		this->currentLayout = imageLayout;

		device->FlushCommandBuffer(copyCmd, copyQueue, device->commandPoolManagers[0].m_commandpool);

		// Clean up staging resources
		vmaDestroyBuffer(device->m_allocator, stagingBuffer.buffer, stagingBuffer.alloc);

		

		//// Create image.image view
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = NULL;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		viewCreateInfo.subresourceRange.levelCount = mipLevels; // generate mip maps will set this
		viewCreateInfo.image = image.image;
		//vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &view);
		//VK_NAME(device->logicalDevice, "fromBuffer::view", view);

		CreateImageView();


		for (size_t i = 0; i < mipLevels; i++)
		{
			viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			viewCreateInfo.subresourceRange.levelCount = 1;
			viewCreateInfo.subresourceRange.baseMipLevel = i;
			viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

			//vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &mipChainViews[i]);
			//VK_NAME(device->logicalDevice, "fromBuffer::viewMip", &mipChainViews[i]);
		}

		// Update descriptor image.image info member that can be used for setting up descriptor sets
		updateDescriptor();
	}

	void Texture2D::AllocateImageMemory(VulkanDevice* device, const VkImageUsageFlags& imageUsageFlags, uint32_t mips)
	{
		mipLevels = mips;

		// Create optimal tiled target image.image
		VkImageCreateInfo imageCreateInfo = oGFX::vkutils::inits::imageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.mipLevels = mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { width, height, 1 };
		imageCreateInfo.usage = imageUsageFlags;
		// Ensure that the TRANSFER_DST bit is set for staging
		// if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
		{
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		
		VmaAllocationCreateInfo allocCI{};
		allocCI.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCI.usage = VMA_MEMORY_USAGE_AUTO;
		allocCI.priority = 1.0f;

		VkResult result = vmaCreateImage(device->m_allocator, &imageCreateInfo, &allocCI, &image.image, &image.allocation, &image.allocationInfo);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Failed to create a image!" << std::endl;
			__debugbreak();
		}
		
		VK_NAME(device->logicalDevice, name.empty() ? "AllocateImage" : name.c_str(), image.image);
	}

	void Texture2D::forFrameBuffer(VulkanDevice* device,
		VkFormat _format,
		VkImageUsageFlags imageUsageFlags,
		uint32_t texWidth, uint32_t texHeight,
		bool forFullscr,
		float _renderscale,
		uint32_t _mipLevels, 
		VkImageLayout _imageLayout, // = VK_IMAGE_LAYOUT_UNDEFINED
		VkMemoryPropertyFlags properties,
		VkFilter _filter
	)
	{
		this->device = device;
		targetSwapchain = forFullscr;
		renderScale = _renderscale;
		width = static_cast<uint32_t>(texWidth * renderScale);
		height = static_cast<uint32_t>(texHeight* renderScale);
		format = _format;
		filter = _filter;
		imageLayout = _imageLayout;

		aspectMask = 0;
		if (imageLayout == VK_IMAGE_LAYOUT_UNDEFINED) // no user defined layout, set automatically
		{
			if (imageUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			else if (imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;
				imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
		}		

		usage = imageUsageFlags;

		// for blitting
		usage = imageUsageFlags | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		
		assert(aspectMask > 0);

		//uint mipLevels = std::floor(std::log2(std::max(texWidth, texHeight))) + 1;
		mipLevels = 1;

		bool n = name.empty();
		AllocateImageMemory(device, usage);

		CreateImageView();

		filter = _filter;

			
	}

	void Texture2D::Resize(uint32_t texWidth, uint32_t texHeight)
	{
		if (device == nullptr)
			return;

		width = static_cast<uint32_t>(texWidth * renderScale);
		height = static_cast<uint32_t>(texHeight * renderScale);

		VkImageView oldview = view;
		VmaAllocation oldMemory = image.allocation;
		VkImage oldImage = image.image;

		vkDestroyImageView(device->logicalDevice, oldview, nullptr);
		vmaDestroyImage(device->m_allocator, oldImage, oldMemory);

		image.allocation = VK_NULL_HANDLE;
		image.image = VK_NULL_HANDLE;

		bool n = name.empty();

		AllocateImageMemory(device, usage);

		CreateImageView();

	}

	void Texture2D::Update(void* buffer, VkDeviceSize bufferSize, 
		VkFormat _format, uint32_t texWidth, uint32_t texHeight, 
		std::vector<VkBufferImageCopy> mipInfo, VulkanDevice* device,
		VkQueue copyQueue, VkFilter _filter, VkImageUsageFlags imageUsageFlags)
	{
		assert(buffer);

		assert(this->device);
		assert(width == texWidth);
		assert(height == texHeight);
		assert(format == _format);
		assert(usage += imageUsageFlags);
		assert(mipLevels == static_cast<uint32_t>(mipInfo.size()));
		assert(image.image);

		

		// Use a separate command buffer for texture loading
		VkCommandBuffer copyCmd = device->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,device->commandPoolManagers[0].m_commandpool, true);

		
		oGFX::AllocatedBuffer stagingBuffer{};
		oGFX::CreateBuffer(device->m_allocator, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer);

		void* mappedData = nullptr;
		auto result = vmaMapMemory(device->m_allocator, stagingBuffer.alloc, &mappedData);
		if (result != VK_SUCCESS)
		{
			assert(false);
		}
		memcpy(mappedData, buffer, (size_t)bufferSize);
		vmaUnmapMemory(device->m_allocator, stagingBuffer.alloc);

		std::vector<VkBufferImageCopy>bufferCopyRegion = mipInfo;

		// be careful
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 1;

		// Image barrier for optimal image.image (target)
		// Optimal image.image will be used as destination for the copy
		oGFX::vkutils::tools::setImageLayout(
			copyCmd,
			image.image,
			this->currentLayout,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer.buffer,
			image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mipLevels,
			bufferCopyRegion.data()
		);

		// Change texture image.image layout to shader read after all mip levels have been copied
		oGFX::vkutils::tools::setImageLayout(
			copyCmd,
			image.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			this->currentLayout,
			subresourceRange);

		device->FlushCommandBuffer(copyCmd, copyQueue,device->commandPoolManagers[0].m_commandpool);

		// Clean up staging resources
		vmaDestroyBuffer(device->m_allocator, stagingBuffer.buffer, stagingBuffer.alloc);
	}

	void Texture2D::CreateImageView()
	{
		assert(view == VK_NULL_HANDLE);

		// Create image.image view
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.pNext = NULL;
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange = { aspectMask, 0, 1, 0, 1 };
		viewCreateInfo.subresourceRange.levelCount = mipLevels;
		viewCreateInfo.image = image.image;
		VK_CHK(vkCreateImageView(device->logicalDevice, &viewCreateInfo, nullptr, &view));
		VK_NAME(device->logicalDevice, name.empty() ? "CreateImage::view" : name.c_str(), view);
	}

	void TransitionImage(VkCommandBuffer cmd, Texture2D& texture, VkImageLayout targetLayout, uint32_t mipBegin, uint32_t mipEnd)
	{
		//printf("\t Transition::%s -> %s\n", texture.name, oGFX::vkutils::tools::VkImageLayoutString(targetLayout).c_str());

		if (texture.currentLayout == targetLayout) return; // might bug with write

		auto subresrange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		if (texture.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			subresrange =  VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
		}
		
		// default behavior transitiona all mips
		subresrange.baseMipLevel = mipBegin;
		subresrange.levelCount = mipEnd - mipBegin;
		
		if (mipEnd == 0) 
		{// transition some mips
			subresrange.levelCount = texture.mipLevels - mipBegin;
		}

		oGFX::vkutils::tools::insertImageMemoryBarrier(
			cmd,
			texture.image.image,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			texture.currentLayout,
			targetLayout,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			subresrange);
		texture.currentLayout = targetLayout;
	}

	void ComputeImageBarrier(VkCommandBuffer cmd, Texture2D& texture, VkImageLayout targetLayout, uint32_t mipBegin, uint32_t mipEnd)
	{
		auto subresrange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		if (texture.format == VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			subresrange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1 };
		}
		
		// default behavior transitiona all mips
		subresrange.baseMipLevel = mipBegin;
		subresrange.levelCount = mipEnd - mipBegin;

		if (mipEnd == 0)
		{// transition some mips
			subresrange.levelCount = texture.mipLevels - mipBegin;
		}

		oGFX::vkutils::tools::insertImageMemoryBarrier(
			cmd,
			texture.image.image,
			VK_ACCESS_MEMORY_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			texture.currentLayout,
			targetLayout,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			subresrange);
		texture.currentLayout = targetLayout;
	}

}
