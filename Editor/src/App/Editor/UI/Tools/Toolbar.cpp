/************************************************************************************//*!
\file          ToolbarView.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution (100%)
\par           email: junxiang.leong\@digipen.edu
\date          October 3, 2021
\brief         UI for the top bar which contains the Play Pasue and Stop button

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Toolbar.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

//#include "Scene/SceneManager.h"
//#include "Ouroboros/Scripting/ScriptSystem.h"

//#include "Editor/Editor.h"
//#include "Ouroboros/Asset/AssetsManager.h"

//#include <Editor/Views/EditorViewport.h>
#include "Ouroboros/Core/Timer.h"

#include "Ouroboros/EventSystem/EventManager.h"
//#include "Editor/Events/ButtonEvents.h"
#include <Windows.h>
#include <shellapi.h>
#include "App/Editor/Utility/ImGui_Utill.h"
#include "App/Editor/UI/Tools/WarningMessage.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "App/Editor/Events/ToolbarButtonEvent.h"

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>

void Toolbar::InitAssets()
{
	m_iconsSaved.emplace("TranslateButton", *ImGuiManager::s_editorAssetManager.LoadName("TranslateButton.png").begin());
	m_iconsSaved.emplace("RotateButton", *ImGuiManager::s_editorAssetManager.LoadName("RotateButton.png").begin());
	m_iconsSaved.emplace("ScaleButton", *ImGuiManager::s_editorAssetManager.LoadName("ScaleButton.png").begin());
	m_iconsSaved.emplace("PlayButton", *ImGuiManager::s_editorAssetManager.LoadName("PlayButton.png").begin());
	m_iconsSaved.emplace("PauseButton", *ImGuiManager::s_editorAssetManager.LoadName("PauseButton.png").begin());
	m_iconsSaved.emplace("StopButton", *ImGuiManager::s_editorAssetManager.LoadName("StopButton.png").begin());
	m_iconsSaved.emplace("LockButton", *ImGuiManager::s_editorAssetManager.LoadName("LockButton.png").begin());
	m_iconsSaved.emplace("ListIcon", *ImGuiManager::s_editorAssetManager.LoadName("ListIcon.png").begin());
	m_iconsSaved.emplace("GridIcon", *ImGuiManager::s_editorAssetManager.LoadName("GridIcon.png").begin());

}
void Toolbar::Show()
{
	float w = ImGui::GetWindowWidth();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
	{
		ImGui::BeginChild("ChildToolbar", { 0,0 });

		TRACY_PROFILE_SCOPE_NC(ImageButton, tracy::Color::Blue);
		auto data = m_iconsSaved["TranslateButton"].GetData<ImTextureID>();
		if (ImGuiUtilities::ImageButton_ToolTip(1,"Gizmo Translate Mode",
			m_iconsSaved["TranslateButton"].GetData<ImTextureID>(),
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(/*EditorViewport::GetOperation() == ImGuizmo::OPERATION::TRANSLATE*/1) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::TRANSFORM);
			oo::EventManager::Broadcast(&tbe);
		}

		TRACY_PROFILE_SCOPE_END();

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(2,"Gizmo Rotate Mode", 
			m_iconsSaved["RotateButton"].GetData<ImTextureID>(),
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(/*EditorViewport::GetOperation() == ImGuizmo::OPERATION::ROTATE*/1) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::ROTATE);
			oo::EventManager::Broadcast(&tbe);
		}

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(3, "Gizmo Scale Mode",
			m_iconsSaved["ScaleButton"].GetData<ImTextureID>(),
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(/*EditorViewport::GetOperation() == ImGuizmo::OPERATION::SCALE*/1) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::SCALE);
			oo::EventManager::Broadcast(&tbe);
		}


		ImGui::SameLine();
		if (ImGui::Button("Compile", { 0,btn_height }))
		{
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::COMPILE);
			oo::EventManager::Broadcast(&tbe);
		}
		if (ImGui::IsItemHovered())
			WarningMessage::DisplayToolTip("Compiles C# scripts");

		ImGui::EndChild();
		ImGui::SameLine(w * 0.5f - (btn_width * 3 * 0.5f));
	}
	{
		ImGui::BeginChild("ChildToolbar2", { 0,0 });
		if (ImGuiUtilities::ImageButton_ToolTip(4, "Start Simulation", 
			m_iconsSaved["PlayButton"].GetData<ImTextureID>(),
			{btn_width,btn_height}))
		{
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::PLAY);
			oo::EventManager::Broadcast(&tbe);
		};

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(5, "Pause/Next frame",
			m_iconsSaved["PauseButton"].GetData<ImTextureID>(),
			{ btn_width,btn_height }))
		{
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::PAUSE);
			oo::EventManager::Broadcast(&tbe);
		};

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(6, "Stop Simulation",
			m_iconsSaved["StopButton"].GetData<ImTextureID>(),
			{ btn_width,btn_height }))
		{
			ToolbarButtonEvent tbe(ToolbarButtonEvent::ToolbarButton::STOP);
			oo::EventManager::Broadcast(&tbe);
		};

		ImGui::EndChild(); 
	}
	{
		ImGui::SameLine(w - (btn_width * 5));
		ImGui::BeginChild("ChildToolbar3", { 0,0 });
		if (ImGuiUtilities::ImageButton_ToolTip(7, "Undocks the toolbar",
			m_iconsSaved["LockButton"].GetData<ImTextureID>(),
			{ btn_width,btn_height }))
		{
			docking = !docking;
		}


		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(8, "Pen Tool", 
			m_iconsSaved["ListIcon"].GetData<ImTextureID>(),
			{ btn_width,btn_height }))
		{
			try
			{
				auto& item = ImGuiManager::GetItem("PenTool");
				item.m_enabled = !item.m_enabled;
			}
			catch (...)
			{
				WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_ERROR,"Pen Tool Not Found!");
			}
			//GUIglobals::s_EditorState.values.t_pentool = !GUIglobals::s_EditorState.values.t_pentool;
		}

		ImGui::SameLine();
		
		if (ImGuiUtilities::ImageButton_ToolTip(9, "Open Calculator",
			m_iconsSaved["GridIcon"].GetData<ImTextureID>(),
			{ btn_width,btn_height }))
		{
			ShellExecute(0, 0, L"calculator:\\", 0, 0, SW_SHOW);
		}

		ImGui::EndChild();
	}
	ImGui::PopStyleVar();


	auto* window = ImGui::FindWindowByName("Toolbar");
	if (window->DockNode)
	{
		if (docking)
			window->DockNode->LocalFlags = 0;
		else
			window->DockNode->LocalFlags = ImGuiDockNodeFlags_NoTabBar|ImGuiDockNodeFlags_NoResizeY | ImGuiDockNodeFlags_NoDocking;
	}
	else
		ImGui::SetWindowSize(window,{ 500,100 });

}
