#pragma once
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_internal.h>
#include <string>
#include <functional>
#include <unordered_map>
#include "ImGuiObject.h"
class ImGuiManager
{
public:
	static void Create(const std::string name,const bool enabled,const ImGuiWindowFlags_ flag, std::function<void()> fnc);
	static void UpdateAllUI();
	static ImGuiObject& GetItem(const std::string& item);
public:
	inline static std::unordered_map<std::string, ImGuiObject> s_GUIContainer;

	inline static const ImVec2 image_small = { 20,20 };
	inline static const ImVec2 image_medium = { 50,50 };
	
};
