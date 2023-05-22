#include "pch.h"
#include "PreviewWindow.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "SceneManagement/include/SceneManager.h"
#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/EventSystem/EventManager.h"

PreviewWindow::PreviewWindow()
{
	oo::EventManager::Subscribe<PreviewWindow, GetPreviewWindowSizeEvent>(this, &PreviewWindow::GetPreviewWindowSize);
}

PreviewWindow::~PreviewWindow()
{
	oo::EventManager::Unsubscribe<PreviewWindow, GetPreviewWindowSizeEvent>(this, &PreviewWindow::GetPreviewWindowSize);
}

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
	graphicsworld->shouldRenderCamera[0] = true;
	ImTextureID imageid = graphicsworld->imguiID[0];
	if (imageid == 0)
	{
		ImVec2 cursorPosition = ImGui::GetCursorPos();
		ImVec2 imagesize = ImGui::GetContentRegionAvail();
		imagesize.x = imagesize.x * 0.5f;
		imagesize.y = imagesize.y * 0.5f;
		ImVec2 textSize = ImGui::CalcTextSize("No Camera Detected, Add a Camera Component");
		ImGui::SetCursorPos({ cursorPosition.x + imagesize.x - textSize.x * 0.5f,
							cursorPosition.y + imagesize.y - textSize.y * 0.5f });
		ImGui::Text("No Camera Detected, Add a Camera Component");
		return;
	}
	
	// Extra event for others to use
	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	PreviewWindowImageResizeEvent e;
	//e.StartPosition  = ImGui:://= { vMin.x, vMin.y};
	// Launch preview window resize event.

	float contentWidth = 0;
	float contentHeight = 0;
	float ar = graphicsworld->cameras[0].GetAspectRatio();
	ImVec2 imagesize = ImGui::GetContentRegionAvail();
	ImVec2 resize_by_ar = { imagesize.y * ar, imagesize.x / ar };
	ImVec2 cursorPosition = ImGui::GetCursorPos();
	if (imagesize.x > resize_by_ar.x)
	{
		cursorPosition.x = (imagesize.x - resize_by_ar.x) * 0.5f;
		ImGui::SetCursorPos(cursorPosition);
		ImGui::Image(imageid, { resize_by_ar.x, imagesize.y});
		contentWidth = resize_by_ar.x;
		contentHeight = imagesize.y;
	}
	else
	{
		float offset = (imagesize.y - resize_by_ar.y) * 0.5f;
		cursorPosition.y = ( (offset < cursorPosition.y) ? cursorPosition.y : offset);
		ImGui::SetCursorPos(cursorPosition);
		ImGui::Image(imageid, { imagesize.x, resize_by_ar.y });
		contentWidth = imagesize.x;
		contentHeight = resize_by_ar.y;
	}

	ImGui::SetCursorPos(cursorPosition);
	e.StartPosition = { ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y };
	e.Width = contentWidth;
	e.Height = contentHeight;
	//ImGui::GetForegroundDrawList()->AddRect(ImVec2{e.StartPosition.x, e.StartPosition.y}, ImVec2{e.StartPosition.x + e.Width, e.StartPosition.y + e.Height }, 0xFFFFFFFF);
	// blast event 
	oo::EventManager::Broadcast<PreviewWindowImageResizeEvent>(&e);

	if ( floorf( m_viewportWidth - contentWidth) != 0 || floorf(m_viewportHeight - contentHeight) != 0)
	{

		//resize viewport
		m_windowStartPosition = { vMin.x, vMin.y };
		m_viewportWidth = contentWidth;
		m_viewportHeight = contentHeight;

		PreviewWindowResizeEvent pwre;
		pwre.StartPosition = m_windowStartPosition;
		pwre.X = m_viewportWidth;
		pwre.Y = m_viewportHeight;
		oo::EventManager::Broadcast<PreviewWindowResizeEvent>(&pwre);
	}

	

}

void PreviewWindow::UpdateWhenNotShown()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto graphicsworld = scene->GetGraphicsWorld();
	graphicsworld->shouldRenderCamera[0] = false;
}

void PreviewWindow::GetPreviewWindowSize(GetPreviewWindowSizeEvent* e)
{
	e->StartPosition = m_windowStartPosition;
	e->Width = m_viewportWidth;
	e->Height = m_viewportHeight;
}
