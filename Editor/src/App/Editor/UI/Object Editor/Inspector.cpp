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
#include <Ouroboros/ECS/DeferredComponent.h>
#include <Ouroboros/Transform/TransformComponent.h>
#include <Ouroboros/Prefab/PrefabComponent.h>

#include <glm/gtc/type_ptr.hpp>
Inspector::Inspector()
{
	m_InspectorUI[UI_RTTRType::UItypes::BOOL_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		bool value = v.get_value<bool>();
		edited = ImGui::Checkbox(name.c_str(),&value);
		if (edited) { v = value; endEdit = true; };
	};
	m_InspectorUI[UI_RTTRType::UItypes::STRING_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.to_string();
		edited = ImGui::InputText(name.c_str(), &value);
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC2_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::vec2>();
		edited = ImGui::DragFloat2(name.c_str(), glm::value_ptr(value));
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC3_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::vec3>();
		edited = ImGui::DragFloat3(name.c_str(), glm::value_ptr(value));
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::VEC4_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::vec4>();
		edited = ImGui::DragFloat4(name.c_str(), glm::value_ptr(value));
		if (edited) { v = value; };
		endEdit = ImGui::IsItemDeactivatedAfterEdit();
	};
	m_InspectorUI[UI_RTTRType::UItypes::UUID_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<UUID>();
		ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
		ImGui::InputScalarN(name.c_str(),ImGuiDataType_::ImGuiDataType_U64, &value,1);//read only
		ImGui::PopItemFlag();
	};
	m_InspectorUI[UI_RTTRType::UItypes::MAT3_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::mat3>();
		edited |= ImGui::DragFloat3(name.c_str(), glm::value_ptr(value[0]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat3("##2", glm::value_ptr(value[1]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat3("##3", glm::value_ptr(value[2]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		if (edited) { v = value; };
	};
	m_InspectorUI[UI_RTTRType::UItypes::MAT4_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<glm::mat4>();
		edited |= ImGui::DragFloat4(name.c_str(), glm::value_ptr(value[0]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat4("##2", glm::value_ptr(value[1]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat4("##3", glm::value_ptr(value[2]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		edited |= ImGui::DragFloat4("##4", glm::value_ptr(value[3]));
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
		if (edited) { v = value; };
	};
	m_InspectorUI[UI_RTTRType::UItypes::QUAT_TYPE] = [](std::string& name, rttr::variant& v, bool& edited, bool& endEdit)
	{
		auto value = v.get_value<quaternion>().value;
		edited = ImGui::DragFloat4(name.c_str(), glm::value_ptr(value));
		if (edited) { v = value; };
		endEdit |= ImGui::IsItemDeactivatedAfterEdit();
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
		auto gameobject = scene->FindWithInstanceID(*selected_items.begin());//first item
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
		DisplayComponent<oo::DeferredComponent>(*gameobject);
	}
}
void Inspector::DisplayNestedComponent(std::string name , rttr::type class_type, rttr::variant& value, bool& edited, bool& endEdit)
{

	ImGui::Text(name.c_str());
	ImGui::Dummy({ 5.0f,0 });
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Separator();
	ImGui::PushID(class_type.get_id());

	for (rttr::property prop : class_type.get_properties())
	{
		bool propReadonly = prop.is_readonly();
		if (propReadonly && m_showReadonly == false)
			continue;

		rttr::type prop_type = prop.get_type();

		auto ut = UI_RTTRType::types.find(prop_type.get_id());
		if (ut == UI_RTTRType::types.end())
		{
			if (prop_type.is_sequential_container())//vectors and lists
			{
				rttr::variant v = prop.get_value(value);
				bool set_edited = false;
				bool end_edit = false;
				DisplayArrayView(prop.get_name().data(), prop_type, v, set_edited, end_edit);
				if (end_edit)
					endEdit = true;
				if (set_edited == true)
				{
					edited = true;
					prop.set_value(value, v);//set value to variant
				}
			}
			continue;
		}

		auto iter = m_InspectorUI.find(ut->second);
		if (iter == m_InspectorUI.end())
			continue;

		rttr::variant v = prop.get_value(value);
		bool set_value = false;
		bool end_edit = false; //when the item is let go
		std::string name = prop.get_name().data();

		if (propReadonly)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, ImGui_StylePresets::disabled_color);
			iter->second(name, v, set_value, end_edit);
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		}
		else
		{
			iter->second(name, v, set_value, end_edit);
			if (end_edit)
				endEdit = true;
			if (set_value == true)
			{
				edited = true;
				prop.set_value(value, v);//set value to variant
			}
		}
		ImGui::Dummy({ 0,5.0f });
	}
	ImGui::PopID();
	ImGui::Separator();
	ImGui::EndGroup();

}
void Inspector::DisplayArrayView(std::string name, rttr::type variable_type, rttr::variant& value, bool& edited, bool& endEdit)
{
	rttr::variant_sequential_view sqv =	value.create_sequential_view();
	rttr::type::type_id id = sqv.get_value_type().get_id();

	auto ut = UI_RTTRType::types.find(id);
	if (ut == UI_RTTRType::types.end())
		return;
	auto iter = m_InspectorUI.find(ut->second);
	if (iter == m_InspectorUI.end())
		return;
	ImGui::Text(name.c_str());
	ImGui::PushID(id);
	ImGui::Dummy({ 5.0f,0 });
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Separator();
	
	size_t size = sqv.get_size();
	constexpr size_t min_arrSize = 0;
	constexpr size_t max_arrSize = 30;

	//Size Slider

	if (ImGui::DragScalarN("##Size", ImGuiDataType_::ImGuiDataType_U64, &size, 1,0.3f,&min_arrSize,&max_arrSize))
		edited = endEdit = sqv.set_size(size);
	ImGui::SameLine();
	if (ImGui::ArrowButton("reduceSize", ImGuiDir_::ImGuiDir_Left))
		edited = endEdit = sqv.set_size(size - 1);
	ImGui::SameLine();
	if (ImGui::ArrowButton("increaseSize", ImGuiDir_::ImGuiDir_Right))
		edited = endEdit = sqv.set_size(size + 1);
	ImGui::SameLine();
	ImGui::Text("Size");

	bool itemEdited = false;
	bool itemEndEdit = false;
	std::string tempstring = "##empty";
	for (size_t i = 0; i < sqv.get_size(); ++i)
	{
		ImGui::PushID(i);
		rttr::variant v = sqv.get_value(i);
		std::string temp = v.get_type().get_name().data();
		std::string tmep2 = rttr::type::get<bool>().get_name().data();
		iter->second(tempstring, v, itemEdited, itemEndEdit);
		if (itemEdited)
		{
			edited = true;
			sqv.set_value(i, v);
		}
		endEdit |= itemEndEdit;
		ImGui::PopID();
	}
	ImGui::EndGroup();
	ImGui::PopID();
	ImGui::Separator();
}