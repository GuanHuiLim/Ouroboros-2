#include "pch.h"
#include "SceneOrderingWindow.h"
#include <Ouroboros/EventSystem/EventTypes.h>
#include <Ouroboros/EventSystem/EventManager.h>
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/RuntimeController.h"
#include "App/Editor/Utility/ImGuiStylePresets.h"
SceneOrderingWindow::SceneOrderingWindow()
{
}

SceneOrderingWindow::~SceneOrderingWindow()
{
}

void SceneOrderingWindow::Show()
{
	auto* runtimeController = ImGuiManager::s_runtime_controller;

	ImGui::BeginChild("SceneOrderingchild", { 0,0 }, true);

	ImVec2 cursor_pos = ImGui::GetCursorPos();
	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::Selectable("##dragScenesHere", false, ImGuiSelectableFlags_AllowItemOverlap, ImGui::GetContentRegionAvail());
	ImGui::PopItemFlag();
	if (ImGui::BeginDragDropTarget())
	{
		auto* payload = ImGui::AcceptDragDropPayload(".scn");
		if (payload)
		{
			std::filesystem::path p = *static_cast<std::filesystem::path*>(payload->Data);
			runtimeController->AddLoadPath(p.stem().string(), p.string());
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SetCursorPos(cursor_pos);
	
	auto loadpaths = runtimeController->GetLoadPaths();
	int counter = 0;
	for (auto& scenes : loadpaths)
	{
		ImGui::PushID(counter);
		ImGui::BeginGroup();
		ImGui::ImageButton(ImGuiManager::s_editorAssetManager.LoadName("SceneIcon").begin()->GetData<ImTextureID>(), ImGui_StylePresets::image_medium);
		ImGui::EndGroup();
		
		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::Text(scenes.SceneName.c_str());
		ImGui::EndGroup();

		ImGui::SameLine();
		ImGui::BeginGroup();
		bool up_enabled = (counter != 0);
		bool down_enabled = (counter < loadpaths.size() - 1);
	
		if (up_enabled == false)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImGui_StylePresets::disabled_color);
		}
		if (ImGui::ArrowButton("up button", ImGuiDir_::ImGuiDir_Up))
		{
			runtimeController->Swap(counter, counter - 1);
		}
		if (up_enabled == false)
		{
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		}

		if (down_enabled == false)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImGui_StylePresets::disabled_color);
		}
		if (ImGui::ArrowButton("down button", ImGuiDir_::ImGuiDir_Down))
		{
			runtimeController->Swap(counter, counter + 1);
		}
		if (down_enabled == false)
		{
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		}
		ImGui::EndGroup();

		ImGui::PopID();
		++counter;
	}
	ImGui::EndChild();
}
