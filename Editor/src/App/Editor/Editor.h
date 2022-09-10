#pragma once
#include "UI/Tools/StyleEditor.h"
#include "UI/Tools/WarningMessage.h"
#include "UI/Object Editor/Hierarchy.h"
#include "UI/Object Editor/Inspector.h"
#include "UI/Object Editor/FileBrowser.h"
#include "UI/Tools/Toolbar.h"
#include "App/Editor/UI/Tools/LoggingView.h"
#include "UI/Tools/PenTool.h"

#include "App/Editor/Events/OpenPromtEvent.h"
#include "App/Editor/Events/LoadProjectEvents.h"

class PopupHelperWindow
{
public:
	PopupHelperWindow();
	void Popups();
	OpenPromptEvent<CloseProjectEvent>::OpenPromptAction eventAfterPrompt;
private:
	bool closeproject = false;
	void CloseProjectEvent_EventReceiver(OpenPromptEvent<CloseProjectEvent>* e);
};

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
	Toolbar m_toolbar;
	LoggingView m_loggingView;
	PenTool m_pentool;
public:
	PopupHelperWindow helper;
private:


};

