/************************************************************************************//*!
\file           ImGuiManager.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          one stop access point for all Editor Stuffs 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui/imgui_internal.h>
#include <string>
#include <functional>
#include <unordered_map>
#include <filesystem>
#include "ImGuiObject.h"
#include <Ouroboros/Asset/AssetManager.h>

namespace oo
{
	class PrefabSceneController;
	class RuntimeController;
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
	inline static SceneManager const* s_scenemanager = nullptr;
	inline static oo::PrefabSceneController * s_prefab_controller = nullptr;
	inline static oo::RuntimeController* s_runtime_controller = nullptr;
	inline static oo::AssetManager s_editorAssetManager = oo::AssetManager("./icons");

private:
};
