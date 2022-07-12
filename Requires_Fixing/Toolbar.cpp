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
#include "Toolbar.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <iostream>
//#include "Scene/SceneManager.h"
//#include "Ouroboros/Scripting/ScriptSystem.h"

//#include "Editor/Editor.h"
//#include "Ouroboros/Asset/AssetsManager.h"

//#include <Editor/Views/EditorViewport.h>
#include "Ouroboros/Core/Timer.h"

#include "Ouroboros/EventSystem/EventManager.h"
//#include "Editor/Events/ButtonEvents.h"
#include <shellapi.h>

#include "Editor/Utility/ImGui_Utill.h"

void Toolbar::Show()
{
	float w = ImGui::GetWindowWidth();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0,0 });
	{
		ImGui::BeginChild("ChildToolbar", { 0,0 });
		
		if (ImGuiUtilities::ImageButton_ToolTip(1,"Gizmo Translate Mode", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_Translate_Icon_Black")->Get_IMTEXTURE_ID(),
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(EditorViewport::GetOperation() == ImGuizmo::OPERATION::TRANSLATE) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			EditorCallbacks::GuizmoMode(ImGuizmo::TRANSLATE);
		}

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(2,"Gizmo Rotate Mode", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_Rotate_Icon_Black")->Get_IMTEXTURE_ID(),
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(EditorViewport::GetOperation() == ImGuizmo::OPERATION::ROTATE) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			EditorCallbacks::GuizmoMode(ImGuizmo::ROTATE);
		}

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(3, "Gizmo Scale Mode", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_Scale_Icon_Black")->Get_IMTEXTURE_ID(),
			{ btn_width,btn_height }, { 0,0 }, { 1,1 }, -1,
			(EditorViewport::GetOperation() == ImGuizmo::OPERATION::SCALE) ? ImVec4{ 0.7f, 0.0f, 0, 1 } : ImVec4{ 0,0,0,0 }))
		{
			EditorCallbacks::GuizmoMode(ImGuizmo::SCALE);
		}


		ImGui::SameLine();
		if (ImGui::Button("Compile", { 0,btn_height }))
		{
			oo::ScriptSystem::Compile();
		}
		if (ImGui::IsItemHovered())
			WarningView::DisplayToolTip("Compiles C# scripts");

		ImGui::EndChild();
		ImGui::SameLine(w * 0.5f - (btn_width * 3 * 0.5f));
	}
	{
		ImGui::BeginChild("ChildToolbar2", { 0,0 });
		if (ImGuiUtilities::ImageButton_ToolTip(4, "Start Simulation", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_Play_White")->Get_IMTEXTURE_ID(), {btn_width,btn_height}))
		{
			GenericButtonEvents e = { Buttons::PLAY_BUTTON };
			oo::EventManager::Broadcast(&e);
		};

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(5, "Pause/Next frame", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_Pause_White")->Get_IMTEXTURE_ID(), { btn_width,btn_height }))
		{
			GenericButtonEvents e = { Buttons::PAUSE_BUTTON };
			oo::EventManager::Broadcast(&e);
		};

		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(6, "Stop Simulation", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_Stop_White")->Get_IMTEXTURE_ID(), { btn_width,btn_height }))
		{
			GenericButtonEvents e = { Buttons::STOP_BUTTON };
			oo::EventManager::Broadcast(&e);
		};

		ImGui::EndChild(); 
	}
	{
		ImGui::SameLine(w - (btn_width * 5));
		ImGui::BeginChild("ChildToolbar3", { 0,0 });
		if (ImGuiUtilities::ImageButton_ToolTip(7, "Undocks the toolbar", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_Lock_Icon_White")->Get_IMTEXTURE_ID(),
			{ btn_width,btn_height }))
		{
			docking = !docking;
		}


		ImGui::SameLine();
		if (ImGuiUtilities::ImageButton_ToolTip(8, "Pen Tool", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_UI_Component_White")->Get_IMTEXTURE_ID(),
			{ btn_width,btn_height }))
		{
			GUIglobals::s_EditorState.values.t_pentool = !GUIglobals::s_EditorState.values.t_pentool;
		}

		ImGui::SameLine();
		
		if (ImGuiUtilities::ImageButton_ToolTip(9, "Open Calculator", oo::AssetManager::GetInternalAsset<oo::Texture>("Ouroboros_UI_Component_White")->Get_IMTEXTURE_ID(),
			{ btn_width,btn_height }))
		{
			ShellExecuteA(0, 0, "calculator:\\", 0, 0, SW_SHOW);
		}

		ImGui::EndChild();
	}
	ImGui::PopStyleVar();
	ImGui::End();

	auto* window = ImGui::FindWindowByName(GUIglobals::s_ToolbarView_Name);
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
