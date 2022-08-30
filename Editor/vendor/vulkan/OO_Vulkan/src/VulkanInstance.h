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
	void CreateSurface(Window& window);
	
	VkInstance GetInstancePtr();

	VkSurfaceKHR surface{VK_NULL_HANDLE};
	VkInstance instance{VK_NULL_HANDLE};
};

