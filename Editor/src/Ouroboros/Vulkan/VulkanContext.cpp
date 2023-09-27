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


#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Core/Events/ApplicationEvent.h"
#include "Ouroboros/Core/WindowsWindow.h"
#include <App/Editor/Utility/ImGuiManager.h>

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>
namespace oo
{
    extern std::barrier<std::_No_completion_function>* g_frameBarrier;
    extern std::mutex* g_ImGuiMutex;

    VulkanRenderer* VulkanContext::vr{nullptr};
    Window VulkanContext::m_window;

    VulkanContext::VulkanContext(SDL_Window* window)
        : m_windowHandle(window)
    {
        ASSERT_MSG(m_windowHandle == nullptr, "Window handle is null!");

        //vkEngine.SetWindow(window);
    }

    VulkanContext::~VulkanContext()
    {
        //vkEngine.cleanup();
        // set boolean to false and join thread
        m_renderThreadRunning = false;
        m_renderThread.join();
        delete vr;
    }
#pragma optimize("",off)
    void VulkanContext::Init()
    {

        EventManager::Subscribe<VulkanContext, WindowResizeEvent>(this, &VulkanContext::OnWindowResize);
        EventManager::Subscribe<VulkanContext, WindowMinimizeEvent>(this, &VulkanContext::OnWindowMinimize);
        EventManager::Subscribe<VulkanContext, WindowRestoredEvent>(this, &VulkanContext::OnWindowRestored);

        
        // Setup Vulkan
        uint32_t extensions_count = 0;
        SDL_Vulkan_GetInstanceExtensions(m_windowHandle, &extensions_count, NULL);
        std::vector<const char*> extensions;
        extensions.resize(extensions_count);
        SDL_Vulkan_GetInstanceExtensions(m_windowHandle, &extensions_count, &extensions[0]);

        vr = VulkanRenderer::get();

        oGFX::SetupInfo si;
#ifdef OO_END_PRODUCT
        si.debug = false;
#else
        si.debug = true;
#endif
        si.renderDoc = false;
        si.SurfaceFunctionPointer = std::function<bool()>([&]() {
            return SDL_Vulkan_CreateSurface(m_windowHandle, vr->m_instance.instance, &vr->m_instance.surface);
            });
        si.extensions = extensions;
        //m_window.m_width = w;
        //m_window.m_height = h;
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

        OnImGuiEnd();
        SwapBuffers();

        // start render thread here
        const auto renderWorker = [
            vr=vr,
            &mut = *g_ImGuiMutex,
            &barrier = *g_frameBarrier,
            &keepRendering = m_renderThreadRunning,
            &minimized = m_minimized
        ]() {

            while (keepRendering) 
            {
                // Needs 3 parameters : Barrier, mutex, boolean & graphics engine
                if (vr->PrepareFrame() == true)
                {
                    TRACY_PROFILE_SCOPE_N(Vulkan_RenderFrame);
                    // Render the frame
                    vr->RenderFrame();
                    TRACY_PROFILE_SCOPE_END();

                    if (!minimized)
                    {
                        //TODO: global imgui mutex here
                        TRACY_PROFILE_SCOPE_N(Vulkan_DrawGUI);
                        //mut.lock();
                        //LOG_CORE_INFO("render thread locked mutex\n");
                        //std::cout << "render thread locked mutex\n";
                        //// Renderer lock mutex
                        //vr->DrawGUI();
                        //// Renderer release mutex
                        //mut.unlock();
                        //LOG_CORE_INFO("render thread unlocked mutex\n");
                        //std::cout << "render thread unlocked mutex\n";
                        TRACY_PROFILE_SCOPE_END();

                        vr->Present();
                    }

                } // if prepare frame is true

                // TODO: put barrier here
                barrier.arrive_and_wait();
            }
        };
        m_renderThread = std::thread{renderWorker};
    }
#pragma optimize("",on)

    void VulkanContext::OnUpdateBegin()
    {
    }

    void VulkanContext::SwapBuffers()
    {
        if (!m_minimized)
            vr->Present();
    }

    void VulkanContext::InitImGui()
    {
        ImGui_ImplSDL2_InitForVulkan(m_windowHandle);
        vr->InitImGUI();
        ImGuiManager::InitAssetsAll();
    }

    void VulkanContext::ResetImguiInit()
    {
        ImGui_ImplSDL2_InitForVulkan(m_windowHandle);
        vr->RestartImgui();
    }

    void VulkanContext::OnImGuiBegin()
    {
        // TODO: Start Mutex here
        g_ImGuiMutex->lock(); 
        LOG_CORE_INFO("cpu thread locked mutex\n");
        std::cout << "cpu thread locked mutex\n";
        ImGui_ImplVulkan_NewFrame();
    }

    // TODO: this will be migrated to the dedicated renderthread
    void VulkanContext::OnImGuiEnd()
    {
        TRACY_PROFILE_SCOPE_N(Vulkan_Render);
        // temporarily shift here for better structuring
        //m_runtimeCC.Update(oo::timer::dt());

        // Mutex could be released here
        g_ImGuiMutex->unlock(); 
        LOG_CORE_INFO("cpu thread unlocked mutex\n");
        std::cout << "cpu thread unlocked mutex\n";

        TRACY_PROFILE_SCOPE_END();
    }

    void VulkanContext::ResetImguiShutdown()
    {
        vr->ImguiSoftDestroy();
    }


    void VulkanContext::OnImGuiShutdown()
    {
        vr->DestroyImGUI();
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

    void VulkanContext::OnWindowResize(WindowResizeEvent* e)
    {
        m_window.m_height = e->GetHeight();
        m_window.m_width = e->GetWidth();
    }

    void VulkanContext::OnWindowMinimize(WindowMinimizeEvent*)
    {
        m_minimized = true;
        m_window.m_height = 0;
        m_window.m_width = 0;
    }

    void VulkanContext::OnWindowRestored(WindowRestoredEvent*)
    {
        m_minimized = false;
        int w, h;
        SDL_Vulkan_GetDrawableSize(m_windowHandle, &w, &h);
        m_window.m_width = w;
        m_window.m_height = h;
    }

    void VulkanContext::SetWindowResized()
    {
        //vkEngine.SetWindowResized();
    }

}