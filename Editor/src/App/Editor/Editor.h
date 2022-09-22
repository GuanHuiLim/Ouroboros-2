#pragma once
#include "UI/Object Editor/Hierarchy.h"
#include "UI/Object Editor/Inspector.h"
#include "UI/Object Editor/FileBrowser.h"
#include "UI/Object Editor/ScriptSequencer.h"
#include "UI/Object Editor/EditorViewport.h"

#include "UI/Tools/StyleEditor.h"
#include "UI/Tools/WarningMessage.h"
#include "UI/Tools/LoggingView.h"
#include "UI/Tools/Toolbar.h"
#include "UI/Tools/PenTool.h"
#include "UI/Tools/InputManagerUI.h"

#include "UI/Optional Windows/SceneOrderingWindow.h"

#include "App/Editor/Events/OpenPromtEvent.h"
#include "App/Editor/Events/LoadProjectEvents.h"
#include "App/Editor/Events/OpenFileEvent.h"

class PopupHelperWindow
{
public:
	PopupHelperWindow();
	void Popups();
	OpenPromptEvent<CloseProjectEvent>::OpenPromptAction eventAfterPrompt;
	OpenPromptEvent<OpenFileEvent>::OpenPromptAction eventAfterPrompt_ofe;
private:
	bool closeproject = false;
	bool openfile = false;
	void CloseProjectEvent_EventReceiver(OpenPromptEvent<CloseProjectEvent>* e);
	void OpenFileEvent_EventReceiver(OpenPromptEvent<OpenFileEvent>* e);
private:
	void CloseProjectPopup();
	void OpenFilePopup();
};

class Editor
{
public:
	Editor();
	~Editor();
	void Update();
	static void MenuBar();

	Hierarchy m_hierarchy;
	Inspector m_inspector;
	FileBrowser m_fileBrowser;
	ScriptSequencer m_scriptSequencer;
	EditorViewport m_EditorViewport;

	StyleEditor m_styleEditor;
	WarningMessage m_warningMessage;
	Toolbar m_toolbar;
	LoggingView m_loggingView;
	PenTool m_pentool;
	InputManagerUI m_inputManager;

	SceneOrderingWindow m_sceneOderingWindow;
public:
	PopupHelperWindow helper;
private:


};

