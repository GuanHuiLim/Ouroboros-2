#include "pch.h"

#include "ImGui_ToggleButton.h"


ToggleButton::ToggleButton(const std::string& name, bool default_state, ImVec2 buttonsize, ImColor color_toggled, ImColor default_color)
	:m_buttonName{ name },
	m_toggled{ default_state },
	m_buttonsize{buttonsize},
	m_toggled_color{ color_toggled },
	m_default_color{ default_color }
{
}

bool ToggleButton::UpdateToggle()
{
	if (m_toggled)
	{
		ImVec4 col = m_toggled_color.Value;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);//brightest col
		
		col.w *= 0.8f;//dimmer the col
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,col);
		
		col.w *= 0.8f;//dimmer the col
		ImGui::PushStyleColor(ImGuiCol_Button, col);//dimmest col
	}
	else
	{
		ImVec4 col = m_default_color.Value;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);//brightest col

		col.w *= 0.8f;//dimmer the col
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, col);

		col.w *= 0.8f;//dimmer the col
		ImGui::PushStyleColor(ImGuiCol_Button, col);//dimmest col
	}
	bool result = ImGui::Button(m_buttonName.c_str(), m_buttonsize);
	ImGui::PopStyleColor(3);

	if (result)
	{
		m_toggled = !m_toggled;
	}
	return result;
}



ColorButton::ColorButton(std::vector<std::string>&& names, std::vector<ImColor>&& colors, ImVec2 buttonsize, int start_index)
	:m_names{ names }, 
	m_colorList{ colors }, 
	m_buttonsize{ buttonsize }, 
	m_index{ start_index }
{

}

bool ColorButton::UpdateToggle()
{
	ImVec4 col = m_colorList[m_index].Value;
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, col);//brightest col

	col.w *= 0.8f;//dimmer the col
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, col);

	col.w *= 0.8f;//dimmer the col
	ImGui::PushStyleColor(ImGuiCol_Button, col);//dimmest col

	bool result = ImGui::Button(m_names[m_index].c_str(), m_buttonsize);
	ImGui::PopStyleColor(3);

	if (result)
	{
		++m_index;
		if ((size_t)m_index >= m_names.size())
			m_index = 0;
	}
	return result;
}
