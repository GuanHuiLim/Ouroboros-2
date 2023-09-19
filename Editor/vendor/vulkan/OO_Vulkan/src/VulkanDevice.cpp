/************************************************************************************//*!
\file           VulkanDevice.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a vulkan device wrapper

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#include "VulkanDevice.h"

#include "VulkanInstance.h"
#include "Window.h"

#include <vector>
#include <stdexcept>
#include <set>

#include "VulkanUtils.h"

VulkanDevice::~VulkanDevice()
{
    for (size_t i = 0; i < 2; i++)
    {
        commandPoolManagers[i].DestroyPool();
        //if (transferPools[i])
        //{
        //    //vkDestroyCommandPool(logicalDevice, transferPools[i], nullptr);
        //}
    }

    if (m_allocator)
    {
        vmaDestroyAllocator(m_allocator);
        m_allocator = NULL;
    }

    // no need destory phys device
	if (logicalDevice)
	{
		vkDestroyDevice(logicalDevice,NULL);
		logicalDevice = NULL;
	}

}

void VulkanDevice::InitPhysicalDevice(const oGFX::SetupInfo& si, VulkanInstance& instance)
{
	m_instancePtr = &instance;
	// Enumerate over physical devices the VK instance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.instance, &deviceCount, nullptr);

	// If no devices available, then none support vulkan!
	if (deviceCount == 0)
	{
		std::cerr << "Can't find GPUs that support vulkan instance!" << std::endl;
        __debugbreak();
	}

	// Get ist of physical devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance.instance, &deviceCount, deviceList.data());

	uint32_t best = 0;
	uint32_t memory = 0;
	for (size_t i = 0; i < deviceList.size(); i++)
	{
		auto& device = deviceList[i];
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(device, &props);
		if (props.limits.sparseAddressSpaceSize > memory)
		{
			memory = props.limits.sparseAddressSpaceSize;
			std::swap(deviceList[i], deviceList[best]);
            best = static_cast<uint32_t>(i);
		}
	}

	// find a suitable device
	for (const auto& device : deviceList)
	{
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && CheckDeviceSuitable(si, device))
		{
            
            printf("Selected device %s\n", props.deviceName);
			//found a nice device
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE)
	{
		std::cerr << "No suitable physical device found!" << std::endl;
        __debugbreak();
	}

}

void VulkanDevice::InitLogicalDevice(const oGFX::SetupInfo& si,VulkanInstance& instance)
{

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    //get the queue family indices for the chosen Physical Device
    queueIndices = oGFX::GetQueueFamilies(physicalDevice, instance.surface);
    oGFX::QueueFamilyIndices& indices = queueIndices;

    //vector for queue creation information and set for family indices
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices = { indices.graphicsFamily,indices.presentationFamily, indices.transferFamily };
    
    float priority = 1.0f;
    //queues the logical device needs to create in the info to do so.
    for (int queueFamilyIndex : queueFamilyIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //the index of the family to create a queue from
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        //number of queues to create
        queueCreateInfo.queueCount = 1;
        //vulkan needs to know how to handle multiple queues and thus we need a priority, 1.0 is the highest priority
        
        queueCreateInfo.pQueuePriorities = &priority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    std::vector<const char*>deviceExtensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    };

    if (si.debug && si.renderDoc)
    {
#if VULKAN_VALIDATION
        deviceExtensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME); 
#endif // VULKAN_VALIDATION
    }

    //information to create logical device (somtimes called only "device")
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    //queue create info so the device can create required queues
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    //number of enabled logical device extensions
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    //all features will be disabled by default
    VkPhysicalDeviceFeatures deviceFeatures = {};
    //physical device features that logical device will use
    deviceFeatures.samplerAnisotropy = VK_TRUE; // Enabling anisotropy
    deviceFeatures.multiDrawIndirect = VK_TRUE;
    
    deviceFeatures.fillModeNonSolid = VK_TRUE;  //wireframe drawing
    deviceFeatures.drawIndirectFirstInstance = VK_TRUE;
    deviceFeatures.independentBlend = VK_TRUE;

    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
    dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRendering.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceVulkan12Features vk12Features{};
    vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk12Features.descriptorIndexing = VK_TRUE;
    vk12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vk12Features.runtimeDescriptorArray = VK_TRUE; 
    vk12Features.descriptorBindingVariableDescriptorCount = VK_TRUE; 
    vk12Features.descriptorBindingPartiallyBound = VK_TRUE; 
    vk12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE; 
    vk12Features.timelineSemaphore = VK_TRUE;
    

    // required for instance base vertex
    VkPhysicalDeviceShaderDrawParametersFeatures shaderDrawFeatures{};
    shaderDrawFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shaderDrawFeatures.shaderDrawParameters = VK_TRUE;

    VkPhysicalDeviceMaintenance4FeaturesKHR maintainence4{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES }; // AMD_SPD req: LocalSizeId 
    maintainence4.maintenance4 = VK_TRUE;
    // Bindless design requirement Descriptor indexing for descriptors // contained in vulkan12 features
    // VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features{};
    // descriptor_indexing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    // // Enable non-uniform indexing
    // descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    // descriptor_indexing_features.runtimeDescriptorArray = VK_TRUE;
    // descriptor_indexing_features.descriptorBindingVariableDescriptorCount = VK_TRUE;
    // descriptor_indexing_features.descriptorBindingPartiallyBound = VK_TRUE;
    // descriptor_indexing_features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE; // needed for image descriptors
    VkPhysicalDeviceSynchronization2Features sync2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
    sync2.synchronization2 = VK_TRUE;

    deviceCreateInfo.pNext = &vk12Features;
    vk12Features.pNext = &shaderDrawFeatures;
    shaderDrawFeatures.pNext = &dynamicRendering;
    dynamicRendering.pNext = &maintainence4;
    maintainence4.pNext = &sync2;
    sync2.pNext = NULL;

    this->enabledFeatures = deviceFeatures;

    // TODO: memory management
    // Create logical device for the given physical device
    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
    if (result != VK_SUCCESS)
    {
		std::cerr << "VK Renderer Failed to create a logical device!\n" << oGFX::vkutils::tools::VkResultString(result) << std::endl;
        __debugbreak();
    }
    VK_NAME(logicalDevice, "logicalDevice", logicalDevice);

    // Queues are created as the same time as the device...
    // So we want to handle the queues
    // From given logical device of given queue family of given index, place reference in VKqueue
    vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily; //Queue family type that buffers from this command pool will use


    commandPoolManagers.resize(2);
    // commandPools.resize(2);
    //transferPools.resize(2);
    for (size_t i = 0; i < 2; i++)
    {
        result = commandPoolManagers[i].InitPool(logicalDevice, indices.graphicsFamily);
        //create a graphics queue family command pool
        // poolInfo.queueFamilyIndex = indices.graphicsFamily; //Queue family type that buffers from this command pool will use
        // result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPools[i]);
        // VK_NAME(logicalDevice, "commandPool", commandPools[i]);
        // 
        // //poolInfo.queueFamilyIndex = indices.transferFamily;
        // //result = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &transferPools[i]);
        // //VK_NAME(logicalDevice, "transferPool", transferPools[i]);
        // transferPools[i] = commandPools[i];
        if (result != VK_SUCCESS)
        {
            std::cerr << "Failed to create a command pool!" << std::endl;
            __debugbreak();
        }
    }
  

}

void VulkanDevice::InitAllocator(const oGFX::SetupInfo& si, VulkanInstance& instance)
{

    VmaVulkanFunctions vulkanFuns{};
    vulkanFuns.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFuns.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice= physicalDevice;
    allocatorInfo.device = logicalDevice;
    allocatorInfo.flags = {};
    allocatorInfo.instance = instance.instance;
    allocatorInfo.pVulkanFunctions = &vulkanFuns;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    VK_CHK(vmaCreateAllocator(&allocatorInfo, &m_allocator));
}

bool VulkanDevice::CheckDeviceSuitable(const oGFX::SetupInfo& si,VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	oGFX::QueueFamilyIndices indices = oGFX::GetQueueFamilies(device,m_instancePtr->surface);

	bool extensionsSupported = CheckDeviceExtensionSupport(si,device);

	bool swapChainValid = false;
	if (extensionsSupported)
	{
		oGFX::SwapChainDetails swapChainDetails = oGFX::GetSwapchainDetails(*m_instancePtr,device);
		swapChainValid = !swapChainDetails.presentationModes.empty() && !swapChainDetails.formats.empty();
	}
	return indices.isValid() && extensionsSupported && swapChainValid && deviceFeatures.samplerAnisotropy;
}


bool VulkanDevice::CheckDeviceExtensionSupport(const oGFX::SetupInfo& si,VkPhysicalDevice device)
{
    // get device extension count
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    //if no devices return failure
    if (extensionCount == 0)
    {
        return false;
    } 

    //populate list of extensions
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    std::vector<const char*>deviceExtensions   = 
    { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
        //        ,   VK_NV_GLSL_SHADER_EXTENSION_NAME  // nVidia useful extension to be able to load GLSL shaders
    };
    // TODO BETTER

    if (si.debug && si.renderDoc)
    {
#if VULKAN_VALIDATION
        deviceExtensions.emplace_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
#endif // VULKAN_VALIDATION
    }

    //check extensions
    for (const auto &deviceExtension : deviceExtensions)
    {
        bool hasExtension = false;
        for (const auto &extension : extensions)
        {
            if (strcmp(deviceExtension, extension.extensionName) == 0)
            {
                hasExtension = true;
                break;
            }
        }
        if (!hasExtension)
        {
            //TODO: what extension not supported
            std::cerr << std::string("Does not support extension ") + deviceExtension << std::endl;
            return false;
        }
    }

    return true;
}


VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = oGFX::vkutils::inits::commandBufferAllocateInfo(pool, level, 1);
    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, &cmdBuffer);
    // If requested, also start recording for the new command buffer
    if (begin)
    {
        VkCommandBufferBeginInfo cmdBufInfo = oGFX::vkutils::inits::commandBufferBeginInfo();
        vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo);
    }
    return cmdBuffer;
}

//VkCommandBuffer VulkanDevice::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
//{
//    return CreateCommandBuffer(level, commandPool, begin);
//}

void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
{
    if (commandBuffer == VK_NULL_HANDLE)
    {
        return;
    }

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = oGFX::vkutils::inits::submitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceInfo = oGFX::vkutils::inits::fenceCreateInfo(0);
    VkFence fence;
    vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fence);
    VK_NAME(logicalDevice, "fence", fence);
    // Submit to the queue
    vkQueueSubmit(queue, 1, &submitInfo, fence);
    // Wait for the fence to signal that command buffer has finished executing
    vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, 100000000000);
    vkDestroyFence(logicalDevice, fence, nullptr);
    if (free)
    {
        vkFreeCommandBuffers(logicalDevice, pool, 1, &commandBuffer);
    }
}

//void VulkanDevice::FlushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
//{
//    return FlushCommandBuffer(commandBuffer, queue, commandPool, free);
//}


