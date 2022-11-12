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
	//if (ImGui::BeginMenuBar())
	//{
	//	if (ImGui::BeginMenu("Options"))
	//	{
	//		if (ImGui::MenuItem("Maximize On Play", nullptr, GUIglobals::s_MaximizeOnPlay))
	//		{
	//			GUIglobals::s_MaximizeOnPlay = !GUIglobals::s_MaximizeOnPlay;
	//		}
	//		ImGui::EndMenu();
	//	}
	//	ImGui::EndMenuBar();
	//}


	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto graphicsworld = scene->GetGraphicsWorld();
	ImTextureID imageid = graphicsworld->imguiID[1];
	if (imageid == 0)
		return;

	float ar = graphicsworld->cameras[1].GetAspectRatio();
	ImVec2 imagesize = ImGui::GetContentRegionAvail();
	ImVec2 resize_by_ar = { imagesize.y * ar, imagesize.x * ar };
	ImVec2 cursorPosition = ImGui::GetCursorPos();
	if (imagesize.x > resize_by_ar.x)
	{
		cursorPosition.x = (imagesize.x - resize_by_ar.x) * 0.5f;
		ImGui::SetCursorPos(cursorPosition);
		ImGui::Image(imageid, { resize_by_ar.x, imagesize.y});
	}
	else
	{
		float offset = (imagesize.y - resize_by_ar.y) * 0.5f;
		cursorPosition.y = ( (offset > cursorPosition.y) ? cursorPosition.y : offset);
		ImGui::SetCursorPos(cursorPosition);
		ImGui::Image(imageid, { imagesize.x, resize_by_ar.y });
	}


	

}
