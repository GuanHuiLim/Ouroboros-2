/************************************************************************************//*!
\file           ImGuiAbstraction.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 25, 2022
\brief          Implements a basic ImGuiAbstraction that sets up for the core ImGui Features
                Required into the main application.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Ouroboros/ImGui/ImGuiAbstraction.h"
//#include "Ouroboros/Core/GraphicsContext.h"
#include "Ouroboros/Vulkan/VulkanContext.h"
#include "Ouroboros/Core/Application.h"


#include <sdl2/SDL.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl.h>

#include "Ouroboros/EventSystem/EventManager.h"

namespace oo
{
    ImGuiAbstraction::ImGuiAbstraction()
        : m_restart { false }
#ifdef OO_PLATFORM_WINDOWS
        , m_window { static_cast<SDL_Window*>(Application::Get().GetWindow().GetNativeWindow())  }
#endif
        , m_renderer{ Application::Get().GetWindow().GetVulkanContext() }
    {
        Init();
    }

    ImGuiAbstraction::~ImGuiAbstraction()
    {
        Destroy();
    }

    void ImGuiAbstraction::Init()
    {
        

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
        
                                                                    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
        
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // dont do shit to the mouse curosr

        //io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
        //io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        
        //SetDarkThemeColors();

        // Setup Platform/Renderer bindings
        if (m_restart)
        {
            m_renderer->ResetImguiInit();
        }
        else
        {
            m_renderer->InitImGui();
        }

    }

    void ImGuiAbstraction::Destroy()
    {
        if (m_restart)
        {
            m_renderer->ResetImguiShutdown();
        }
        else
        {
            m_renderer->OnImGuiShutdown();
        }
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }


    void ImGuiAbstraction::Begin()
    {
        DetermineRestart();

        m_renderer->OnImGuiBegin();

#ifdef OO_PLATFORM_WINDOWS
        ImGui_ImplSDL2_NewFrame(m_window);
#endif
        
        ImGui::NewFrame();

    }

    void ImGuiAbstraction::End()
    {
        ////figure out what the below 3 lines do?
        /*ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());*/

        // Rendering
        ImGui::Render();
        
        m_renderer->OnImGuiEnd();
    }

    void ImGuiAbstraction::DetermineRestart()
    {
        // if restarting is required
        if (m_restart)
        {
            LOG_CORE_WARN("Restarting ImGui");

            Destroy();

            ReplaceImGuiSettings();

            Init();

            m_restart = false;
        }
    }

    void ImGuiAbstraction::ReplaceImGuiSettings()
    {
        //char exePath[1000];
        //GetModuleFileNameA(NULL, exePath, 1000);
        // std::filesystem::path p = exePath; hard-path
        std::filesystem::path p = "./"; // current-path but cannot debug.
        std::string target_file = p.parent_path().string() + "/default.ini";
        std::string overwritten_file = p.parent_path().string() + "/imgui.ini";
        std::filesystem::copy_file(target_file, overwritten_file, std::filesystem::copy_options::overwrite_existing);
    }

}