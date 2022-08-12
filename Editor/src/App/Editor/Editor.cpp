#include "pch.h"
#include "Editor.h"
#include "Utility/ImGuiManager.h"
#include "Serializer.h"
Editor::Editor()
{
	UI_RTTRType::Init();
	Serializer::Init();//runs the init function

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
	ImGuiManager::UpdateAllUI();
	m_warningMessage.Show();

	if (ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_::ImGuiKey_S))
	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		std::string scene_name = scene->GetFilePath();
		Serializer::SaveScene(*(scene));
	}
}
