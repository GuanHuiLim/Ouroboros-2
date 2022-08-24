#include "pch.h"
#include "Editor.h"
#include "Utility/ImGuiManager.h"
#include "Serializer.h"
#include "App/Editor/UI/Tools/WarningMessage.h"
#include <Ouroboros/EventSystem/EventManager.h>
#include "App/Editor/Events/LoadSceneEvent.h"
#include "App/Editor/Events/ImGuiRestartEvent.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "Project.h"
Editor::Editor()
{
	UI_RTTRType::Init();
	Serializer::Init();//runs the init function
	Serializer::InitEvents();
	ImGuiManager::Create("Style Editor", true, ImGuiWindowFlags_MenuBar, [this] {this->m_styleEditor.Show(); });
	ImGuiManager::Create("Hierarchy", true, ImGuiWindowFlags_MenuBar, [this] {this->m_hierarchy.Show(); });
	ImGuiManager::Create("Inspector", true, ImGuiWindowFlags_MenuBar, [this] {this->m_inspector.Show(); });
}

Editor::~Editor()
{
}

void Editor::Update()
{
	ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	//MenuBar();

	ImGuiManager::UpdateAllUI();
	m_warningMessage.Show();
	if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_S))
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		Serializer::SaveScene(*(scene));
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Scene Saved");
	}
	if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_D))
	{
		OpenFileEvent ofe(Project::GetSceneFolder().string() + "Scene1.scn");
		oo::EventManager::Broadcast(&ofe);
	}
}

void Editor::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Reset ImGui"))
			{
				ImGuiRestartEvent restartEvent;
				oo::EventManager::Broadcast(&restartEvent);
			}
			if (ImGui::MenuItem(oo::OO_TracyProfiler::m_server_active ? ("Close Profiler") : ("Open Profiler")))
			{
				if (oo::OO_TracyProfiler::m_server_active)
					oo::OO_TracyProfiler::CloseTracyServer();
				else
					oo::OO_TracyProfiler::StartTracyServer();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
