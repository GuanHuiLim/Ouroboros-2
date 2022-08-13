/************************************************************************************//*!
\file          WarningView.cpp
\project       Ouroboros
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution (100%)
\par           email: junxiang.leong\@digipen.edu
\date          October 3, 2022
\brief         Displays a warning when the user does an unexpected move 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "WarningMessage.h"

#include <imgui/imgui.h>
//#include <Editor/GUIglobals.h>
#include "Ouroboros/Core/Timer.h"
#include "Ouroboros/Core/Assert.h"
void WarningMessage::Show()
{
	if (s_ShowWarning == false)
		return;
	
	if (ImGui::Begin("Warning Message", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs) == false)
	{
		ImGui::End();
		return;
	}
	ImGui::BeginTooltip();

	switch (s_dtype)
	{
	case DisplayType::DISPLAY_LOG:
		ImGui::TextColored({ 0.8,1,0.8,1 }, "****Log****");
		ImGui::TextWrapped("%s", s_WarningMessage.c_str());
		break;
	case DisplayType::DISPLAY_WARNING:
		ImGui::TextColored({ 1,0.5,0.5,1 }, "****Warning****");
		ImGui::TextWrapped("%s", s_WarningMessage.c_str());
		break;
	case DisplayType::DISPLAY_ERROR:
		ImGui::TextColored({ 1,0,0,1 }, "****Error****");
		ImGui::TextWrapped("%s", s_WarningMessage.c_str());
		break;
	default:
		break;
	}
	ImGui::EndTooltip();
	ImGui::End();
		
	s_counter -= oo::timer::dt();
	if (s_counter<0)
		s_ShowWarning = false;
}
void WarningMessage::DisplayWarning(DisplayType type ,const std::string& str,float time)
{
	if(type == DisplayType::DISPLAY_ERROR)//should cause a debug break when debugging
		ASSERT_MSG(true, s_WarningMessage.c_str());

	s_dtype = type;
	s_WarningMessage = str;
	s_ShowWarning = true;
	s_counter = time;
	s_position[0] = ImGui::GetMousePos().x;
	s_position[1] = ImGui::GetMousePos().y;
}

void WarningMessage::DisplayToolTip(const std::string& str)
{
	constexpr float toolTipTime = 0.3f;
	static float time = 0;

	if (ImGui::IsItemHovered())
	{
		time += oo::timer::dt();
		if (time < toolTipTime)
			return;
		DisplayWarning(DisplayType::DISPLAY_LOG, str,0.31f);
		time = 0;
	}
}

void WarningMessage::PreImGuiWindowOperation()
{
	ImGui::SetNextWindowPos({ s_position[0],s_position[1] });
	ImGui::SetNextWindowBgAlpha(0);
}
