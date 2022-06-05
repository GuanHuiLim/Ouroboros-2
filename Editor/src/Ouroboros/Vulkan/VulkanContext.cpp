/************************************************************************************//*!
\file           VulkanContext.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 17, 2021
\brief          Describes the Implementation of the Vulkan Backend Graphics Context
                and capabilities.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Ouroboros/Vulkan/VulkanContext.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_vulkan.h"

namespace oo
{
    //VulkanEngine VulkanContext::vkEngine;

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
    }

    void VulkanContext::Init()
    {
        //vkEngine.init();

        //OpenGL + SDL code
        //m_glContext = SDL_GL_CreateContext(m_windowHandle);
        //SDL_GL_MakeCurrent(m_windowHandle, m_glContext);

        //bool status = (gl3wInit() == 0);
        //ASSERT_MSG(status, "Failed to initialize OpenGL loader!(gl3w)\n");

        //LOG_ENGINE_INFO("OpenGL Info:");
        //LOG_ENGINE_INFO("  Vendor   : {0}", glGetString(GL_VENDOR));
        //LOG_ENGINE_INFO("  Renderer : {0}", glGetString(GL_RENDERER));
        //LOG_ENGINE_INFO("  Version  : {0}", glGetString(GL_VERSION));
    }

    void VulkanContext::OnUpdateBegin()
    {

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
    }

    void VulkanContext::InitImGui()
    {
        ImGui_ImplSDL2_InitForVulkan(m_windowHandle);
        //vkEngine.init_imgui();
    }

    void VulkanContext::OnImGuiBegin()
    {
        ImGui_ImplVulkan_NewFrame();
    }

    void VulkanContext::OnImGuiEnd()
    {
        // Vulkan will call internally

        ImGuiIO& io = ImGui::GetIO();
        /*if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }*/
    }

    void VulkanContext::OnImGuiShutdown()
    {
        //vkDeviceWaitIdle(vkEngine._device);
        ImGui_ImplVulkan_Shutdown();
    }

    bool VulkanContext::SetVSync(bool enable)
    {
        // vulkan does not currently support vsync yet
        return false;
    }

    void VulkanContext::SetWindowResized()
    {
        //vkEngine.SetWindowResized();
    }

}