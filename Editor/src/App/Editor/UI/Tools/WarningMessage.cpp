/************************************************************************************//*!
\file          WarningView.cpp
\project       Editor
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
#include "App/Editor/Utility/Windows_Utill.h"
void WarningMessage::Show()
{
	if (s_ShowWarning == false)
		return;
	//
	//if (ImGui::Begin("Warning Message", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs) == false)
	//{
	//	ImGui::End();
	//	return;
	//}
	ImGui::BeginTooltip();

	ImGui::PushTextWrapPos(300.0f);
	switch (s_dtype)
	{
	case DisplayType::DISPLAY_LOG:
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.8f,1.f,0.8f,1.f });
		ImGui::TextWrapped("%s", s_WarningMessage.c_str());
		ImGui::PopStyleColor();
		break;
	case DisplayType::DISPLAY_WARNING:
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.f,0.5f,0.5f,1.f });
		ImGui::TextWrapped("%s", s_WarningMessage.c_str());
		ImGui::PopStyleColor();
		break;
	case DisplayType::DISPLAY_ERROR:
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.f,0,0,1.f });
		ImGui::TextWrapped("%s", s_WarningMessage.c_str());
		ImGui::PopStyleColor();
		break;
	default:
		break;
	}
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
	//ImGui::End();
		
	s_counter -= oo::timer::dt();
	if (s_counter<0)
		s_ShowWarning = false;
}
void WarningMessage::DisplayWarning(DisplayType type ,const std::string& str,float time)
{
	switch (type)
	{
	case WarningMessage::DisplayType::DISPLAY_LOG:
		break;
	case WarningMessage::DisplayType::DISPLAY_WARNING:
		WindowsUtilities::Windows_Beep_Warn(); break;
	case WarningMessage::DisplayType::DISPLAY_ERROR:
		ASSERT_MSG(true, s_WarningMessage.c_str()); break;
	}

	
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
