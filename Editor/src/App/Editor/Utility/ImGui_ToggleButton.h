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

class ColorButton
{
public:
	ColorButton(std::vector<std::string>&& names, std::vector<ImColor>&& colors, ImVec2 buttonsize, int start_index);
	bool UpdateToggle();
	int GetIndex();
private:
	std::vector<ImColor> m_colorList;
	std::vector<std::string> m_names;
	int m_index;
	ImVec2 m_buttonsize;
};
