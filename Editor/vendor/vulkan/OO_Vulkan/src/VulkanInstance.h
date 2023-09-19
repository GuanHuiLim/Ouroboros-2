/************************************************************************************//*!
\file           VulkanInstance.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a vulkan instance wrapper

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "gpuCommon.h"

struct Window;
struct VulkanInstance
{

	VulkanInstance() = default;
	~VulkanInstance();
	bool Init(const oGFX::SetupInfo& si);
	bool CreateSurface(Window& window, VkSurfaceKHR& surface);
	
	VkInstance GetInstancePtr();

	VkSurfaceKHR surface{VK_NULL_HANDLE};
	VkInstance instance{VK_NULL_HANDLE};
};

