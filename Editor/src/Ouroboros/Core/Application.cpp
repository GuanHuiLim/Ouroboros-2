/************************************************************************************//*!
\file           Application.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2021
\brief          Core Application Loop and functionality. 
                Will be inherited by Sandbox project.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Application.h"

//#include "Ouroboros/Core/Input.h"
//
//#include "Ouroboros/Renderer/GraphicsContext.h"
//#include "Ouroboros/Renderer/Renderer2D.h"
//#include "Ouroboros/Renderer/ParticleRenderer.h"
//#include "Ouroboros/Renderer/FontRenderer.h"
//#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

#include "Timer.h"

namespace oo
{
    Application* Application::s_instance = nullptr;

    Application::Application(const std::string& name, CommandLineArgs args)
        : m_commandLineArgs{ args }
        , m_running{ true }
    {

        /*ENGINE_ASSERT_MSG(!s_instance, "Application already exist!");*/
        s_instance = this;
        /*m_window = Window::Create(WindowProperties{ name });*/

        ////Binds window callback to call Application::OnEvent
        //m_window->SetEventCallback(ENGINE_BIND_EVENT_FN(Application::OnEvent));
        //Retrieve renderer from window

        //m_renderer = static_cast<GraphicsContext*>(m_window->GetRenderingContext());

        /*Initialize Input Management*/
        //Input::Init();

        //Timestep::Init();
    }

    Application::~Application()
    {
        /*Shutdown Input Management*/
        //Input::ShutDown();

        //m_window->~Window();
    }

    void Application::Run()
    {
        constexpr const char* const application_run_name = "application run";
        constexpr const char* const update_loop_name = "update_loop";
        constexpr const char* const update_layerstack_name = "LayerStack OnUpdate";
        constexpr const char* const imgui_layerstack_name = "LayerStack OnImGuiUpdate";

        while (m_running)
        {
            /*Calculate dt*/
            //Timestep dt{ m_window->CalcDeltaTime() };
            
            timer::Timestep dt = {};

            /*Process Inputs here*/
            //Input::Update();

            /* Process window input events */
            //m_window->ProcessEvents();

            //whatever the renderer needs to call at the beggining if each frame e.g. clear color
            //m_renderer->OnUpdateBegin();

            // Layerstack update : layers gets drawn first followed by overlays
            // starting with the standard layers
            /*{
                for (LayerStack::value_type layer : m_layerStack)
                {
                    layer->OnUpdate(dt);
                }
            }

            m_window->SwapBuffers();*/
        }
    }

    void Application::Close()
    {
        m_running = false;
    }

}