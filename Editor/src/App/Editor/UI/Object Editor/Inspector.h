#pragma once
//gameobject for getting component
#include <Ouroboros/ECS/GameObject.h>
//undo redo commands
#include <Ouroboros/Commands/Component_ActionCommand.h>
#include <Ouroboros/Commands/CommandStackManager.h>
//rttr stuffs
#include <rttr/type.h>
#include <rttr/property.h>
//imgui stuff
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
//std libs
#include <unordered_map>
#include <functional>
#include <string>

//editor utility
#include <App/Editor/Utility/UI_RTTRType.h>
#include <App/Editor/Utility/ImGuiStylePresets.h>

class Inspector
{
public:
	Inspector();

	void Show();
private:
	template <typename Component>
	void DisplayComponent(oo::GameObject& gameobject);
	template <typename Component>
	void SaveComponentDataHelper(Component& component, rttr::property prop, rttr::variant& pre_value, rttr::variant&& edited_value, UUID id, bool edited, bool endEdit );
	void DisplayNestedComponent(rttr::type class_type, rttr::variant& value, bool& edited, bool& endEdit);

	void DisplayArrayView(rttr::type class_type, rttr::variant& value, bool& edited, bool& endEdit);
private:
	std::unordered_map<UI_RTTRType::UItypes, std::function<void(std::string& name,rttr::variant & v, bool & edited , bool& endEdit)>> m_InspectorUI;
	bool m_showReadonly = false;
};
template<typename Component>
inline void Inspector::SaveComponentDataHelper(Component& component, rttr::property prop, rttr::variant& pre_value, rttr::variant&& edited_value, UUID id, bool edited, bool endEdit)
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
	
	ImGui::Text(type.get_name().data());
	ImGui::Separator();
	ImGui::PushID(type.get_id());
	for (rttr::property prop : type.get_properties())
	{
		bool propReadonly = prop.is_readonly();
		if (propReadonly && m_showReadonly == false)
			continue;

		rttr::type prop_type = prop.get_type();

		auto ut = UI_RTTRType::types.find(prop_type.get_id());
		if (ut == UI_RTTRType::types.end())
			continue;

		auto iter = m_InspectorUI.find(ut->second);
		//special cases
		if (iter == m_InspectorUI.end())
		{
			//nested variables & arrays
			if (prop_type.is_array())//array
			{
				//ASSERT_MSG(true, "AYO u doing illegal shit");
				rttr::variant value = prop.get_value(component);
				bool edited = false;
				bool end_edit = false;
				if (prop_type.is_sequential_container())
				{
					DisplayArrayView(prop_type,value,edited,end_edit);
					SaveComponentDataHelper(component, prop, pre_edited, std::move(value), gameobject.GetInstanceID(), edited, end_edit);
				}
			}
			else if (prop_type.is_class())//nested
			{
				rttr::variant value = prop.get_value(component);
				bool edited = false;
				bool end_edit = false;
				DisplayNestedComponent(prop_type, value, edited, end_edit);
				SaveComponentDataHelper(component, prop, pre_edited, std::move(value), gameobject.GetInstanceID(), edited, end_edit);
			}
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
			iter->second(name, v, set_value, end_edit);
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		}
		else
		{
			iter->second(name, v, set_value, end_edit);
			SaveComponentDataHelper(component, prop, pre_edited, std::move(v), gameobject.GetInstanceID(), set_value, end_edit);
		}
		ImGui::Dummy({ 0,5.0f });
	}
	ImGui::PopID();
	ImGui::Separator();
}
