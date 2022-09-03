/************************************************************************************//*!
\file          PenTool.cpp
\project       Sandbox
\author        Leong Jun Xiang, junxiang.leong , 390007920
\par           email: junxiang.leong\@digipen.edu
\date          March 16, 2022
\brief          

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "PenTool.h"
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "Ouroboros/Core/Timer.h"
void PenTool::Show()
{
	ImGui::Text("Pen Tool Currently In Use!");
	ImDrawList* drawlist = ImGui::GetForegroundDrawList();

	if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		pos.clear();
	}
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		ImVec2 posMouse = ImGui::GetMousePos();
		ImVec2 previous = pos.empty() ? ImVec2(0,0):pos.back();
		float dotproduct = posMouse.x * posMouse.x + posMouse.y * posMouse.y;
		float dotprodct2 = previous.x * previous.x + previous.y * previous.y;
		if(fabsf(dotproduct - dotprodct2) > m_lineGranularity)
			pos.emplace_back(ImGui::GetMousePos());
		m_currTime_erase = m_timebefore_Erase;
	}
	else if (pos.size())
	{
		if (m_currTime_erase < 0)
			pos.pop_front();
		else
			m_currTime_erase -= oo::timer::dt();
	}
	
	if (pos.empty())
	{
		//ImGui::End();
		return;
	}
	ImVec2 prevpos = pos.front();
	for (const ImVec2 m_pos : pos)
	{
		drawlist->AddLine(prevpos, m_pos, 0xffffffff, 2.0f);
		prevpos = m_pos;

	}
}
