#include "pch.h"

#include "ImGuiManager.h"

#include <Ouroboros/Core/Application.h>
#include <Ouroboros/Vulkan/VulkanContext.h>

void ImGuiManager::InitAssetsAll()
{
	auto vc = reinterpret_cast<oo::VulkanContext*>(oo::Application::Get().GetWindow().GetRenderingContext());
	auto vr = vc->getRenderer();
	LoadHelper("Log Icons/ErrorIcon.png",vr);
	LoadHelper("Log Icons/LogsIcon.png",vr);
	LoadHelper("Log Icons/WarningIcon.png",vr);
	LoadHelper("Generic Button Icons/GridIcon.png",vr);
	LoadHelper("Generic Button Icons/ListIcon.png",vr);
	LoadHelper("Generic Button Icons/LockButton.png", vr);
	LoadHelper("Generic Button Icons/PauseButton.png", vr);
	LoadHelper("Generic Button Icons/PlayButton.png", vr);
	LoadHelper("Generic Button Icons/RotateButton.png", vr);
	LoadHelper("Generic Button Icons/ScaleButton.png", vr);
	LoadHelper("Generic Button Icons/SearchButton.png", vr);
	LoadHelper("Generic Button Icons/StopButton.png", vr);
	LoadHelper("Generic Button Icons/TranslateButton.png", vr);
	LoadHelper("File Icons/AnimationClipIcon.png", vr);
	LoadHelper("File Icons/AnimatorIcon.png", vr);
	LoadHelper("File Icons/AudioFileIcon.png", vr);
	LoadHelper("File Icons/FBXFileIcon.png", vr);
	LoadHelper("File Icons/FolderIcon.png", vr);
	LoadHelper("File Icons/GenericFileIcon.png", vr);
	LoadHelper("File Icons/MP3FileIcon.png", vr);
	LoadHelper("File Icons/OBJFileIcon.png", vr);
	LoadHelper("File Icons/CSFileIcon.png", vr);
	LoadHelper("File Icons/PNGIcon.png", vr);
	LoadHelper("File Icons/PrefabIcon.png", vr);
	LoadHelper("File Icons/SceneIcon.png", vr);
	LoadHelper("Component Icons/AnimationComponent.png", vr);
	LoadHelper("Component Icons/AudioComponent.png", vr);
	LoadHelper("Component Icons/CameraComponent.png", vr);
	LoadHelper("Component Icons/CollisionComponent.png", vr);
	LoadHelper("Component Icons/GUIComponent.png", vr);
	LoadHelper("Component Icons/ParticleComponent.png", vr);
	LoadHelper("Component Icons/PathfindingComponent.png", vr);
	LoadHelper("Component Icons/RigidbodyComponent.png", vr);
	LoadHelper("Component Icons/ScriptComponent.png", vr);
	LoadHelper("Component Icons/SpriteComponent.png", vr);
	LoadHelper("Component Icons/TransformComponent.png", vr);
	LoadHelper("Component Icons/UILayerComponent.png", vr);
	LoadHelper("Component Icons/UITextComponent.png", vr);
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
	auto iter = s_GUIContainer.find(item);
	if (iter == s_GUIContainer.end())
		throw;
	return s_GUIContainer.at(item);
}

void ImGuiManager::LoadHelper(const std::filesystem::path& fp, void* renderer)
{
	try
	{
		auto num = s_editorAssetManager.LoadPath(fp).GetData<uint32_t>();
		std::string name = fp.stem().string();
		LOG_CORE_ERROR(name);
		
		if (s_EditorIcons.find(name) != s_EditorIcons.end())
			s_EditorIcons[name] = reinterpret_cast<ImTextureID>(reinterpret_cast<VulkanRenderer*>(renderer)->GetImguiID(num));
		
		s_EditorIcons.emplace(name, reinterpret_cast<ImTextureID>(reinterpret_cast<VulkanRenderer*>(renderer)->GetImguiID(num)));
	}
	catch (...)
	{
		LOG_CORE_ERROR("ASSET NOT FOUND");
	}
}
