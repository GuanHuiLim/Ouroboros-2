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

#include <Ouroboros/Core/Application.h>
#include <Ouroboros/Vulkan/VulkanContext.h>

#include "Ouroboros/Audio/Audio.h"

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>


void EditorLayer::OnAttach()
{
    ImGuiManager_Launcher::Create("project tracker", true, ImGuiWindowFlags_None, [this]() { this->m_tracker.Show(); });
	
}

void EditorLayer::OnUpdate()
{
	//steam
	
	TRACY_PROFILE_SCOPE_NC(editor_ui_update, tracy::Color::Blue);

    if(m_editormode == false)
        ImGuiManager_Launcher::UpdateAllUI();
#ifdef OO_EDITOR
    else
	    m_editor.Update();
#endif

	//top menu bar
	//Editor::MenuBar();
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			if (ImGui::MenuItem("Open Launcher")) 
			{
#ifdef OO_EDITOR
				CloseProjectEvent e;
				OpenPromptEvent<CloseProjectEvent> ope(e, [this]() {this->SetEditorMode(false); });
				oo::EventManager::Broadcast(&ope);
#else
				m_editormode = false;
#endif
			}
			// TODO : Remove or wrap in something else!
#ifdef OO_EXECUTABLE
			if (ImGui::MenuItem(oo::OO_TracyProfiler::m_server_active ? ("Close Profiler") : ("Open Profiler")))
			{
				if (oo::OO_TracyProfiler::m_server_active)
					oo::OO_TracyProfiler::CloseTracyServer();
				else
					oo::OO_TracyProfiler::StartTracyServer();
			}
#endif

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
#ifdef OO_EDITOR
	m_editor.AddSequence(Editor::TimedSequence([this] {
			m_discord_helper.Update();
		}, 2.0f));
#endif
	TRACY_PROFILE_SCOPE_END();
}

void EditorLayer::OnDetach()
{
	
}
