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
	std::unordered_map<UI_RTTRType::UItypes, std::function<void(std::string& name,rttr::variant & v, bool & edited)>> m_InspectorUI;

private:
	bool m_showReadonly = false;
};

template<typename Component>
inline void Inspector::DisplayComponent(oo::GameObject& gameobject)
{
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
		if (iter == m_InspectorUI.end())
			continue;

		rttr::variant v = prop.get_value(component);
		bool set_value = false;
		std::string name = prop.get_name().data();
		
		if (propReadonly)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled,true);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, ImGui_StylePresets::disabled_color);
			iter->second(name, v, set_value);
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		}
		else
		{
			iter->second(name, v, set_value);
			if (set_value == true)
			{
				//undo redo command
				oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<Component>
					(prop.get_value(component), v, prop, gameobject.GetInstanceID()));
				//set value to component
				prop.set_value(component,v);
			}
		}
		ImGui::Dummy({ 0,5.0f });
	}
	ImGui::PopID();
	ImGui::Separator();
}
