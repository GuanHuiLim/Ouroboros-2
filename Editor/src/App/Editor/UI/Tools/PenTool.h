/************************************************************************************//*!
\file          PenTool.h
\project       Sandbox
\author        Leong Jun Xiang, junxiang.leong , 390007920
\par           email: junxiang.leong\@digipen.edu
\date          March 16, 2022
\brief         Declarations for the pen tool

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <deque>
#include <imgui/imgui.h>
class PenTool
{
public:
	void Show();
private:
	float m_lineGranularity = 30.0f;
	float m_currTime_erase = 0;
	const float m_timebefore_Erase = 2.0f;
	std::deque<ImVec2> pos;
};
