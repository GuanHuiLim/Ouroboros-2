#pragma once
#include "UI/Tools/StyleEditor.h"
#include "UI/Tools/WarningMessage.h"
#include "UI/Object Editor/Hierarchy.h"
#include "UI/Object Editor/Inspector.h"
#include "UI/Object Editor/FileBrowser.h"
class Editor
{
public:
	Editor();
	~Editor();
	void Update();
	static void MenuBar();

	StyleEditor m_styleEditor;
	WarningMessage m_warningMessage;
	Hierarchy m_hierarchy;
	Inspector m_inspector;
	FileBrowser m_fileBrowser;
private:
};

