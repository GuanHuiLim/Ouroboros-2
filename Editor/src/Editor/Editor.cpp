#include "pch.h"
#include "Editor.h"
#include "Editor/Utility/ImGuiManager.h"

Editor::Editor()
{
	ImGuiManager::Create("Style Editor", true, ImGuiWindowFlags_MenuBar, [this] {this->m_styleEditor.Show(); });
	m_warningMessage.Show();
}

Editor::~Editor()
{
}

void Editor::Update()
{
	ImGui::DockSpaceOverViewport(ImGui::GetWindowViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGuiManager::UpdateAllUI();
}
