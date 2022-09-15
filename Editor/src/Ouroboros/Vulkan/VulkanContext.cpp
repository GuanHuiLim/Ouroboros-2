/************************************************************************************//*!
\file           VulkanContext.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 17, 2022
\brief          Describes the Implementation of the Vulkan Backend Graphics Context
                and capabilities.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Ouroboros/Vulkan/VulkanContext.h"
#include "Ouroboros/Core/Timer.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include <sdl2/SDL.h>
#include <sdl2/SDL_vulkan.h>

#include "OO_Vulkan/src/DefaultMeshCreator.h"

namespace oo
{
    //VulkanEngine VulkanContext::vkEngine;
    
      
    VulkanRenderer* VulkanContext::vr{nullptr};
    GraphicsWorld VulkanContext::gw;
    Window VulkanContext::m_window;

//#define TEMPORARY_CODE

#ifdef TEMPORARY_CODE
    //#define IMGUI_UNLIMITED_FRAME_RATE
    #ifdef _DEBUG
    #define IMGUI_VULKAN_DEBUG_REPORT
    #endif

    static VkAllocationCallbacks*   g_Allocator = NULL;
    static VkInstance               g_Instance = VK_NULL_HANDLE;
    static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
    static VkDevice                 g_Device = VK_NULL_HANDLE;
    static uint32_t                 g_QueueFamily = (uint32_t)-1;
    static VkQueue                  g_Queue = VK_NULL_HANDLE;
    static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
    static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
    static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

    static ImGui_ImplVulkanH_Window g_MainWindowData;
    static uint32_t                 g_MinImageCount = 2;
    static bool                     g_SwapChainRebuild = false;

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
    {
        (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
        fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
        return VK_FALSE;
    }
#endif // IMGUI_VULKAN_DEBUG_REPORT

    static void SetupVulkan(const char** extensions, uint32_t extensions_count)
    {
        VkResult err;

        // Create Vulkan Instance
        {
            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.enabledExtensionCount = extensions_count;
            create_info.ppEnabledExtensionNames = extensions;
#ifdef IMGUI_VULKAN_DEBUG_REPORT
            // Enabling validation layers
            const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
            create_info.enabledLayerCount = 1;
            create_info.ppEnabledLayerNames = layers;

            // Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
            const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
            memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
            extensions_ext[extensions_count] = "VK_EXT_debug_report";
            create_info.enabledExtensionCount = extensions_count + 1;
            create_info.ppEnabledExtensionNames = extensions_ext;

            // Create Vulkan Instance
            err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
            check_vk_result(err);
            free(extensions_ext);

            // Get the function pointer (required for any extensions)
            auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkCreateDebugReportCallbackEXT");
            IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);

            // Setup the debug report callback
            VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
            debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debug_report_ci.pfnCallback = debug_report;
            debug_report_ci.pUserData = NULL;
            err = vkCreateDebugReportCallbackEXT(g_Instance, &debug_report_ci, g_Allocator, &g_DebugReport);
            check_vk_result(err);
#else
            // Create Vulkan Instance without any debug feature
            err = vkCreateInstance(&create_info, g_Allocator, &g_Instance);
            check_vk_result(err);
            IM_UNUSED(g_DebugReport);
#endif
        }

        // Select GPU
        {
            uint32_t gpu_count;
            err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, NULL);
            check_vk_result(err);
            IM_ASSERT(gpu_count > 0);

            VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
            err = vkEnumeratePhysicalDevices(g_Instance, &gpu_count, gpus);
            check_vk_result(err);

            // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
            // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
            // dedicated GPUs) is out of scope of this sample.
            int use_gpu = 0;
            for (int i = 0; i < (int)gpu_count; i++)
            {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(gpus[i], &properties);
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    use_gpu = i;
                    break;
                }
            }

            g_PhysicalDevice = gpus[use_gpu];
            free(gpus);
        }

        // Select graphics queue family
        {
            uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, NULL);
            VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
            vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, queues);
            for (uint32_t i = 0; i < count; i++)
                if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    g_QueueFamily = i;
                    break;
                }
            free(queues);
            IM_ASSERT(g_QueueFamily != (uint32_t)-1);
        }

        // Create Logical Device (with 1 queue)
        {
            int device_extension_count = 1;
            const char* device_extensions[] = { "VK_KHR_swapchain" };
            const float queue_priority[] = { 1.0f };
            VkDeviceQueueCreateInfo queue_info[1] = {};
            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = g_QueueFamily;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = queue_priority;
            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
            create_info.pQueueCreateInfos = queue_info;
            create_info.enabledExtensionCount = device_extension_count;
            create_info.ppEnabledExtensionNames = device_extensions;
            err = vkCreateDevice(g_PhysicalDevice, &create_info, g_Allocator, &g_Device);
            check_vk_result(err);
            vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
        }

        // Create Descriptor Pool
        {
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            err = vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool);
            check_vk_result(err);
        }
    }

    // All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
    // Your real engine/app may not use them.
    static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
    {
        wd->Surface = surface;

        // Check for WSI support
        VkBool32 res;
        vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
        if (res != VK_TRUE)
        {
            fprintf(stderr, "Error no WSI support on physical device 0\n");
            exit(-1);
        }

        // Select Surface Format
        const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

        // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
        wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
        //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

        // Create SwapChain, RenderPass, Framebuffer, etc.
        IM_ASSERT(g_MinImageCount >= 2);
        ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
    }

    static void CleanupVulkan()
    {
        vkDestroyDescriptorPool(g_Device, g_DescriptorPool, g_Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
        // Remove the debug report callback
        auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_Instance, "vkDestroyDebugReportCallbackEXT");
        vkDestroyDebugReportCallbackEXT(g_Instance, g_DebugReport, g_Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

        vkDestroyDevice(g_Device, g_Allocator);
        vkDestroyInstance(g_Instance, g_Allocator);
    }

    static void CleanupVulkanWindow()
    {
        ImGui_ImplVulkanH_DestroyWindow(g_Instance, g_Device, &g_MainWindowData, g_Allocator);
    }

    static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
    {      
        VkResult err;


        VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            g_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);

        ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
        {
            err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
            check_vk_result(err);

            err = vkResetFences(g_Device, 1, &fd->Fence);
            check_vk_result(err);
        }
        {
            err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            check_vk_result(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = wd->Width;
            info.renderArea.extent.height = wd->Height;
            info.clearValueCount = 1;
            info.pClearValues = &wd->ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            check_vk_result(err);
            err = vkQueueSubmit(g_Queue, 1, &info, fd->Fence);
            check_vk_result(err);
        }
    }

    static void FramePresent(ImGui_ImplVulkanH_Window* wd)
    {
        if (g_SwapChainRebuild)
            return;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &wd->Swapchain;
        info.pImageIndices = &wd->FrameIndex;
        VkResult err = vkQueuePresentKHR(g_Queue, &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            g_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);
        wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
    }

#endif // TEMPORARY_CODE

    VulkanContext::VulkanContext(SDL_Window* window)
        : m_windowHandle(window)
    {
        ASSERT_MSG(m_windowHandle == nullptr, "Window handle is null!");

        //vkEngine.SetWindow(window);
    }

    VulkanContext::~VulkanContext()
    {
        // do nothing for now
        //vkEngine.cleanup();
        delete vr;

#ifdef TEMPORARY_CODE
        CleanupVulkanWindow();
        CleanupVulkan();
#endif // TEMPORARY_CODE

    }

    void VulkanContext::Init()
    {
        // Setup Vulkan
        uint32_t extensions_count = 0;
        SDL_Vulkan_GetInstanceExtensions(m_windowHandle, &extensions_count, NULL);
        std::vector<const char*> extensions;
        extensions.resize(extensions_count);
        SDL_Vulkan_GetInstanceExtensions(m_windowHandle, &extensions_count, &extensions[0]);

        int w, h;
        SDL_GetWindowSize(m_windowHandle, &w, &h);

        vr = VulkanRenderer::get();

        auto& camera = vr->camera;
        m_cc.SetCamera(&camera);

        camera.m_CameraMovementType = Camera::CameraMovementType::firstperson;
        camera.movementSpeed = 5.0f;

        camera.SetRotation(glm::vec3(0.0f, 180.0f, 0.0f));
        camera.SetRotationSpeed(0.5f);
        camera.SetPosition(glm::vec3(0.0f, 2.0f, 4.0f));
        camera.SetAspectRatio((float)m_window.m_width / (float)m_window.m_height);
        
        

        oGFX::SetupInfo si;
        si.debug = true;
        si.renderDoc = false;
        si.SurfaceFunctionPointer = std::function<void()>([&]() {
            return SDL_Vulkan_CreateSurface(m_windowHandle, vr->m_instance.instance, &vr->m_instance.surface);
            });
        si.extensions = extensions;
        m_window.m_width = w;
        m_window.m_height = h;
        m_window.m_type = Window::WindowType::SDL2;
        m_window.rawHandle = m_windowHandle;
        try
        {
            vr->Init(si, m_window);
        } 
        catch (std::runtime_error e)
        {
            std::cout << "VK_init: " << e.what() << std::endl;
        }

        // setup world..
        // TODO: move this out of here pls
        auto obj = gw.CreateObjectInstance();
        auto plane = gw.CreateObjectInstance();

        int32_t white = 0x00404040;
        auto tex = vr->CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&white));

        DefaultMesh dm = CreateDefaultCubeMesh();
        cubeMesh.reset(vr->LoadMeshFromBuffers(dm.m_VertexBuffer, dm.m_IndexBuffer, nullptr));
        DefaultMesh pm = CreateDefaultPlaneXZMesh();
        planeMesh.reset(vr->LoadMeshFromBuffers(pm.m_VertexBuffer, pm.m_IndexBuffer, nullptr));

        {
            auto& myObj = gw.GetObjectInstance(obj); 
            myObj.modelID = cubeMesh->gfxIndex;
            myObj.scale = glm::vec3{ 2.1f,1.1f,1.1f };
            myObj.rotVec = glm::vec3{ 1.1f,1.1f,1.1f };
            myObj.rot = 35.0f;
            myObj.localToWorld = glm::mat4(1.0f);
            myObj.localToWorld = glm::translate(myObj.localToWorld, myObj.position);
            myObj.localToWorld = glm::rotate(myObj.localToWorld,glm::radians(myObj.rot), myObj.rotVec);
            myObj.localToWorld = glm::scale(myObj.localToWorld, myObj.scale);
            myObj.bindlessGlobalTextureIndex_Albedo = tex;
        }
       
        {
            auto& myPlane = gw.GetObjectInstance(plane);
            myPlane.modelID = planeMesh->gfxIndex;
            myPlane.position = { 0.0f,-1.0f,0.0f };
            myPlane.scale = { 15.0f,1.0f,15.0f };
            myPlane.localToWorld = glm::mat4(1.0f);
            myPlane.localToWorld = glm::translate(myPlane.localToWorld, myPlane.position);
            myPlane.localToWorld = glm::rotate(myPlane.localToWorld,glm::radians(myPlane.rot), myPlane.rotVec);
            myPlane.localToWorld = glm::scale(myPlane.localToWorld, myPlane.scale);
            myPlane.bindlessGlobalTextureIndex_Albedo = tex;
        }
        

        vr->SetWorld(&gw);
#ifdef TEMPORARY_CODE 
        // Create Window Surface
        VkSurfaceKHR surface;
        VkResult err;
        if (SDL_Vulkan_CreateSurface(m_windowHandle, g_Instance, &surface) == 0)
        {
            printf("Failed to create Vulkan surface.\n");
            //return 1;
        }

        // Create Framebuffers
        ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
        SetupVulkanWindow(wd, surface, w, h);
#endif // TEMPORARY_CODE

    }

    void VulkanContext::OnUpdateBegin()
    {
        int w, h;
        SDL_Vulkan_GetDrawableSize(m_windowHandle, &w, &h);
        m_window.m_width = w;
        m_window.m_height = h;

        m_cc.Update(oo::timer::dt());
        if (vr->PrepareFrame() == true)
        {
            auto& obj = gw.GetObjectInstance(0);
            obj.rot += 0.25f;
            obj.localToWorld = glm::mat4(1.0f);
            obj.localToWorld = glm::translate(obj.localToWorld, obj.position);
            obj.localToWorld = glm::rotate(obj.localToWorld,glm::radians(obj.rot), obj.rotVec);
            obj.localToWorld = glm::scale(obj.localToWorld, obj.scale);

            {
                auto& lights = gw.m_HardcodedOmniLights;

                static float lightTimer = 0.0f;
                lightTimer += oo::timer::dt() * 0.25f;

                lights[0].position = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
                lights[0].color = glm::vec4(1.5f);
                lights[0].radius.x = 15.0f;
                // Red
                lights[1].position = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
                lights[1].color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
                lights[1].radius.x = 15.0f;
                // Blue
                lights[2].position = glm::vec4(2.0f, -1.0f, 0.0f, 0.0f);
                lights[2].color = glm::vec4(0.0f, 0.0f, 2.5f, 0.0f);
                lights[2].radius.x = 5.0f;
                // Yellow
                lights[3].position = glm::vec4(0.0f, -0.9f, 0.5f, 0.0f);
                lights[3].color = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
                lights[3].radius.x = 2.0f;
                // Green
                lights[4].position = glm::vec4(0.0f, -0.5f, 0.0f, 0.0f);
                lights[4].color = glm::vec4(0.0f, 1.0f, 0.2f, 0.0f);
                lights[4].radius.x = 5.0f;
                // Yellow
                lights[5].position = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
                lights[5].color = glm::vec4(1.0f, 0.7f, 0.3f, 0.0f);
                lights[5].radius.x = 25.0f;

                lights[0].position.x = sin(glm::radians(360.0f * lightTimer)) * 5.0f;
                lights[0].position.z = cos(glm::radians(360.0f * lightTimer)) * 5.0f;

                lights[1].position.x = -4.0f + sin(glm::radians(360.0f * lightTimer) + 45.0f) * 2.0f;
                lights[1].position.z = 0.0f + cos(glm::radians(360.0f * lightTimer) + 45.0f) * 2.0f;

                lights[2].position.x = 4.0f + sin(glm::radians(360.0f * lightTimer)) * 2.0f;
                lights[2].position.z = 0.0f + cos(glm::radians(360.0f * lightTimer)) * 2.0f;

                lights[4].position.x = 0.0f + sin(glm::radians(360.0f * lightTimer + 90.0f)) * 5.0f;
                lights[4].position.z = 0.0f - cos(glm::radians(360.0f * lightTimer + 45.0f)) * 5.0f;

                lights[5].position.x = 0.0f + sin(glm::radians(-360.0f * lightTimer + 135.0f)) * 10.0f;
                lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * lightTimer - 45.0f)) * 10.0f;
            }

            // Upload CPU light data to GPU. Ideally this should only contain lights that intersects the camera frustum.
            vr->UploadLights();

            // Render the frame
            vr->RenderFrame();
        }
    }

    void VulkanContext::SwapBuffers()
    {
        /*vkEngine.RenderFrame();
        if (!vkEngine._recreateSwapchain)
        {
            vkEngine.PresentFrame();
        }
        if (vkEngine._recreateSwapchain)
        {
            vkEngine.RecreateSwapchain();
        }*/
        vr->Present();

#ifdef TEMPORARY_CODE
        // Resize swap chain?
        if (g_SwapChainRebuild)
        {
            int width, height;
            SDL_GetWindowSize(m_windowHandle, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
                g_MainWindowData.FrameIndex = 0;
                g_SwapChainRebuild = false;
            }
        }
#endif // TEMPORARY_CODE
    }

    void VulkanContext::InitImGui()
    {
        ImGui_ImplSDL2_InitForVulkan(m_windowHandle);
        vr->InitImGUI();
#ifdef TEMPORARY_CODE
        ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
        // Setup Platform/Renderer backends
        
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = g_Instance;
        init_info.PhysicalDevice = g_PhysicalDevice;
        init_info.Device = g_Device;
        init_info.QueueFamily = g_QueueFamily;
        init_info.Queue = g_Queue;
        init_info.PipelineCache = g_PipelineCache;
        init_info.DescriptorPool = g_DescriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = g_MinImageCount;
        init_info.ImageCount = wd->ImageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = g_Allocator;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

        VkResult err;
        // Upload Fonts
        {
            // Use any command queue
            VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
            VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

            err = vkResetCommandPool(g_Device, command_pool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(err);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            err = vkEndCommandBuffer(command_buffer);
            check_vk_result(err);
            err = vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(err);

            err = vkDeviceWaitIdle(g_Device);
            check_vk_result(err);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
#endif // TEMPORARY_CODE

        //vkEngine.init_imgui();
    }

    void VulkanContext::OnImGuiBegin()
    {
        ImGui_ImplVulkan_NewFrame();
       
    }

    void VulkanContext::OnImGuiEnd()
    {
        // Vulkan will call internally
        vr->DrawGUI();
#ifdef TEMPORARY_CODE
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
        ImDrawData* main_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
        wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
        wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
        wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
        wd->ClearValue.color.float32[3] = clear_color.w;
        if (!main_is_minimized)
            FrameRender(wd, main_draw_data);
#endif // TEMPORARY_CODE

        ImGui::EndFrame();
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

#ifdef TEMPORARY_CODE
        // Present Main Platform Window
        if (!main_is_minimized)
        {
            vr->Present();
            //FramePresent(wd);
        }
#endif // TEMPORARY_CODE
    }

    void VulkanContext::OnImGuiShutdown()
    {
        vr->DestroyImGUI();
        //vkDeviceWaitIdle(vkEngine._device);
#ifdef TEMPORARY_CODE
        ImGui_ImplVulkan_Shutdown();
#endif // TEMPORARY_CODE
    }

    bool VulkanContext::SetVSync(bool enable)
    {
        UNREFERENCED(enable);
        // vulkan does not currently support vsync yet
        return false;
    }

    VulkanRenderer* VulkanContext::getRenderer()
    {
        return VulkanRenderer::get();
    }

    void VulkanContext::SetWindowResized()
    {
        //vkEngine.SetWindowResized();
    }

}