#include "VulkanFramebufferAttachment.h"
#include <assert.h>
//#include "VulkanUtils.h"
//
//void VulkanFramebufferAttachment::createAttachment(VulkanDevice& indevice, uint32_t width, uint32_t height,
//	VkFormat format, VkImageUsageFlagBits usage, const char* name)
//{
//	VkImageAspectFlags aspectMask = 0;
//	VkImageLayout imageLayout;
//	VkDevice device = indevice.logicalDevice;
//
//	this->format = format;
//
//	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
//	{
//		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//	}
//	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
//	{
//		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;// | VK_IMAGE_ASPECT_STENCIL_BIT;
//		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//	}
//
//	assert(aspectMask > 0);
//	layout = imageLayout;
//
//	VkImageCreateInfo image = oGFX::vkutils::inits::imageCreateInfo();
//	image.imageType = VK_IMAGE_TYPE_2D;
//	image.format = format;
//	image.extent.width = width;
//	image.extent.height = height;
//	image.extent.depth = 1;
//	image.mipLevels = 1;
//	image.arrayLayers = 1;
//	image.samples = VK_SAMPLE_COUNT_1_BIT;
//	image.tiling = VK_IMAGE_TILING_OPTIMAL;
//	image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
//
//	VkMemoryAllocateInfo memAlloc = oGFX::vkutils::inits::memoryAllocateInfo();
//	VkMemoryRequirements memReqs;
//
//	
//	VK_CHK(vkCreateImage(device, &image, nullptr, &this->image));
//	if (name)
//	{
//		std::string img= std::string("createAttachment::") +name;
//		VK_NAME(device, img.c_str(), this->image);
//	}
//	else
//	{
//		VK_NAME(device, "createAttachment::image", this->image);
//	}
//	
//	vkGetImageMemoryRequirements(device, this->image, &memReqs);
//	memAlloc.allocationSize = memReqs.size;
//
//	memAlloc.memoryTypeIndex = oGFX::FindMemoryTypeIndex(indevice.physicalDevice,
//		memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
//	VK_CHK(vkAllocateMemory(device, &memAlloc, nullptr, &this->mem));
//	VK_CHK(vkBindImageMemory(device, this->image, this->mem, 0));
//
//	VkImageViewCreateInfo imageView = oGFX::vkutils::inits::imageViewCreateInfo();
//	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
//	imageView.format = format;
//	imageView.subresourceRange = {};
//	imageView.subresourceRange.aspectMask = aspectMask;
//	imageView.subresourceRange.baseMipLevel = 0;
//	imageView.subresourceRange.levelCount = 1;
//	imageView.subresourceRange.baseArrayLayer = 0;
//	imageView.subresourceRange.layerCount = 1;
//	imageView.image = this->image;
//	VK_CHK(vkCreateImageView(device, &imageView, nullptr, &this->view));
//	if (name)
//	{
//		std::string img= std::string("createAttachment::") +name;
//		VK_NAME(device, img.c_str(), this->view);
//	}
//	else
//	{
//		VK_NAME(device, "createAtt::imgView", this->view);
//	}
//	
//}
//
//void VulkanFramebufferAttachment::destroy(VkDevice device)
//{
//	vkDestroyImageView(device, view, nullptr);
//	vkDestroyImage(device, image, nullptr);
//	vkFreeMemory(device, mem, nullptr);
//}
