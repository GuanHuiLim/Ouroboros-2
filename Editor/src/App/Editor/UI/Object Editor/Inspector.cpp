#include "pch.h"
#include "Inspector.h"
#include "Hierarchy.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/Events/OpenFileEvent.h"

#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/EventSystem/EventManager.h>
#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/Prefab/PrefabManager.h>


#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Transform/TransformComponent.h>
#include <Ouroboros/Prefab/PrefabComponent.h>

#include <glm/gtc/type_ptr.hpp>
Inspector::Inspector()
{
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
	m_InspectorUI[UI_RTTRType::UItypes::VEC2_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<glm::vec2>();
		edited = ImGui::DragFloat2(name.c_str(), glm::value_ptr(value));
		if (edited) v = value;
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC3_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<glm::vec3>();
		edited = ImGui::DragFloat3(name.c_str(), glm::value_ptr(value));
		if (edited) v = value;
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC4_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<glm::vec4>();
		edited = ImGui::DragFloat4(name.c_str(), glm::value_ptr(value));
		if (edited) v = value;
	};
	m_InspectorUI[UI_RTTRType::UItypes::UUID_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<UUID>();
		ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
		ImGui::InputScalarN(name.c_str(),ImGuiDataType_::ImGuiDataType_U64, &value,1);//read only
		ImGui::PopItemFlag();
	};
	m_InspectorUI[UI_RTTRType::UItypes::MAT3_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<glm::mat3>();
		edited |= ImGui::DragFloat3(name.c_str(), glm::value_ptr(value[0]));
		edited |= ImGui::DragFloat3("##2", glm::value_ptr(value[1]));
		edited |= ImGui::DragFloat3("##3", glm::value_ptr(value[2]));
		if (edited) v = value;
	};
	m_InspectorUI[UI_RTTRType::UItypes::MAT4_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<glm::mat4>();
		edited |= ImGui::DragFloat4(name.c_str(), glm::value_ptr(value[0]));
		edited |= ImGui::DragFloat4("##2", glm::value_ptr(value[1]));
		edited |= ImGui::DragFloat4("##3", glm::value_ptr(value[2]));
		edited |= ImGui::DragFloat4("##4", glm::value_ptr(value[3]));
		if (edited) v = value;
	};
	m_InspectorUI[UI_RTTRType::UItypes::QUAT_TYPE] = [](std::string& name, rttr::variant& v, bool& edited)
	{
		auto value = v.get_value<quaternion>().value;
		edited = ImGui::DragFloat4(name.c_str(), glm::value_ptr(value));
		if (edited) v = value;
	};
}

void Inspector::Show()
{
	if(ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Show ReadOnly", nullptr, m_showReadonly))
			m_showReadonly = !m_showReadonly;
		ImGui::EndMenuBar();
	}
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
		if (gameobject->HasComponent<oo::PrefabComponent>())
		{
			ImGui::SameLine();
			if (ImGui::Button("Update Prefab"))
			{
				OpenFileEvent ofe(gameobject->GetComponent<oo::PrefabComponent>().prefab_filePath);
				oo::EventManager::Broadcast(&ofe);
			}
		}
		else
		{
			ImGui::SameLine();
			if (ImGui::Button("Create Prefab"))
			{
				oo::PrefabManager::MakePrefab(gameobject);
			}
		}
		//gameobject->GetComponent<>();
		DisplayComponent<oo::GameObjectComponent>(*gameobject);
		DisplayComponent<oo::Transform3D>(*gameobject);
	}
}
