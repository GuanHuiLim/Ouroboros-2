#pragma once
#include "VulkanUtils.h"
#include "VulkanTexture.h"

struct VulkanDevice;

struct VulkanSwapchain
{
	~VulkanSwapchain();
	void Init(VulkanInstance& instance,VulkanDevice& device);
	void CreateDepthBuffer();

	VkSwapchainKHR swapchain;
	
	// These textures are just used as containers
	std::vector<vkutils::Texture2D> swapChainImages;
	uint32_t minImageCount;

	vkutils::Texture2D depthAttachment;

	//utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkFormat depthFormat;

	VulkanDevice* m_devicePtr;

};

