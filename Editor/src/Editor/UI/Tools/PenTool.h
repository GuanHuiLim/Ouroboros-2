/************************************************************************************//*!
\file          PenTool.h
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
#pragma once
#include <deque>
#include <imgui.h>
class PenTool
{
public:
	void Show();
private:
	std::deque<ImVec2> pos;
};
