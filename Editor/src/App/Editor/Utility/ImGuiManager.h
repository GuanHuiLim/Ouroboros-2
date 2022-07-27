#pragma once
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/imgui_internal.h>
#include <string>
#include <functional>
#include <unordered_map>
#include "ImGuiObject.h"
#include <SceneManagement/include/SceneManager.h>
class ImGuiManager
{
public:
	static void Create(const std::string name,const bool enabled,const ImGuiWindowFlags_ flag, std::function<void()> fnc, std::function<void()> pre_window = 0);
	static void UpdateAllUI();
	static ImGuiObject& GetItem(const std::string& item);
public:
	inline static std::unordered_map<std::string, ImGuiObject> s_GUIContainer;

	inline static const ImVec2 image_small = { 20,20 };
	inline static const ImVec2 image_medium = { 50,50 };

	inline static SceneManager const* s_scenemanager = nullptr;
};
