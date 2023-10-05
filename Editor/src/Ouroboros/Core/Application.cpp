/************************************************************************************//*!
\file           Application.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2022
\brief          Core Application Loop and functionality.
                Will be inherited by Sandbox project.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Application.h"

#include "Ouroboros/Core/Input.h"

//#include "Ouroboros/Core/GraphicsContext.h"
#include "Ouroboros/Vulkan/VulkanContext.h"

#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

#include "Ouroboros/Audio/Audio.h"

#include "Ouroboros/Asset/AssetManager.h"

#include "Ouroboros/EventSystem/EventManager.h"

#include "Timer.h"

namespace oo
{
    Application* Application::s_instance = nullptr;

    std::mutex ImGuiMutex{};
    std::barrier frameBarrier{ 2 };
    std::barrier<std::_No_completion_function>* g_frameBarrier = &frameBarrier;

    Application::Application(const std::string& name, CommandLineArgs args)
        : m_commandLineArgs{ args }
        , m_running{ true }
    {
        ASSERT_MSG(s_instance == this, "Application already exist!");
        s_instance = this;
        m_window = std::make_unique<WindowsWindow>(WindowProperties{ name });
        //Retrieve renderer from window
        m_renderer = m_window->GetVulkanContext();

        /*Initialize Input Management*/
        input::Init();

        // Only start running file watchers ones dependencies are intiialised
        /*AssetManager::GlobalStartRunning();*/

        OPTICK_SET_MEMORY_ALLOCATOR(
            [](size_t size) -> void* { return operator new(size); },
            [](void* p) { operator delete(p); },
            []() { /* Do some TLS initialization here if needed */ }
        );

    }

    Application::~Application()
    {
        /*Shutdown Input Management*/
        input::ShutDown();
    }

    void Application::Run()
    {
        constexpr const char* const update_loop_name = "core app update loop";
        while (m_running)
        {
            OPTICK_FRAME("Application::Run");
            if (m_window->IsMinimized())
            {
                m_window->ProcessWindowEvents();
                Sleep(1);
                continue;
            }

            /* Update Tracy */
            OO_TracyProfiler::CheckIfServerToBeOpened();
            OO_TracyProfiler::CheckIfServerToBeClosed();

            /*Calculate dt*/
            timer::Timestep dt = {};

            /* Start profiling */
            TRACY_PROFILE_FRAME_START(update_loop_name);
            OPTICK_CATEGORY(update_loop_name, Optick::Category::Application);
            {
                /*Process Inputs here*/
                TRACY_PROFILE_SCOPE_N(input_update);
                OPTICK_CATEGORY(input_update, Optick::Category::Input);
                input::Update();
                TRACY_PROFILE_SCOPE_END();
            }

            {
                // Update audio
                TRACY_PROFILE_SCOPE_N(audio_update);
                OPTICK_CATEGORY(audio_update, Optick::Category::Audio);
                audio::Update();
                TRACY_PROFILE_SCOPE_END();
            }

            {
                /* Process window input events */
                TRACY_PROFILE_SCOPE_N(windows_process_events);
                OPTICK_CATEGORY(windows_process_events, Optick::Category::IO);
                m_window->ProcessEvents();
                TRACY_PROFILE_SCOPE_END();
            }

            {
                //whatever the renderer needs to call at the beggining if each frame e.g. clear color
                TRACY_PROFILE_SCOPE_N(renderer_update_begin);
                OPTICK_CATEGORY(renderer_update_begin, Optick::Category::Rendering);
                m_renderer->OnUpdateBegin();
                TRACY_PROFILE_SCOPE_END();
            }

            {
                // run derived class update here
                TRACY_PROFILE_SCOPE_N(derived_on_update);
                OPTICK_CATEGORY(derived_on_update, Optick::Category::Scene);
                OnUpdate();
                TRACY_PROFILE_SCOPE_END();
            }

            // Should wait for dedicated renderer here
            {
                OPTICK_EVENT("Wait Renderer");
                frameBarrier.arrive_and_wait();
            }
            //{
            //    // swap buffers at the end of frame
            //    TRACY_PROFILE_SCOPE_N(windows_swap_buffer);
            //    OPTICK_CATEGORY(windows_swap_buffer, Optick::Category::Rendering);
            //    m_window->SwapBuffers();
            //    TRACY_PROFILE_SCOPE_END();
            //}
            
            TRACY_PROFILE_END_OF_FRAME();
        }

    }

    void Application::Close()
    {
        m_running = false;
        OPTICK_SHUTDOWN();
    }

}
