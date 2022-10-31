/************************************************************************************//*!
\file           ImGuiManager.cpp
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          Holds most UI updates and AssetManager and access to Important Controllers 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "ImGuiManager.h"

#include <Ouroboros/Core/Application.h>
#include <Ouroboros/Vulkan/VulkanContext.h>

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>

void ImGuiManager::InitAssetsAll()
{
	//will change to using filesystem later.
	//currently not all assets are required
	s_editorAssetManager.GetOrLoadDirectory("Log Icons");
	s_editorAssetManager.GetOrLoadDirectory("Generic Button Icons");
	s_editorAssetManager.GetOrLoadDirectory("Component Icons");
	s_editorAssetManager.GetOrLoadDirectory("File Icons");
	//ImGuiManager::s_editorAssetManager.LoadName("FolderIcon.png");
	//ImGuiManager::s_editorAssetManager.LoadName("GenericFileIcon.png");
}

void ImGuiManager::Create(const std::string name, const bool enabled,const ImGuiWindowFlags_ flag, std::function<void()> fnc, std::function<void()> pre_window)
{
	s_GUIContainer.emplace(name, ImGuiObject(enabled,flag,fnc, pre_window));
}

void ImGuiManager::UpdateAllUI()
{
	for (auto& field : s_GUIContainer)
	{
		if (field.second.m_enabled == false)
			continue;

		TRACY_PROFILE_SCOPE_NC(editor_ui_object, tracy::Color::BlueViolet);

		if (field.second.m_prewindow)
			field.second.m_prewindow();

		if (ImGui::Begin(field.first.c_str(), &field.second.m_enabled, field.second.m_flags) == false)
		{
			ImGui::End();
			
			TRACY_PROFILE_SCOPE_END();
			continue;
		}

		field.second.m_UIupdate();
		ImGui::End();

		TRACY_PROFILE_SCOPE_END();
	}
}
ImGuiObject& ImGuiManager::GetItem(const std::string& item)
{
	auto iter = s_GUIContainer.find(item);
	if (iter == s_GUIContainer.end())
		throw;
	return s_GUIContainer.at(item);
}