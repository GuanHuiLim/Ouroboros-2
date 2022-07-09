#pragma once
#include <string>
#include <functional>
#include <imgui.h>



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
