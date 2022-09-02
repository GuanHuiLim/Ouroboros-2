/************************************************************************************//*!
\file          ToolbarView.cpp
\project       Ouroboros
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
void Toolbar::Show()
{
	float w = ImGui::GetWindowWidth();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
	{
		ImGui::BeginChild("ChildToolbar", { 0,0 });
		
		if (ImGuiUtilities::ImageButton_ToolTip(1,"Gizmo Translate Mode", ImGuiManager::s_EditorIcons["TranslateButton"],
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(/*EditorViewport::GetOperation() == ImGuizmo::OPERATION::TRANSLATE*/1) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			//EditorCallbacks::GuizmoMode(ImGuizmo::TRANSLATE);
		}

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(2,"Gizmo Rotate Mode", ImGuiManager::s_EditorIcons["RotateButton"],
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(/*EditorViewport::GetOperation() == ImGuizmo::OPERATION::ROTATE*/1) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			//EditorCallbacks::GuizmoMode(ImGuizmo::ROTATE);
		}

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(3, "Gizmo Scale Mode", ImGuiManager::s_EditorIcons["ScaleButton"],
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(/*EditorViewport::GetOperation() == ImGuizmo::OPERATION::SCALE*/1) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			//EditorCallbacks::GuizmoMode(ImGuizmo::SCALE);
		}


		ImGui::SameLine();
		if (ImGui::Button("Compile", { 0,btn_height }))
		{
			//oo::ScriptSystem::Compile();
		}
		if (ImGui::IsItemHovered())
			WarningMessage::DisplayToolTip("Compiles C# scripts");

		ImGui::EndChild();
		ImGui::SameLine(w * 0.5f - (btn_width * 3 * 0.5f));
	}
	{
		ImGui::BeginChild("ChildToolbar2", { 0,0 });
		if (ImGuiUtilities::ImageButton_ToolTip(4, "Start Simulation", ImGuiManager::s_EditorIcons["PlayButton"], {btn_width,btn_height}))
		{
			//GenericButtonEvents e = { Buttons::PLAY_BUTTON };
			//oo::EventManager::Broadcast(&e);
		};

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(5, "Pause/Next frame", ImGuiManager::s_EditorIcons["PauseButton"], { btn_width,btn_height }))
		{
			//GenericButtonEvents e = { Buttons::PAUSE_BUTTON };
			//oo::EventManager::Broadcast(&e);
		};

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(6, "Stop Simulation", ImGuiManager::s_EditorIcons["StopButton"], { btn_width,btn_height }))
		{
			//GenericButtonEvents e = { Buttons::STOP_BUTTON };
			//oo::EventManager::Broadcast(&e);
		};

		ImGui::EndChild(); 
	}
	{
		ImGui::SameLine(w - (btn_width * 5));
		ImGui::BeginChild("ChildToolbar3", { 0,0 });
		if (ImGuiUtilities::ImageButton_ToolTip(7, "Undocks the toolbar", ImGuiManager::s_EditorIcons["LockButton"],
			{ btn_width,btn_height }))
		{
			docking = !docking;
		}


		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(8, "Pen Tool", ImGuiManager::s_EditorIcons["ListIcon"],
			{ btn_width,btn_height }))
		{
			//GUIglobals::s_EditorState.values.t_pentool = !GUIglobals::s_EditorState.values.t_pentool;
		}

		ImGui::SameLine();
		
		if (ImGuiUtilities::ImageButton_ToolTip(9, "Open Calculator", ImGuiManager::s_EditorIcons["GridIcon"],
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
