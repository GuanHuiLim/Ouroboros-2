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
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto graphicsworld = scene->GetGraphicsWorld();
	ImTextureID imageid = graphicsworld->imguiID[0];
	if (imageid == 0)
		return;
	
	// Launch preview window resize event.
	{
		ImVec2 vMin = ImGui::GetWindowContentRegionMin();
		ImVec2 vMax = ImGui::GetWindowContentRegionMax();

		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		ImVec2 vpDim = { vMax.x - vMin.x ,vMax.y - vMin.y };

		auto contentWidth = vpDim.x;
		auto contentHeight = vpDim.y;

		if (m_viewportWidth != contentWidth || m_viewportHeight != contentHeight)
		{
			//resize viewport
			m_viewportWidth = contentWidth;
			m_viewportHeight = contentHeight;

			PreviewWindowResizeEvent e;
			e.X = m_viewportWidth;
			e.Y = m_viewportHeight;
			oo::EventManager::Broadcast<PreviewWindowResizeEvent>(&e);
		}
	}

	ImGui::Image(imageid, ImGui::GetContentRegionAvail());

}

void PreviewWindow::GetPreviewWindowSize(GetPreviewWindowSizeEvent* e)
{
	e->Width = m_viewportWidth;
	e->Height = m_viewportHeight;
}
