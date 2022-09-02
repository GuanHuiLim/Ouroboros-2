#include "pch.h"

#include "ImGuiManager.h"
#include <Ouroboros/Asset/AssetManager.h>
#include <Ouroboros/Core/Application.h>
#include <Ouroboros/Vulkan/VulkanContext.h>

void ImGuiManager::InitAssetsAll()
{

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

		if (field.second.m_prewindow)
			field.second.m_prewindow();

		if (ImGui::Begin(field.first.c_str(), &field.second.m_enabled, field.second.m_flags) == false)
		{
			ImGui::End();
			continue;
		}
		field.second.m_UIupdate();
		ImGui::End();
	}
}

ImGuiObject& ImGuiManager::GetItem(const std::string& item)
{
	return s_GUIContainer.at(item);
}
