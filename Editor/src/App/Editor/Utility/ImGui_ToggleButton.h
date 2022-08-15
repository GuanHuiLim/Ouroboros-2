#pragma once
#include <string>
#include <imgui/imgui.h>
class ToggleButton
{
public:
	ToggleButton(const std::string & name , bool default_state,ImVec2 buttonsize, ImColor color_toggled , ImColor default_color);
	bool UpdateToggle();
private:
	ImColor m_toggled_color;
	ImColor m_default_color;
	ImVec2 m_buttonsize;
	const std::string m_buttonName;
	bool m_toggled = false;

};