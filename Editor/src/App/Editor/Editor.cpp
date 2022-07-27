#include "pch.h"
#include "Editor.h"
#include "Utility/ImGuiManager.h"

Editor::Editor()
{
	ImGuiManager::Create("Style Editor", true, ImGuiWindowFlags_MenuBar, [this] {this->m_styleEditor.Show(); });
	ImGuiManager::Create("Hierarchy", true, ImGuiWindowFlags_MenuBar, [this] {this->m_hierarchy.Show(); });

}

Editor::~Editor()
{
}

void Editor::Update()
{
	ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGuiManager::UpdateAllUI();
	m_warningMessage.Show();
}
