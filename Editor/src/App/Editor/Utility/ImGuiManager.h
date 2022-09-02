#pragma once
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/imgui_internal.h>
#include <string>
#include <functional>
#include <unordered_map>
#include "ImGuiObject.h"

namespace oo
{
	class AssetManager;
	class PrefabSceneController;
};
class SceneManager;

class ImGuiManager
{
public:
	static void InitAssetsAll();
	static void Create(const std::string name,const bool enabled,const ImGuiWindowFlags_ flag, std::function<void()> fnc, std::function<void()> pre_window = 0);
	static void UpdateAllUI();
	static ImGuiObject& GetItem(const std::string& item);
public:
	inline static std::unordered_map<std::string, ImGuiObject> s_GUIContainer;
	inline static std::unordered_map<std::string, ImTextureID> s_EditorIcons;
	inline static SceneManager const* s_scenemanager = nullptr;
	inline static oo::PrefabSceneController * s_prefab_controller = nullptr;
	inline static oo::AssetManager* s_assetManager = nullptr;
};
