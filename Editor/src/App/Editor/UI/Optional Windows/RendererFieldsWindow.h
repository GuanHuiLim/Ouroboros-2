#pragma once
#include "App/Editor/Properties/InspectorProperties.h"

#include "App/Editor/Properties/UI_RTTRType.h"
#include <rttr/type.h>
#include <rttr/variant.h>
#include <rttr/property.h>
class RendererFieldsWindow
{
public:
	RendererFieldsWindow();
	void Show();
private:
	template <typename Setting>
	bool DisplaySetting(Setting& setting);
	InspectorProperties m_properties;
};

template<typename Setting>
inline bool RendererFieldsWindow::DisplaySetting(Setting& setting)
{
	auto& display_value = setting;
	rttr::type t = display_value.get_type();
	bool editing = false;
	for (auto prop : t.get_properties())
	{
		auto typeiter = UI_RTTRType::types.find(prop.get_type().get_id());
		if (typeiter == UI_RTTRType::types.end())
			continue;
		auto ui_iter = m_properties.m_InspectorUI.find(typeiter->second);
		if (ui_iter == m_properties.m_InspectorUI.end())
			continue;
		rttr::variant val = prop.get_value(display_value);

		bool edited = false;
		bool editend = false;
		std::string name = prop.get_name().data();

		ui_iter->second(prop, name, val, edited, editend);
		prop.set_value(display_value, val);
		editing |= edited | editend;
	}
	return editing;
}
