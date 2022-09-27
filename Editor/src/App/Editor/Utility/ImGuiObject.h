/************************************************************************************//*!
\file           ImGuiObject.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          wrapper for creating a UI 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <string>
#include <functional>
#include <imgui/imgui.h>



class ImGuiObject
{
public:
	ImGuiObject(const bool enable,const ImGuiWindowFlags_ flag,std::function<void()>fnc,std::function<void()>pre_window = 0)
		:m_enabled{ enable }, m_flags{ flag }, m_UIupdate{ fnc }, m_prewindow{ pre_window } {};
	std::function<void()> m_UIupdate;
	std::function<void()> m_prewindow;
	ImGuiWindowFlags_ m_flags;
	bool m_enabled = true;
private:
};
