/************************************************************************************//*!
\file           VulkanSwapchain.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Swapchain wrapper for vulkan

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
	std::vector<vkutils::Texture> swapChainImages;
	uint32_t minImageCount;

	vkutils::Texture2D depthAttachment;

	//utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkFormat depthFormat;

	VulkanDevice* m_devicePtr;

};

