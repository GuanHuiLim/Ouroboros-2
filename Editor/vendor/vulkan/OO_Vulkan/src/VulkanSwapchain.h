#pragma once
#include "VulkanUtils.h"
#include "VulkanFramebufferAttachment.h"

struct VulkanDevice;
struct oGFX::SwapChainImage;
struct VulkanSwapchain
{
	~VulkanSwapchain();
	void Init(VulkanInstance& instance,VulkanDevice& device);
	void CreateDepthBuffer();

	VkSwapchainKHR swapchain;
	std::vector<oGFX::SwapChainImage> swapChainImages;
	uint32_t minImageCount;

	VulkanFramebufferAttachment depthAttachment;

	//utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkFormat depthFormat;

	VulkanDevice* m_devicePtr;

};

