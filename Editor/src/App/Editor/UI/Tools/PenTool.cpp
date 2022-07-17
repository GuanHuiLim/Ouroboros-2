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

void PenTool::Show()
{
	ImDrawList* drawlist = ImGui::GetForegroundDrawList();

	if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		pos.clear();
	}
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
		pos.emplace_back(ImGui::GetMousePos());
	else if (pos.size())
		pos.pop_front();
	
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
