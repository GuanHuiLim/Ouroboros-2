#include "pch.h"
#include "Inspector.h"
#include "Hierarchy.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "App/Editor/Utility/ImGuiManager.h"

#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/ECS/GameObject.h>
Inspector::Inspector()
{
}

void Inspector::Show()
{
	auto& selected_items = Hierarchy::GetSelected();
	size_t size = selected_items.size();
	if (size == 0)
		return;
	if (size > 1)
	{

		return;
	}

	{
		auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
		auto gameobject = scene->FindWithInstanceID(selected_items[0]);//first item
		bool active = gameobject->ActiveInHierarchy();
		
		ImGui::InputText("Name:",&gameobject->Name(),ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if (ImGui::Checkbox("Active", &active))
			gameobject->SetActive(active);
		//gameobject->GetComponent<>();
	}
}
