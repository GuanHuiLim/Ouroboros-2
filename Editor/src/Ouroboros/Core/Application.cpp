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

    Application::Application(const std::string& name, CommandLineArgs args)
        : m_commandLineArgs{ args }
        , m_running{ true }
    {
        ASSERT_MSG(s_instance == this, "Application already exist!");
        s_instance = this;
        m_window = std::make_unique<WindowsWindow>(WindowProperties{ name });
        //Retrieve renderer from window
        //m_renderer = static_cast<GraphicsContext*>(m_window->GetRenderingContext());
        m_renderer = m_window->GetVulkanContext();

        /*Initialize Input Management*/
        input::Init();

        // Initialise audio
        audio::Init();

        // Only start running file watchers ones dependencies are intiialised
        /*AssetManager::GlobalStartRunning();*/
    }

    Application::~Application()
    {
        /*Shutdown Input Management*/
        input::ShutDown();

        // Shutdown audio
        audio::ShutDown();

        //m_window->~Window();
    }

    void Application::Run()
    {
        constexpr const char* const update_loop_name = "core app update loop";
        std::chrono::file_clock::time_point fileWatchTime = std::chrono::file_clock::now();
        while (m_running)
        {
            OO_TracyProfiler::CheckIfServerToBeOpened();
            OO_TracyProfiler::CheckIfServerToBeClosed();

            /*Calculate dt*/
            timer::Timestep dt = {};

            TRACY_PROFILE_FRAME_START(update_loop_name);


            TRACY_PROFILE_SCOPE_NC(FILE_WATCH, tracy::Color::Aquamarine1);

            std::chrono::file_clock::time_point now = std::chrono::file_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - fileWatchTime).count() >= AssetManager::WATCH_INTERVAL)
            {
                FileWatchEvent fwe{ fileWatchTime };
                EventManager::Broadcast<FileWatchEvent>(&fwe);
                fileWatchTime = now;
            }

            TRACY_PROFILE_SCOPE_END();

            /*Process Inputs here*/
            input::Update();

            // Update audio
            audio::Update();

            /* Process window input events */
            m_window->ProcessEvents();

            //whatever the renderer needs to call at the beggining if each frame e.g. clear color
            m_renderer->OnUpdateBegin();

            // run derived class update here
            OnUpdate();

            // swap buffers at the end of frame
            m_window->SwapBuffers();
            
            TRACY_PROFILE_END_OF_FRAME();
        }
    }

    void Application::Close()
    {
        m_running = false;
    }

}