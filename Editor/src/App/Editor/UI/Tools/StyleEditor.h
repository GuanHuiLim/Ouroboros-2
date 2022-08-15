/************************************************************************************//*!
\file          StyleEditorView.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution (100%)
\par           email: junxiang.leong\@digipen.edu
\date          November 12, 2021
\brief		   Allows user to edit their editor looks

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <imgui/imgui.h>
#include <string>
class StyleEditor
{
public:
	StyleEditor();
	~StyleEditor();
	void Show();
private:
	void MenuBar();
	void SaveStyle();
	void LoadStyle();
	bool SaveStylePopUp();
	std::string name;
	ImGuiStyle ref;
	ImGuiStyle ref_saved_style;
	char namebuffer[100];
	static ImGuiID m_styleeditor_popup;
};

