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
class Inspector
{
public:
	Inspector();

	void Show();
private:
	void DisplayAllComponents(oo::GameObject& gameobject);
	void DisplayAddComponents(oo::GameObject& gameobject,float x,float y);
	template <typename Component>
	bool AddComponentSelectable(oo::GameObject& go);
	bool AddScriptsSelectable(oo::GameObject& go);
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

	void DisplayScript(oo::GameObject& gameobject);

private: //inspecting elements
	InspectorProperties m_inspectorProperties;
	ScriptingProperties m_scriptingProperties;
	bool m_showReadonly = false;
};
template<typename Component>
inline void Inspector::SaveComponentDataHelper(Component& component, rttr::property prop, rttr::variant& pre_value, rttr::variant&& edited_value, oo::UUID id, bool edited, bool endEdit)
{
	if (endEdit)
	{
		//undo redo command
		oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<Component>
			(pre_value, edited_value, prop, id));
		pre_value.clear();//reset this variant
	}
	if (edited == true)
	{
		if (pre_value.is_valid() == false)//if the variant is empty that means it is ready to hold data
			pre_value = prop.get_value(component);
		//set value to component
		prop.set_value(component, edited_value);
	}
}
template<typename Component>
inline bool Inspector::AddComponentSelectable(oo::GameObject& go)
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
		go.AddComponent<Component>();
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
	bool open = ImGui::TreeNodeEx(type.get_name().data(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::PushID(type.get_name().data());
	{
		bool smallbtn = true;
		rttr::variant metadata_removable = type.get_metadata(UI_metadata::NOT_REMOVABLE);
		if (metadata_removable.is_valid())
			smallbtn = false;
		if (smallbtn)
		{
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - 10.0f);
			if (ImGui::SmallButton("x"))
			{
				gameobject.RemoveComponent<Component>();
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
	for (rttr::property prop : type.get_properties())
	{
		bool propReadonly = prop.is_readonly();
		rttr::variant variant_hidden = prop.get_metadata(UI_metadata::HIDDEN);
		if (variant_hidden.is_valid())
			propReadonly = variant_hidden.get_value<bool>();

		if (propReadonly && m_showReadonly == false)
			continue;

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
			continue;
		}

		auto iter = m_inspectorProperties.m_InspectorUI.find(ut->second);
		//special cases
		if (iter == m_inspectorProperties.m_InspectorUI.end())
			continue;

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
	}
	ImGui::PopID();
	ImGui::Separator();
}
