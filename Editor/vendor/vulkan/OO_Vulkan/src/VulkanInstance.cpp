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
#ifdef _DEBUG
		"VK_LAYER_KHRONOS_validation"
#endif // DEBUG
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
	appInfo.apiVersion = VK_API_VERSION_1_2;				//vulkan version, might need to change

	//create list to hold instance extensions
	std::vector<const char *> requiredExtensions = setupSpecs.extensions;

	if (setupSpecs.extensions.empty())
	{
		requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	
		// Enable surface extensions depending on os
		#if defined(_WIN32)
				requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
				requiredExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
		#elif defined(_DIRECT2DISPLAY)
				requiredExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
				requiredExtensions.push_back(VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
				requiredExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_XCB_KHR)
				requiredExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_IOS_MVK)
				requiredExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
		#elif defined(VK_USE_PLATFORM_MACOS_MVK)
				requiredExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
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
#ifdef _DEBUG
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			requiredExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			//
			// Enable render doc if requested by the user
			//
			if(setupSpecs.renderDoc == true)
			{
				validationLayers.emplace_back( "VK_LAYER_RENDERDOC_Capture" );
			}
#endif // DEBUG
		}
	}

	//check if instance extensions are supported
	if (!checkInstanceExtensionSupport(&requiredExtensions))
	{
		throw std::runtime_error("VkInstance does not support required supportedExtensions!");
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

	//create instance


	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a runtime instance!\n" + oGFX::vkutils::tools::VkResultString(result));
	}

	return true;
}

void VulkanInstance::CreateSurface(Window& window)
{
	//
	// Create the surface
	//
	

	// Get the Surface creation extension since we are about to use it
	auto VKCreateWin32Surface = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (nullptr == VKCreateWin32Surface)
	{
		throw std::runtime_error("Vulkan Driver missing the VK_KHR_win32_surface extension");
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
		throw std::runtime_error("Vulkan Fail to create window surface");
	}

	// set surface for imgui
	Window::SurfaceFormat = (uint64_t)surface;

}

VkInstance VulkanInstance::GetInstancePtr()
{
	return instance;
}

