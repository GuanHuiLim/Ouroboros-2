/************************************************************************************//*!
\file          Inspector.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Edit the Properties of each gameobject and ability to make prefab.
			   Edit Components
			   Add Components
			   Remove Components
			   Make Prefab

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Inspector.h"
#include "Hierarchy.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/imgui_internal.h>

#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/Utility/ImGui_ToggleButton.h"
#include "App/Editor/Events/OpenFileEvent.h"
#include "Utility/UUID.h"

#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/EventSystem/EventManager.h>
#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/Prefab/PrefabManager.h>
#include <Ouroboros/Commands/Script_ActionCommand.h>

#include <Ouroboros/Audio/AudioListenerComponent.h>
#include <Ouroboros/Audio/AudioSourceComponent.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/ECS/DeferredComponent.h>
#include <Ouroboros/ECS/DuplicatedComponent.h>
#include <Ouroboros/Transform/TransformComponent.h>
#include <Ouroboros/Prefab/PrefabComponent.h>

#include <Ouroboros/Physics/RigidbodyComponent.h>
#include <Ouroboros/Physics/ColliderComponents.h>

#include <Ouroboros/Scripting/ScriptComponent.h>
#include <Ouroboros/Scripting/ScriptSystem.h>
#include <Ouroboros/Scripting/ScriptManager.h>
#include <Ouroboros/Vulkan/MeshRendererComponent.h>
#include <Ouroboros/Vulkan/LightComponent.h>
#include <Ouroboros/Vulkan/CameraComponent.h>

#include <glm/gtc/type_ptr.hpp>
#include <Ouroboros/ECS/GameObjectDebugComponent.h>
#include <Ouroboros/ECS/GameObjectDisabledComponent.h>
#include <Ouroboros/Animation/AnimationComponent.h>

Inspector::Inspector()
	:m_AddComponentButton("Add Component", false, {200,50},ImGui_StylePresets::disabled_color,ImGui_StylePresets::prefab_text_color)
{
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
		
		//bool active = gameobject->ActiveInHierarchy();
		bool active = gameobject->IsActive();

		//bool disable_prefabEdit = gameobject->GetIsPrefab() && gameobject->HasComponent<oo::PrefabComponent>() == false;
		//if (disable_prefabEdit)
		//{
		//	ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled,true);
		//	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImGui_StylePresets::disabled_color);
		//	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, ImGui_StylePresets::disabled_color);
		//	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, ImGui_StylePresets::disabled_color);
		//}

		ImGui::InputText("Name:",&gameobject->Name(),ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if (ImGui::Checkbox("Active", &active))
			gameobject->SetActive(active);
		if (gameobject->HasComponent<oo::PrefabComponent>())
		{
			if (ImGui::Button("Break Prefab"))
			{
				oo::PrefabManager::BreakPrefab(gameobject);
			}
		}
		else
		{
			if (ImGui::Button("Create Prefab"))
			{
				oo::PrefabManager::MakePrefab(gameobject);
			}
		}
		ImGui::Separator();

		DisplayAllComponents(*gameobject);
		DisplayAddComponents(*gameobject, ImGui::GetContentRegionAvail().x * 0.7f, 150);

		//if (disable_prefabEdit)
		//{
		//	ImGui::PopItemFlag();
		//	ImGui::PopStyleColor(3);
		//}
	}
}
void Inspector::DisplayAllComponents(oo::GameObject& gameobject)
{
	ImGui::PushItemWidth(200.0f);
	DisplayComponent<oo::GameObjectComponent>(gameobject);
	DisplayComponent<oo::TransformComponent>(gameobject);
	DisplayComponent<oo::DeferredComponent>(gameobject);
	DisplayComponent<oo::DuplicatedComponent>(gameobject);
	DisplayComponent<oo::GameObjectDisabledComponent>(gameobject);

	DisplayComponent<oo::RigidbodyComponent>(gameobject);
	DisplayComponent<oo::SphereColliderComponent>(gameobject);
	DisplayComponent<oo::BoxColliderComponent>(gameobject);
	DisplayComponent<oo::CapsuleColliderComponent>(gameobject);

	DisplayComponent<oo::GameObjectDebugComponent>(gameobject);
	DisplayComponent<oo::MeshRendererComponent>(gameobject);
	DisplayComponent<oo::DeferredComponent>(gameobject);
	DisplayComponent<oo::LightComponent>(gameobject);
	DisplayComponent<oo::CameraComponent>(gameobject);

	DisplayComponent<oo::AudioListenerComponent>(gameobject);
	DisplayComponent<oo::AudioSourceComponent>(gameobject);
	DisplayComponent<oo::AnimationComponent>(gameobject);
	
	DisplayScript(gameobject);
	ImGui::PopItemWidth();
}
void Inspector::DisplayAddComponents(oo::GameObject& gameobject, float x , float y)
{
	float offset = (ImGui::GetContentRegionAvail().x - x) * 0.5f;
	//LOG_CORE_INFO(ImGui::FindWindowByID(4029469480)->Name);
	ImGui::Dummy({0,0});//for me to use sameline on
	ImGui::SameLine(offset);
	ImGui::PushID("AddC");
	ImGui::BeginGroup();
	m_AddComponentButton.SetSize({ x,50.0f });
	m_AddComponentButton.UpdateToggle();
	bool selected = false;
	if (m_AddComponentButton.GetToggle())
	{
		if (ImGui::BeginChild("##aclistboxchild", { x , 35.0f}, true))
		{
			ImGui::PushItemWidth(-75.0f);
			ImGui::InputText("Search", &m_filterComponents);
			ImGui::PopItemWidth();
		}
		ImGui::EndChild();
		if (ImGui::BeginListBox("##AddComponents", { x,y }))
		{
			selected |= AddComponentSelectable<oo::GameObjectComponent>(gameobject);
			selected |= AddComponentSelectable<oo::RigidbodyComponent>(gameobject);
			selected |= AddComponentSelectable<oo::BoxColliderComponent>(gameobject);
			selected |= AddComponentSelectable<oo::CapsuleColliderComponent>(gameobject);
			selected |= AddComponentSelectable<oo::SphereColliderComponent>(gameobject);
			selected |= AddComponentSelectable<oo::TransformComponent>(gameobject);
			selected |= AddComponentSelectable<oo::MeshRendererComponent>(gameobject);
			selected |= AddComponentSelectable<oo::LightComponent>(gameobject);
			selected |= AddComponentSelectable<oo::CameraComponent>(gameobject);
			selected |= AddComponentSelectable<oo::AudioListenerComponent>(gameobject);
			selected |= AddComponentSelectable<oo::AudioSourceComponent>(gameobject);
			selected |= AddComponentSelectable<oo::DeferredComponent>(gameobject);
			selected |= AddComponentSelectable<oo::AnimationComponent>(gameobject);
			selected |= AddScriptsSelectable(gameobject);

			ImGui::EndListBox();

		}
	}
	if (selected)
	{
		m_AddComponentButton.SetToggle(false);
	}
	ImGui::EndGroup();
	ImGui::PopID();
}
bool Inspector::AddScriptsSelectable(oo::GameObject& go)
{	
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0.7f, 0.7f, 1.0f));
	for (auto& script : oo::ScriptManager::GetScriptList())
	{
		auto name = script.ToString();
		if (m_filterComponents.empty() == false)
		{
			auto iter = std::search(name.begin(), name.end(),
				m_filterComponents.begin(), m_filterComponents.end(), [](char ch1, char ch2)
				{
					return std::toupper(ch1) == std::toupper(ch2);
				});
			if (iter == name.end())
				continue;//not found
		}
		if (ImGui::Selectable(name.c_str(), false))
		{
            auto ss = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->GetWorld().Get_System<oo::ScriptSystem>();
            ss->AddScript(go.GetInstanceID(), script.name_space.c_str(), script.name.c_str());
			go.GetComponent<oo::ScriptComponent>().AddScriptInfo(script);
			ImGui::PopStyleColor();
			return true;
		}
		ImGui::Separator();
	}
	ImGui::PopStyleColor();
	return false;
}
void Inspector::DisplayNestedComponent(rttr::property main_property , rttr::type class_type, rttr::variant& value, bool& edited, bool& endEdit)
{

	ImGui::Text(main_property.get_name().data());
	ImGui::Dummy({ 5.0f,0 });
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::Separator();
	ImGui::PushID(static_cast<int>(class_type.get_id()));

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
				DisplayArrayView(prop , prop_type, v, set_edited, end_edit);
				if (end_edit)
					endEdit = true;
				if (set_edited == true)
				{
					edited = true;
					prop.set_value(value, v);//set value to variant
				}
			}
			else if (prop_type.is_enumeration())
			{
				rttr::variant v = prop.get_value(value);
				bool set_edited = false;
				bool end_edit = false;
				DisplayEnumView(prop, value, set_edited, end_edit);
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

		auto iter = m_inspectorProperties.m_InspectorUI.find(ut->second);
		if (iter == m_inspectorProperties.m_InspectorUI.end())
			continue;

		rttr::variant v = prop.get_value(value);
		bool set_value = false;
		bool end_edit = false; //when the item is let go
		std::string name = prop.get_name().data();

		if (propReadonly)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, ImGui_StylePresets::disabled_color);
			iter->second(prop, name, v, set_value, end_edit);
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		}
		else
		{
			iter->second(prop, name, v, set_value, end_edit);
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
void Inspector::DisplayArrayView(rttr::property main_property, rttr::type variable_type, rttr::variant& value, bool& edited, bool& endEdit)
{
	rttr::variant_sequential_view sqv =	value.create_sequential_view();
	rttr::type::type_id id = sqv.get_value_type().get_id();

	auto ut = UI_RTTRType::types.find(id);
	if (ut == UI_RTTRType::types.end())
		return;
	auto iter = m_inspectorProperties.m_InspectorUI.find(ut->second);
	if (iter == m_inspectorProperties.m_InspectorUI.end())
		return;
	ImGui::Text(main_property.get_name().data());
	ImGui::Separator();
	
	ImGui::PushID(main_property.get_name().data());

	ImGui::Dummy({ 5.0f,0 });
	ImGui::SameLine();
	ImGui::BeginGroup();
	
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
	std::string tempstring = "##e";

	for (size_t i = 0; i < sqv.get_size(); ++i)
	{
		ImGui::PushID(static_cast<int>(i));
		rttr::variant v = sqv.get_value(i).extract_wrapped_value();
		iter->second(main_property,tempstring, v, itemEdited, itemEndEdit);
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

void Inspector::DisplayEnumView(rttr::property prop, rttr::variant& value, bool& edited, bool& endEdit)
{
	rttr::enumeration enumeration = prop.get_enumeration();
	std::string current_enum = enumeration.value_to_name(value).data();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	ImGui::InputText(prop.get_name().data(), &current_enum);
	ImGui::PopItemFlag();
	ImGui::SetItemAllowOverlap();

	ImGui::SameLine(ImGui::CalcItemWidth() - 12.0f);
	static ImGuiID open_id = 0;
	ImGuiID curr_id = ImGui::GetItemID();
	if (ImGui::ArrowButton("button", ImGuiDir_::ImGuiDir_Down))
	{
		if (open_id == curr_id)
		{
			open_id = 0;
		}
		else
		{
			edited = true;
			open_id = curr_id;
		}
	}
	if (open_id == curr_id)
	{
		if (ImGui::BeginListBox("#enums"))
		{
			for (auto val : enumeration.get_values())
			{
				if (ImGui::Selectable(enumeration.value_to_name(val).data()))
				{
					value.clear();
					value = val;
					open_id = 0;
					endEdit = true;
				}
			}
			ImGui::EndListBox();
		}
	}
}

void Inspector::DisplayScript(oo::GameObject& gameobject)
{
    auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
    auto ss = scene->GetWorld().Get_System<oo::ScriptSystem>();

	static oo::ScriptFieldInfo pre_val;
	static bool new_value = true;
	auto& sc = gameobject.GetComponent<oo::ScriptComponent>();
	for (auto& scriptInfo : sc.GetScriptInfoAll())
	{
		ImGui::PushID(scriptInfo.first.c_str());
		bool opened = ImGui::TreeNodeEx(scriptInfo.first.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen);
		{
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 10.0f);
			if (ImGui::SmallButton("x"))
			{
                ss->RemoveScript(gameobject.GetInstanceID(), scriptInfo.second.classInfo.name_space.c_str(), scriptInfo.second.classInfo.name.c_str());
				sc.RemoveScriptInfo(scriptInfo.second.classInfo);
				ImGui::PopID();
				return;
			}
		}
		ImGui::Separator();
		if (opened == false)
		{
			ImGui::PopID();
			continue;
		}
		for (auto& sfi : scriptInfo.second.fieldMap)
		{
			bool edit = false;
			bool edited = false;
			oo::ScriptFieldInfo s_value = sfi.second;
			auto iter = m_scriptingProperties.m_scriptUI.find(sfi.second.value.GetValueType());
			if (iter == m_scriptingProperties.m_scriptUI.end())
				continue;
			else
			{
				ImGui::PushID(sfi.first.c_str());
				iter->second(s_value, edit, edited);
				ImGui::PopID();
			}

			//undo redo code here
			if (edit == true)
			{
				if (new_value)
				{
					pre_val = sfi.second;
					new_value = false;
				}
				sfi.second.value = s_value.value;
			}
			if (edited == true)
			{
				oo::CommandStackManager::AddCommand(
					new oo::Script_ActionCommand(
						scriptInfo.first,
						sfi.first,
						pre_val.value,
						s_value.value,
						gameobject.GetInstanceID()));
				new_value = true;
			}
		}
		ImGui::PopID();
		ImGui::Separator();
	}
}
