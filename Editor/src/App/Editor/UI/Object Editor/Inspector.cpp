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
#include <Ouroboros/Commands/ScriptAR_ActionCommand.h>

#include <Ouroboros/Audio/AudioListenerComponent.h>
#include <Ouroboros/Audio/AudioSourceComponent.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/ECS/DeferredComponent.h>
#include <Ouroboros/ECS/DuplicatedComponent.h>
#include <Ouroboros/ECS/JustCreatedComponent.h>
#include <Ouroboros/Transform/TransformComponent.h>
#include <Ouroboros/Prefab/PrefabComponent.h>

#include <Ouroboros/Physics/RigidbodyComponent.h>
#include <Ouroboros/Physics/ColliderComponents.h>

#include <Ouroboros/Scripting/ScriptComponent.h>
#include <Ouroboros/Scripting/ScriptSystem.h>
#include <Ouroboros/Scripting/ScriptManager.h>
#include <Ouroboros/Vulkan/MeshRendererComponent.h>
#include <Ouroboros/Vulkan/ParticleEmitterComponent.h>
#include <Ouroboros/Vulkan/SkinRendererComponent.h>
#include <Ouroboros/Vulkan/LightComponent.h>
#include <Ouroboros/Vulkan/CameraComponent.h>

#include <glm/gtc/type_ptr.hpp>
#include <Ouroboros/ECS/GameObjectDebugComponent.h>
#include <Ouroboros/ECS/GameObjectDisabledComponent.h>
#include <Ouroboros/Animation/AnimationComponent.h>

#include <Ouroboros/UI/RectTransformComponent.h>
#include <Ouroboros/UI/UIRaycastComponent.h>
#include <Ouroboros/UI/UICanvasComponent.h>
#include <Ouroboros/UI/UIImageComponent.h>
#include <Ouroboros/UI/GraphicsRaycasterComponent.h>
#include <Ouroboros/UI/UIComponent.h>
#include <Ouroboros/UI/UITextComponent.h>

#include <Ouroboros/Editor/EditorComponent.h>

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
	std::vector<std::shared_ptr<oo::GameObject>> selected_list;
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	for (auto items : selected_items)
	{
		auto gameobject = scene->FindWithInstanceID(items); //first item
		if (gameobject == nullptr)
			continue;
		selected_list.push_back(gameobject);
	}
	size_t size = selected_list.size();
	if (size == 0)
		return;
	if (size > 1)
	{
		DisplayAddComponents(selected_list, ImGui::GetContentRegionAvail().x * 0.7f, 150);
		return;
	}

	{
		auto gameobject = selected_list.back();
		//bool active = gameobject->ActiveInHierarchy();

		//bool disable_prefabEdit = gameobject->GetIsPrefab() && gameobject->HasComponent<oo::PrefabComponent>() == false;
		//if (disable_prefabEdit)
		//{
		//	ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled,true);
		//	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImGui_StylePresets::disabled_color);
		//	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, ImGui_StylePresets::disabled_color);
		//	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, ImGui_StylePresets::disabled_color);
		//}
		bool active = gameobject->IsActive();
		std::string temp_name = gameobject->Name();
		if (ImGui::InputText("Name:", &temp_name, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
		{
			oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<oo::GameObjectComponent>
				(gameobject->Name(), temp_name, rttr::type::get<oo::GameObjectComponent>().get_property("Name"), gameobject->GetInstanceID()));
			gameobject->Name() = temp_name;
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Active", &active))
		{
			oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<oo::GameObjectComponent>
				(gameobject->IsActive(), active, rttr::type::get<oo::GameObjectComponent>().get_property("Active"), gameobject->GetInstanceID()));
			gameobject->SetActive(active);
		}
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

		// Input Layer only applies to items with rigidbody
		if(gameobject->HasComponent<oo::RigidbodyComponent>())
		{
			ImGui::SameLine();

			ImGui::BeginGroup();
			ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
			std::string inLayer = "Layers";
			ImGui::PushItemWidth(100);
			ImGui::InputText("##InputLayers", &inLayer);
			ImGui::PopItemWidth();
			ImGui::PopItemFlag();

			ImGui::SameLine();
			if (ImGui::ArrowButton("##Phy InLayers", ImGuiDir_::ImGuiDir_Down) == true)
			{
				ImGui::OpenPopup("##Edit InLayers");
			}
			if (ImGui::BeginPopup("##Edit InLayers"))
			{
				auto& names = oo::RigidbodyComponent::LayerNames;
				auto& layer = gameobject->GetComponent<oo::RigidbodyComponent>().InputLayer;
				auto& collideAgainstlayer = gameobject->GetComponent<oo::RigidbodyComponent>().OutputLayer;
				if(ImGui::BeginTable("##layersTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
				{
					ImGui::TableNextColumn();
					ImGui::Text("Layer Name         ");
					ImGui::TableNextColumn();
					ImGui::Text("Identify as");
					ImGui::TableNextColumn();
					ImGui::Text("Collide Against");
					ImGui::TableNextColumn();
					for (size_t i = 0; i < names.size(); ++i)
					{
						ImGui::PushID(i);
						ImGui::InputText("##Layer Name", &names[i]);
						ImGui::TableNextColumn();
						bool layerValue = layer[i];
						if (ImGui::Checkbox("##Layer value", &layerValue))
						{
							layer[i] = layerValue;
						}
						ImGui::TableNextColumn();
						bool layerValue2 = collideAgainstlayer[i];
						if (ImGui::Checkbox("##Layer value2", &layerValue2))
						{
							collideAgainstlayer[i] = layerValue2;
						}
						ImGui::TableNextColumn();
						ImGui::PopID();
					}
					ImGui::EndTable();
				}


				ImGui::EndPopup();
			}

			ImGui::EndGroup();

		}
		
		ImGui::Separator();

		DisplayAllComponents(*gameobject);
		DisplayAddComponents(selected_list, ImGui::GetContentRegionAvail().x * 0.7f, 150);

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
	//DisplayComponent<oo::GameObjectComponent>(gameobject);
	// let's display both for now.
	if (gameobject.HasComponent<oo::RectTransformComponent>())
	{
		// for debug purposes.
		if (m_showReadonly)
			DisplayComponent<oo::TransformComponent>(gameobject);
		DisplayComponent<oo::RectTransformComponent>(gameobject);
	}
	else
	{
		DisplayComponent<oo::TransformComponent>(gameobject);
	}
	DisplayComponent<oo::EditorComponent>(gameobject);

	DisplayComponent<oo::JustCreatedComponent>(gameobject);
	DisplayComponent<oo::DeferredComponent>(gameobject);
	DisplayComponent<oo::DuplicatedComponent>(gameobject);
	DisplayComponent<oo::GameObjectDisabledComponent>(gameobject);

	DisplayComponent<oo::RigidbodyComponent>(gameobject);
	DisplayComponent<oo::SphereColliderComponent>(gameobject);
	DisplayComponent<oo::BoxColliderComponent>(gameobject);
	DisplayComponent<oo::CapsuleColliderComponent>(gameobject);
	DisplayComponent<oo::ConvexColliderComponent>(gameobject);

	DisplayComponent<oo::MeshRendererComponent>(gameobject);
	DisplayComponent<oo::ParticleEmitterComponent>(gameobject);
	DisplayComponent<oo::SkinMeshRendererComponent>(gameobject);
	DisplayComponent<oo::LightComponent>(gameobject);
	DisplayComponent<oo::CameraComponent>(gameobject);

	DisplayComponent<oo::AudioListenerComponent>(gameobject);
	DisplayComponent<oo::AudioSourceComponent>(gameobject);
	DisplayComponent<oo::AnimationComponent>(gameobject);

	DisplayComponent<oo::UIComponent>(gameobject);
	DisplayComponent<oo::UITextComponent>(gameobject);
	DisplayComponent<oo::UIRaycastComponent>(gameobject);
	DisplayComponent<oo::UICanvasComponent>(gameobject);
	DisplayComponent<oo::UIImageComponent>(gameobject);
	DisplayComponent<oo::GraphicsRaycasterComponent>(gameobject);

	if(m_showReadonly)
		DisplayComponent<oo::GameObjectDebugComponent>(gameobject);

	DisplayScript(gameobject);
	ImGui::PopItemWidth();
}
void Inspector::DisplayAddComponents(const std::vector<std::shared_ptr<oo::GameObject>>& gameobject, float x , float y)
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
			// Components that you're not supposed to be able to add
			//selected |= AddComponentSelectable<oo::GameObjectComponent>(gameobject);
			//selected |= AddComponentSelectable<oo::DeferredComponent>(gameobject);
			//selected |= AddComponentSelectable<oo::DuplicatedComponent>(gameobject);
			//selected |= AddComponentSelectable<oo::TransformComponent>(gameobject);
			
			// Components that should be in the final version
			selected |= AddComponentSelectable<oo::RigidbodyComponent>(gameobject);
			selected |= AddComponentSelectable<oo::BoxColliderComponent>(gameobject);
			selected |= AddComponentSelectable<oo::CapsuleColliderComponent>(gameobject);
			selected |= AddComponentSelectable<oo::SphereColliderComponent>(gameobject);
			selected |= AddComponentSelectable<oo::ConvexColliderComponent>(gameobject);

			selected |= AddComponentSelectable<oo::MeshRendererComponent>(gameobject);
			selected |= AddComponentSelectable<oo::ParticleEmitterComponent>(gameobject);
			selected |= AddComponentSelectable<oo::SkinMeshRendererComponent>(gameobject);
			selected |= AddComponentSelectable<oo::LightComponent>(gameobject);
			selected |= AddComponentSelectable<oo::CameraComponent>(gameobject);
			selected |= AddComponentSelectable<oo::AudioListenerComponent>(gameobject);
			selected |= AddComponentSelectable<oo::AudioSourceComponent>(gameobject);
			selected |= AddComponentSelectable<oo::AnimationComponent>(gameobject);

			
			selected |= AddComponentSelectable<oo::UITextComponent>(gameobject);
			selected |= AddComponentSelectable<oo::UIComponent>(gameobject);
			selected |= AddComponentSelectable<oo::RectTransformComponent>(gameobject);
			selected |= AddComponentSelectable<oo::UIRaycastComponent>(gameobject);
			selected |= AddComponentSelectable<oo::UICanvasComponent>(gameobject);
			selected |= AddComponentSelectable<oo::GraphicsRaycasterComponent>(gameobject);
			selected |= AddComponentSelectable<oo::UIImageComponent>(gameobject);

			selected |= AddComponentSelectable<oo::EditorComponent>(gameobject);

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

bool Inspector::AddScriptsSelectable(const std::vector<std::shared_ptr<oo::GameObject>>& go_list)
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
			for (auto go : go_list)
			{
				ss->AddScript(go->GetInstanceID(), script.name_space.c_str(), script.name.c_str());
				go->GetComponent<oo::ScriptComponent>().AddScriptInfo(script);
				oo::CommandStackManager::AddCommand(new oo::ScriptAdd_ActionCommand(go->GetInstanceID(), script.name_space, script.name));
			}
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

	ImGui::Dummy({ 5.0f,0 });
	ImGui::SameLine();
	ImGui::BeginGroup();
	
	//ImGui::Text(main_property.get_name().data());
	ImVec4 hc = ImGui::GetStyleColorVec4(ImGuiCol_Header);
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Header, ImVec4(hc.y,hc.z,hc.x,0.6f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderHovered, ImVec4(hc.y, hc.z, hc.x, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderActive, ImVec4(hc.y, hc.z, hc.x, 1.0f));
	bool opened = ImGui::CollapsingHeader(main_property.get_name().data(), ImGuiTreeNodeFlags_NoTreePushOnOpen);
	ImGui::PopStyleColor(3);
	
	if (opened == false)
	{
		ImGui::EndGroup();
		return;
	}

	ImGui::PushID(static_cast<int>(class_type.get_id()));
	int sameline_next = 0;
	float UI_sameline_size = 0;
	float itemsize_sameline = 0;
	for (rttr::property prop : class_type.get_properties())
	{
		bool propReadonly = prop.is_readonly();
		if (propReadonly && m_showReadonly == false)
			continue;

		{
			rttr::variant same_linewith = prop.get_metadata(UI_metadata::SAME_LINE_WITH_NEXT);
			if (same_linewith.is_valid())
			{
				sameline_next = same_linewith.get_value<int>();
				float area = ImGui::GetContentRegionAvail().x;
				constexpr float textlen = 50 + 10;
				itemsize_sameline = ImGui::GetCurrentWindow()->DC.ItemWidth;
				UI_sameline_size = (area) / sameline_next - (textlen);//10 is padding
			}
		}
		if (sameline_next)
		{
			ImGui::PushItemWidth(UI_sameline_size);
			ImGui::BeginGroup();
		}
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
				DisplayEnumView(prop, v, set_edited, end_edit);
				if (end_edit)
					endEdit = true;
				if (set_edited == true)
				{
					edited = true;
					prop.set_value(value, v);//set value to variant
				}
			}
			else//not found
			{
				if (sameline_next)
				{
					ImGui::EndGroup();
					ImGui::PopItemWidth();
					sameline_next = 0;
					UI_sameline_size = 0;
				}
				continue;
			}
			//found
			if (sameline_next)
			{
				--sameline_next;
				ImGui::EndGroup();
				ImGui::PopItemWidth();
				if (sameline_next)
				{
					ImGui::SameLine();
				}
				else
				{
					ImGui::GetCurrentWindow()->DC.ItemWidth = itemsize_sameline;
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
		if (sameline_next)
		{
			--sameline_next;
			ImGui::EndGroup();
			ImGui::PopItemWidth();
			if (sameline_next)
			{
				ImGui::SameLine();
			}
			else
			{
				ImGui::GetCurrentWindow()->DC.ItemWidth = itemsize_sameline;
			}
		}
	}
	ImGui::PopID();
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

	ImVec2 cursorpos = ImGui::GetCursorPos();
	//Size Slider
	if (ImGui::DragScalarN("##Size", ImGuiDataType_::ImGuiDataType_U64, &size, 1,0.3f,&min_arrSize,&max_arrSize))
		edited = endEdit = sqv.set_size(size);
	ImGui::SetItemAllowOverlap();
	float width = ImGui::CalcItemWidth();
	ImGui::SetCursorPos(cursorpos);
	if (ImGui::ArrowButton("reduceSize", ImGuiDir_::ImGuiDir_Left))
		edited = endEdit = sqv.set_size(size - 1);
	ImGui::SetCursorPos({ cursorpos.x + width - 17, cursorpos.y});//15 = arrowbuttonsize
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
		if (ImGui::BeginListBox("##enums"))
		{
			for (auto val : enumeration.get_values())
			{
				if (ImGui::Selectable(enumeration.value_to_name(val).data()))
				{
					value.clear();
					value = val;
					open_id = 0;
					endEdit = true;
					edited = true;
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
		ImVec4 hc = ImGui::GetStyleColorVec4(ImGuiCol_Header);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Header, ImVec4(hc.z, hc.y, hc.x, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderHovered, ImVec4(hc.z, hc.y, hc.x, 0.8f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_HeaderActive, ImVec4(hc.z, hc.y, hc.x, 1.0f));
		bool opened = ImGui::CollapsingHeader(scriptInfo.first.c_str(), ImGuiTreeNodeFlags_NoTreePushOnOpen);
		ImGui::PopStyleColor(3);
		{
			ImGui::SetItemAllowOverlap();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 10.0f);
			if (ImGui::SmallButton("x"))
			{
                ss->RemoveScript(gameobject.GetInstanceID(), scriptInfo.second.classInfo.name_space.c_str(), scriptInfo.second.classInfo.name.c_str());
				oo::CommandStackManager::AddCommand(
					new oo::ScriptRemove_ActionCommand(
						gameobject.GetInstanceID(),
						scriptInfo.second.classInfo.name_space,
						scriptInfo.second.classInfo.name
					)
				);

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
		//for (auto& sfi : scriptInfo.second.fieldMap)

        for(auto& fieldName : scriptInfo.second.displayOrder)
		{
            auto& sfi = *scriptInfo.second.fieldMap.find(fieldName);

			bool edit = false;
			bool edited = false;
			oo::ScriptFieldInfo s_value = sfi.second;
			if (s_value.GetHeader().empty() == false)
			{
				ImFont tempfont = *ImGui::GetFont();
				tempfont.Scale *= 1.1f;
				ImGui::PushFont(&tempfont);
				ImGui::Text(s_value.GetHeader().c_str());
				ImGui::PopFont();
			}
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
