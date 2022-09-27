/************************************************************************************//*!
\file          StyleEditorView.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution (100%)
\par           email: junxiang.leong\@digipen.edu
\date          November 12, 2022
\brief		   Allows user to edit their editor looks

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <imgui/imgui.h>
#include <string>
#include "App/Editor/Events/ToolbarButtonEvent.h"
class StyleEditor
{
public:
	void SetPlayMode(ToolbarButtonEvent* e);
	void SetEditMode(ToolbarButtonEvent* e);
	void InitStyle();
	StyleEditor();
	~StyleEditor();
	void Show();
	static constexpr const char* const defaultStyleName = "EditorMode\0";
	static constexpr const char* const playStyleName = "PlayMode\0";
private:
	void MenuBar();
	void SaveStyle();
	void LoadStyle();
	bool SaveStylePopUp();
	std::string name;
	ImGuiStyle ref;
	ImGuiStyle ref_saved_style;
	char namebuffer[100] = "EditorMode\0";
	static ImGuiID m_styleeditor_popup;
};

