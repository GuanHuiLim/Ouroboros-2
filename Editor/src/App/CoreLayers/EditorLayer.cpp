#include "pch.h"
#include "EditorLayer.h"

#include <Ouroboros/Core/Input.h>

// Project Tracker related includes
#include <Launcher/Utilities/ImGuiManager_Launcher.h>
#include <Ouroboros/EventSystem/EventManager.h>

void EditorLayer::OnAttach()
{
    ImGuiManager_Launcher::Create("project tracker", true, ImGuiWindowFlags_None, [this]() { this->m_tracker.Show(); });
}

// TODO : IMGUI DOESNT WORK YET FOR NOW. VULKAN NEEDS TO BE SET UP
// PROPERLY FOR IMGUI RENDERING TO TAKE PLACE

void EditorLayer::OnUpdate()
{
    if (oo::input::IsKeyPressed(KEY_F5))
    {
        m_showDebugInfo = !m_showDebugInfo;
    }

    ImGuiManager_Launcher::UpdateAllUI();

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

    if (m_demo)
        ImGui::ShowDemoWindow(&m_demo);

    if (ImGui::Button("restart Imgui"))
    {
        ImGuiRestartEvent restartEvent;
        oo::EventManager::Broadcast(&restartEvent);
    }

}
