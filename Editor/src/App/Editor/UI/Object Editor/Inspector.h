#pragma once
#include <Ouroboros/ECS/GameObject.h>

#include <rttr/type.h>
#include <rttr/property.h>

#include <imgui/imgui.h>

#include <unordered_map>
#include <functional>
#include <string>


#include <App/Editor/Utility/UI_RTTRType.h>

class Inspector
{
public:
	Inspector();

	void Show();
private:
	template <typename Component>
	void DisplayComponent(oo::GameObject& gameobject);
	std::unordered_map<UI_RTTRType::UItypes, std::function<void(std::string& name,rttr::variant & v, bool & edited)>> m_InspectorUI;
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
	ImGui::PushID(component.Id);
	for (rttr::property prop : type.get_properties())
	{
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
		
		iter->second(name, v, set_value);
		if(set_value == true)
			prop.set_value(component,v);
	}
	ImGui::PopID();
	ImGui::Separator();
}
