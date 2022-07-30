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
	UI_RTTRType::Init();
	m_InspectorUI[UI_RTTRType::UItypes::BOOL_TYPE] = [](std::string& name, rttr::variant& v, bool& edited) 
	{
		bool value = v.get_value<bool>();
		edited = ImGui::Checkbox(name.c_str(),&value);
		if (edited) v = value;
	};
	m_InspectorUI[UI_RTTRType::UItypes::STRING_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<std::string>();
		edited = ImGui::InputText(name.c_str(), &value);
		if (edited) v = value;
	};
	m_InspectorUI[UI_RTTRType::UItypes::UUID_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<UUID>();
		ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
		ImGui::InputScalarN(name.c_str(),ImGuiDataType_::ImGuiDataType_U64, &value,1);//read only
		ImGui::PopItemFlag();
	};
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
		if (gameobject == nullptr)
			return;
		bool active = gameobject->ActiveInHierarchy();
		
		ImGui::InputText("Name:",&gameobject->Name(),ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if (ImGui::Checkbox("Active", &active))
			gameobject->SetActive(active);
		//gameobject->GetComponent<>();
		DisplayComponent<oo::GameObjectComponent>(*gameobject);
	}
}
