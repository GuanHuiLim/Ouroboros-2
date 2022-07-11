/************************************************************************************//*!
\file           EditorApp.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Customer side of the project that utilizes the functions of the Engine.
                An Editor will be a use case for game engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

// specially include this file only at the entry point of the engine.
#include <EntryPoint.h>

#include <Ouroboros/Core/Input.h>
#include <Ouroboros/Core/Timer.h>
#include <Ouroboros/Core/LayerSet.h>
#include <Ouroboros/ImGui/ImGuiAbstraction.h>
#include <Ouroboros/EventSystem/EventManager.h>

// Debug Layers
#include "TestLayers/InputDebugLayer.h"
#include "TestLayers/MainDebugLayer.h"

// Core Essential Layers
#include "CoreLayers/SceneLayer.h"

// External includes
#include <imgui.h>

// Project Tracker related includes
#include <Launcher/Launcher/ProjectTracker.h>
#include <Launcher/Utilities/ImGuiManager.h>

//Shared Library related includes
#include <SceneManager.h>
#include <Transform.h>
#include <Quaternion.h>
#include <Scenegraph.h>

#include "Ouroboros/EventSystem/Event.h"

struct ImGuiRestartEvent : public oo::Event
{
};

class EditorLayer final : public oo::Layer
{
private:
    //order matters dont change it
    bool m_demo = true;
    bool m_showDebugInfo = false;
    
    ProjectTracker m_tracker;
public:
    EditorLayer()
        : oo::Layer{ "EditorLayer" }
    {
        LOG_INFO("Test Info");
        LOG_TRACE("Test Trace");
        LOG_WARN("Test Warn");
        LOG_ERROR("Test Error");
        LOG_CRITICAL("Test Critical");
    }

    virtual void OnAttach() override final
    {
        ImGuiManager::Create("project tracker", true, ImGuiWindowFlags_None, [this]() { this->m_tracker.Show(); });
    }

    // TODO : IMGUI DOESNT WORK YET FOR NOW. VULKAN NEEDS TO BE SET UP
    // PROPERLY FOR IMGUI RENDERING TO TAKE PLACE
    virtual void OnUpdate() override final
    {
        if (oo::input::IsKeyPressed(KEY_F5))
        {
            m_showDebugInfo = !m_showDebugInfo;
        }
        
        ImGuiManager::UpdateAllUI();
        
         //#if EDITOR_DEBUG || EDITOR_RELEASE
         /*if (m_showDebugInfo)
         {
             oo::TimeDebugInfo timeDebugInfo = oo::Timestep::GetDebugTimeInfo();

             ImGui::Begin("fpsviewer", nullptr,
                 ImGuiWindowFlags_NoScrollbar
                 | ImGuiWindowFlags_NoCollapse
                 | ImGuiWindowFlags_NoTitleBar
                 | ImGuiWindowFlags_AlwaysAutoResize);
             ImGui::Text("Rolling FPS %.2f", timeDebugInfo.AvgFPS);
             ImGui::Text("Rolling DeltaTime %.6f", timeDebugInfo.AvgDeltaTime);
             ImGui::Text("Current Timescale %.2f", timeDebugInfo.CurrentTimeScale);
             ImGui::Text("Time elpased %.2f", timeDebugInfo.TimeElapsed);
             ImGui::End();
         }*/
         //#endif

        //m_editor.ShowAllWidgets();
        
        if(m_demo)
            ImGui::ShowDemoWindow(&m_demo);

        if (ImGui::Button("restart Imgui"))
        {
            ImGuiRestartEvent restartEvent;
            oo::EventManager::Broadcast(&restartEvent);
        }
        
    }

};

class EditorApp final : public oo::Application
{
public:
    EditorApp(oo::CommandLineArgs args)
        : Application{ "Ouroboros v2.0", args }
    {
        //m_layerset.PushLayer(std::make_shared<InputDebugLayer>());
        m_layerset.PushLayer(std::make_shared<MainDebugLayer>());
        
        m_layerset.PushLayer(std::make_shared<EditorLayer>());

        m_imGuiAbstract = std::make_unique<oo::ImGuiAbstraction>();
        
        oo::EventManager::Subscribe<EditorApp, ImGuiRestartEvent>(this, &EditorApp::RestartImGui);
        
        m_layerset.PushLayer(std::make_shared<oo::SceneLayer>());

    }

    void OnUpdate() override
    {
        m_imGuiAbstract->Begin();

        m_layerset.Update();

        m_imGuiAbstract->End();

        if (oo::input::IsKeyPressed(KEY_ESCAPE))
        {
            Close();
        }
    }

    void RestartImGui(ImGuiRestartEvent*)
    {
        m_imGuiAbstract->Restart();
    }

private:
    oo::LayerSet m_layerset;
    std::unique_ptr<oo::ImGuiAbstraction> m_imGuiAbstract;
};

oo::Application* oo::CreateApplication(oo::CommandLineArgs args)
{
    return new EditorApp{ args };
}
