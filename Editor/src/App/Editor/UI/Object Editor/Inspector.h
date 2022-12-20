/************************************************************************************//*!
\file          Inspector.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Declarations for Inspector 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
//gameobject for getting component
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Asset/Asset.h>
//undo redo commands
#include <Ouroboros/Commands/Component_ActionCommand.h>
#include <Ouroboros/Commands/CommandStackManager.h>
//rttr stuffs
#include <rttr/type.h>
#include <rttr/property.h>
#include <rttr/variant_sequential_view.h>
//imgui stuff
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
//std libs
#include <unordered_map>
#include <functional>
#include <string>

//editor utility
#include <App/Editor/Utility/ImGuiStylePresets.h>
#include <App/Editor/Utility/ImGui_ToggleButton.h>
#include <App/Editor/Properties/InspectorProperties.h>
#include <App/Editor/Properties/ScriptingProperties.h>
#include <App/Editor/Properties/UI_metadata.h>
//undo redo for adding components
#include <Ouroboros/Commands/CommandStackManager.h>
#include <Ouroboros/Commands/AddComponent_ActionCommand.h>
class Inspector
{
public:
	Inspector();

	void Show();
private:
	void DisplayAllComponents(oo::GameObject& gameobject);
	void DisplayAddComponents(const std::vector<std::shared_ptr<oo::GameObject>>& gameobject,float x,float y);
	//template <typename Component>
	//bool AddComponentSelectable(oo::GameObject& go);
	template <typename Component>
	bool AddComponentSelectable(const std::vector<std::shared_ptr<oo::GameObject>>& go_list);

	bool AddScriptsSelectable(const std::vector<std::shared_ptr<oo::GameObject>>& go_list);
private: 
	ToggleButton m_AddComponentButton;
	std::string m_filterComponents = "";
private: //inspecting functions
	template <typename Component>
	void DisplayComponent(oo::GameObject& gameobject);
	template <typename Component>
	void SaveComponentDataHelper(Component& component, rttr::property prop, rttr::variant& pre_value, rttr::variant&& edited_value, oo::UUID id, bool edited, bool endEdit );
	void DisplayNestedComponent(rttr::property prop ,rttr::type class_type, rttr::variant& value, bool& edited, bool& endEdit);
	void DisplayArrayView(rttr::property prop,rttr::type class_type, rttr::variant& value, bool& edited, bool& endEdit);
	void DisplayEnumView(rttr::property prop, rttr::variant& value, bool& edited, bool& endEdit);
	void DisplayScript(oo::GameObject& gameobject);

private: //inspecting elements
	InspectorProperties m_inspectorProperties;
	ScriptingProperties m_scriptingProperties;
	bool m_showReadonly = false;
};
template<typename Component>
inline void Inspector::SaveComponentDataHelper(Component& component, rttr::property prop, rttr::variant& pre_value, rttr::variant&& edited_value, oo::UUID id, bool edited, bool endEdit)
{
	if (edited == true)
	{
		if (pre_value.is_valid() == false)//if the variant is empty that means it is ready to hold data
			pre_value = prop.get_value(component);
		//set value to component
		prop.set_value(component, edited_value);
	}
	if (endEdit)
	{
		//undo redo command
		oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<Component>
			(pre_value, edited_value, prop, id));
		pre_value.clear();//reset this variant
	}
}
//template<typename Component>
//inline bool Inspector::AddComponentSelectable(oo::GameObject& go)
//{
//	std::string name = rttr::type::get<Component>().get_name().data();
//	auto iter = std::search(name.begin(), name.end(),
//		m_filterComponents.begin(), m_filterComponents.end(), [](char ch1, char ch2)
//		{
//			return std::toupper(ch1) == std::toupper(ch2);
//		});
//	if (iter == name.end())
//		return false;//not found
//	if (ImGui::Selectable(name.c_str(), false))
//	{
//		if (go.HasComponent<Component>() == false)
//		{
//			go.AddComponent<Component>();
//			oo::CommandStackManager::AddCommand(new oo::AddComponent_ActionCommand<Component>(go));
//		}
//		return true;
//	}
//	ImGui::Separator();
//	return false;
//}
template<typename Component>
inline bool Inspector::AddComponentSelectable(const std::vector<std::shared_ptr<oo::GameObject>>& go)
{
	std::string name = rttr::type::get<Component>().get_name().data();
	auto iter = std::search(name.begin(), name.end(),
		m_filterComponents.begin(), m_filterComponents.end(), [](char ch1, char ch2)
		{
			return std::toupper(ch1) == std::toupper(ch2);
		});
	if (iter == name.end())
		return false;//not found
	if (ImGui::Selectable(name.c_str(), false))
	{
		for (auto gameobj : go)
		{
			if (gameobj->HasComponent<Component>() == false)
			{
				gameobj->AddComponent<Component>();
				oo::CommandStackManager::AddCommand(new oo::AddComponent_ActionCommand<Component>(*gameobj));
			}
		}
		return true;
	}
	ImGui::Separator();
	return false;
}
template<typename Component>
inline void Inspector::DisplayComponent(oo::GameObject& gameobject)
{
	//tracks the currently edited item.
	//the item will get reseted once the edit ends.
	//the reset must happen else something bad will happen
	static rttr::variant pre_edited;
	
	if (gameobject.HasComponent<Component>() == false)
		return;
	
	auto& component = gameobject.GetComponent<Component>();
	rttr::type type = component.get_type();
	bool open = ImGui::CollapsingHeader(type.get_name().data(), ImGuiTreeNodeFlags_NoTreePushOnOpen);
	//ImGui::TreeNodeEx(type.get_name().data(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushID(type.get_name().data());
	{
		bool smallbtn = true;
		rttr::variant metadata_removable = type.get_metadata(UI_metadata::NOT_REMOVABLE);
		if (metadata_removable.is_valid())
			smallbtn = false;
		if (smallbtn)
		{
			ImGui::SetItemAllowOverlap();
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 10.0f);
			if (ImGui::SmallButton("x"))
			{
				gameobject.RemoveComponent<Component>();
				oo::CommandStackManager::AddCommand(new oo::RemoveComponent_ActionCommand<Component>(gameobject));
				ImGui::PopID();
				return;
			}
		}
	}
	ImGui::PopID();

	ImGui::Separator();
	if (open == false)
		return;

	ImGui::PushID(static_cast<int>(type.get_id()));
	int sameline_next = 0;
	float UI_sameline_size = 0;
	for (rttr::property prop : type.get_properties())
	{
		bool propReadonly = prop.is_readonly();
		rttr::variant variant_hidden = prop.get_metadata(UI_metadata::HIDDEN);
		if (variant_hidden.is_valid())
			propReadonly = variant_hidden.get_value<bool>();

		if (propReadonly && m_showReadonly == false)
			continue;
		{
			rttr::variant same_linewith = prop.get_metadata(UI_metadata::SAME_LINE_WITH_NEXT);
			if (same_linewith.is_valid())
			{
				sameline_next = same_linewith.get_value<int>();
				float area = ImGui::GetContentRegionAvail().x;
				constexpr float textlen = 30 + 10;//10 is padding

				UI_sameline_size = (area) / sameline_next - (textlen);
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
			//nested variables & arrays
			if (prop_type.is_sequential_container())//vectors and lists
			{
				rttr::variant value = prop.get_value(component);
				bool edited = false;
				bool end_edit = false;
				DisplayArrayView(prop ,prop_type, value, edited, end_edit);
				SaveComponentDataHelper(component, prop, pre_edited, std::move(value), gameobject.GetInstanceID(), edited, end_edit);
			}
			else if (prop_type.is_class())//nested
			{
				rttr::variant value = prop.get_value(component);
				bool edited = false;
				bool end_edit = false;
				DisplayNestedComponent(prop ,prop_type, value, edited, end_edit);
				SaveComponentDataHelper(component, prop, pre_edited, std::move(value), gameobject.GetInstanceID(), edited, end_edit);
			}
			else if (prop_type.is_enumeration())
			{
				rttr::variant value = prop.get_value(component);
				bool edited = false;
				bool end_edit = false;
				DisplayEnumView(prop, value, edited, end_edit);
				SaveComponentDataHelper(component, prop, pre_edited, std::move(value), gameobject.GetInstanceID(), edited, end_edit);

			}
			else//not found
			{
				if (sameline_next)
				{
					ImGui::EndGroup();
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
					ImGui::SameLine();
			}
			continue;
		}

		auto iter = m_inspectorProperties.m_InspectorUI.find(ut->second);
		//special cases
		if (iter == m_inspectorProperties.m_InspectorUI.end())
		{
			ASSERT_MSG(true, "this means u nvr ask me to support your type before putting in. -jx");
			continue;
		}

		rttr::variant v = prop.get_value(component);
		bool set_value = false;
		bool end_edit = false; //when the item is let go
		std::string name = prop.get_name().data();
		
		if (propReadonly)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled,true);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, ImGui_StylePresets::disabled_color);
			iter->second(prop,name, v, set_value, end_edit);
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		}
		else
		{
			iter->second(prop,name, v, set_value, end_edit);
			SaveComponentDataHelper(component, prop, pre_edited, std::move(v), gameobject.GetInstanceID(), set_value, end_edit);
		}
		ImGui::Dummy({ 0,5.0f });
		if (sameline_next)
		{
			--sameline_next;
			ImGui::EndGroup();
			ImGui::PopItemWidth();
			if(sameline_next)
				ImGui::SameLine();
		}
	}
	ImGui::PopID();
	ImGui::Separator();
}
