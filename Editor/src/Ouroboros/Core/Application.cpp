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

#include "Ouroboros/Core/GraphicsContext.h"
//#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

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
        m_renderer = static_cast<GraphicsContext*>(m_window->GetRenderingContext());

        /*Initialize Input Management*/
        input::Init();
    }

    Application::~Application()
    {
        /*Shutdown Input Management*/
        input::ShutDown();

        //m_window->~Window();
    }

    void Application::Run()
    {
        //constexpr const char* const application_run_name = "application run";
        //constexpr const char* const update_loop_name = "update_loop";
        //constexpr const char* const update_layerstack_name = "LayerStack OnUpdate";
        //constexpr const char* const imgui_layerstack_name = "LayerStack OnImGuiUpdate";

        while (m_running)
        {
            /*Calculate dt*/
            timer::Timestep dt = {};

            /*Process Inputs here*/
            input::Update();

            /* Process window input events */
            m_window->ProcessEvents();

            //whatever the renderer needs to call at the beggining if each frame e.g. clear color
            m_renderer->OnUpdateBegin();

            // run derived class update here
            OnUpdate();

            // swap buffers at the end of frame
            m_window->SwapBuffers();
        }
    }

    void Application::Close()
    {
        m_running = false;
    }

}