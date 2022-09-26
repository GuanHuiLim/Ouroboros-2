/************************************************************************************//*!
\file           ImGui_ToggleButton.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          Contains Both ColorButton and ToggleButton. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <string>
#include <imgui/imgui.h>
class ToggleButton
{
public:
	ToggleButton(const std::string & name , bool default_state,ImVec2 buttonsize, ImColor color_toggled , ImColor default_color);
	bool UpdateToggle();
	void SetSize(ImVec2 size);
	void SetToggle(bool toggle);
	bool GetToggle();
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
