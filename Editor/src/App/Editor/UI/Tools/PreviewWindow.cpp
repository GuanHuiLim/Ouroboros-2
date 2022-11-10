#include "pch.h"
#include "PreviewWindow.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"
void PreviewWindow::Show()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto graphicsworld = scene->GetGraphicsWorld();
	ImTextureID imageid = graphicsworld->imguiID[1];
	if (imageid == 0)
		return;

	ImGui::Image(imageid, ImGui::GetContentRegionAvail());

}
