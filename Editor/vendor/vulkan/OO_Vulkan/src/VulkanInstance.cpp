/************************************************************************************//*!
\file           VulkanInstance.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief               Defines a vulkan instance wrapper

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "VulkanInstance.h"

#include "VulkanUtils.h"
#include "Window.h"
#include <memory>
#include <stdexcept>
#include <vector>
#include <array>


VkResult FilterValidationLayers(std::vector<const char*>& Layers, const std::vector<const char* >& FilterView)
{
	uint32_t    ValidationLayerCount = 0;
	VkResult Err = vkEnumerateInstanceLayerProperties( &ValidationLayerCount, NULL );
	if(  Err ) 
	{
		return Err;
	}

    std::vector<VkLayerProperties>LayerProperties(ValidationLayerCount);
	Err = vkEnumerateInstanceLayerProperties( &ValidationLayerCount, LayerProperties.data() );
	if(   Err )
	{
		return Err;
	}

	//
	// Find all possible filters from our FilterView
	//    
	Layers.clear();
	for ( const auto& LayerEntry :LayerProperties)
		for( const auto& pFilterEntry : FilterView )
			if( strcmp( pFilterEntry, LayerEntry.layerName ) == 0 )
			{
				Layers.push_back(pFilterEntry);
				break;
			}

	return VK_SUCCESS;
}

std::vector<const char*> getSupportedValidationLayers(VulkanInstance& vkinstance)
{
	auto s_ValidationLayerNames_Alt1 = std::vector<const char*>
	{
		"VK_LAYER_KHRONOS_validation"
	};

	auto s_ValidationLayerNames_Alt2 = std::vector<const char*>
	{
		"VK_LAYER_GOOGLE_threading",     "VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_image",         "VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_swapchain",     "VK_LAYER_GOOGLE_unique_objects",
	};

	// Try alt list first 
	std::vector<const char*> Layers{};

	FilterValidationLayers(Layers, s_ValidationLayerNames_Alt1 );
	if (Layers.size() != s_ValidationLayerNames_Alt1.size())
	{
		//Instance.ReportWarning("Fail to get the standard validation layers");

		// if we are not successfull then try our best with the second version
		FilterValidationLayers(Layers, s_ValidationLayerNames_Alt2);
		if (Layers.size() != s_ValidationLayerNames_Alt2.size())
		{
			//Instance.ReportWarning("Fail to get all the basoc validation layers that we wanted");
		}
	}

	// return without move because RVO
	return Layers;
}

bool checkInstanceExtensionSupport(std::vector<const char *> *extensionsToCheck)
{
	//need to get numbe rof extensions to create array size to hold extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	//create a list of VKextension properties usuing count
	std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExtensions.data());

	for (const auto &checkExtension : *extensionsToCheck)
	{
		bool hasExtension = false;
		for (const auto &extension : supportedExtensions)
		{
			if (strcmp(checkExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}

	// we are sure we have all supported extension
	return true;
}

VulkanInstance::~VulkanInstance()
{
	if (surface)
	{
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	if (instance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(instance, nullptr);
	}
	instance = VK_NULL_HANDLE;
}

bool VulkanInstance::Init(const oGFX::SetupInfo& setupSpecs)
{
	//information about the application.
	//does not affect the program and is for dev convenience
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App";				//custom application name
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);	//custom version of the application
	appInfo.pEngineName = "Engine";							//custom engine name
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);		//custom engine version
	appInfo.apiVersion = VK_API_VERSION_1_3;				//vulkan version, might need to change

	//create list to hold instance extensions
	std::vector<const char *> requiredExtensions = setupSpecs.extensions;

	if (setupSpecs.extensions.empty())
	{
		requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		// Win32 Surface
#if defined(_WIN32)
		requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
		static_assert(false, "This engine only works on WIN32...");
#endif
	}

	//requiredExtensions.push_back(// , VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME );     // (NOT WORKING ON MANY GRAPHICS CARDS WILL HAVE TO WAIT...) Allows to change the vertex input to a pipeline (which should have been the default behavior)
	requiredExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); // descriptor indexing

	//create list for validation layers
	std::vector<const char*> validationLayers{};
	//
	// Check if we need to set any validation layers
	//
	if (setupSpecs.debug == true)
	{
		validationLayers = getSupportedValidationLayers( *this );
		if( validationLayers.size() ) 
		{
#if VULKAN_VALIDATION
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

			//
			// Enable render doc if requested by the user
			//
			if(setupSpecs.renderDoc == true)
			{
				validationLayers.emplace_back( "VK_LAYER_RENDERDOC_Capture" );
				//validationLayers.emplace_back( "VK_LAYER_LUNARG_api_dump" ); // for nuclear debugging
			}
#endif // VULKAN_VALIDATION
		}
	}

	//check if instance extensions are supported
	if (!checkInstanceExtensionSupport(&requiredExtensions))
	{
		std::cerr << "VkInstance does not support required supportedExtensions!" << std::endl;
		__debugbreak();
	}

	//create information for a VKinstance
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	if (setupSpecs.debug)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Failed to create a runtime instance!" << std::endl;
		__debugbreak();
	}

	return oGFX::SUCCESS_VAL;
}

bool VulkanInstance::CreateSurface(Window& window, VkSurfaceKHR& surface)
{
	//
	// Create the surface
	//

	// Get the Surface creation extension since we are about to use it
	// TODO: do once..
	auto VKCreateWin32Surface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (nullptr == VKCreateWin32Surface)
	{
		std::cerr << "Vulkan Driver missing the VK_KHR_win32_surface extension" << std::endl;
		__debugbreak();
		return oGFX::ERROR_VAL;
	}

	VkWin32SurfaceCreateInfoKHR SurfaceCreateInfo;
	{
		SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		SurfaceCreateInfo.pNext = nullptr;
		SurfaceCreateInfo.flags = 0;
		SurfaceCreateInfo.hinstance = GetModuleHandle(NULL);
		SurfaceCreateInfo.hwnd = reinterpret_cast<HWND>(window.GetRawHandle());
	}

	if (auto VKErr = VKCreateWin32Surface(instance, &SurfaceCreateInfo, nullptr, &surface); VKErr)
	{
		std::cerr << "Vulkan Fail to create window surface" << std::endl;
		__debugbreak();
		return oGFX::ERROR_VAL;
	}

	return oGFX::SUCCESS_VAL;

}

VkInstance VulkanInstance::GetInstancePtr()
{
	return instance;
}

