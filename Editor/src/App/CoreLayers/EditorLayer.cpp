/************************************************************************************//*!
\file           EditorLayer.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 31, 2022
\brief          Defines a layer that will be running during editor mode
                and its related events

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "EditorLayer.h"

#include <Ouroboros/Core/Input.h>

// Project Tracker related includes
#include <Launcher/Utilities/ImGuiManager_Launcher.h>
#include <Ouroboros/EventSystem/EventManager.h>
#include <App/Editor/Events/LoadProjectEvents.h>

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

    if(m_editormode == false)
        ImGuiManager_Launcher::UpdateAllUI();
    else
	    m_editor.Update();

	//top menu bar
	//Editor::MenuBar();
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::MenuItem("Open Launcher")) 
			{
				m_editormode = false;
				CloseProjectEvent e;
				oo::EventManager::Broadcast(&e);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
    //m_editor.ShowAllWidgets();

    //if (m_demo)
    //    ImGui::ShowDemoWindow(&m_demo);


}
