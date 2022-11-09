/************************************************************************************//*!
\file           Editor.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          holds all editor UI's data 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "UI/Object Editor/Hierarchy.h"
#include "UI/Object Editor/Inspector.h"
#include "UI/Object Editor/FileBrowser.h"
#include "UI/Object Editor/ScriptSequencer.h"
#include "UI/Object Editor/EditorViewport.h"

#include "UI/Tools/StyleEditor.h"
#include "UI/Tools/WarningMessage.h"
#include "UI/Tools/LoggingView.h"
#include "UI/Tools/AnimatorControllerView.h"
#include "UI/Tools/AnimationTimelineView.h"
#include "UI/Tools/Toolbar.h"
#include "UI/Tools/PenTool.h"
#include "UI/Tools/InputManagerUI.h"
#include "UI/Tools/MeshHierarchy.h"
#include "UI/Tools/RendererDebugger.h"

#include "UI/Optional Windows/SceneOrderingWindow.h"

#include "App/Editor/Networking/ChatSystem.h"

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
	AnimatorControllerView m_animatorControllerView;
	AnimationTimelineView m_animationTimelineView;
	PenTool m_pentool;
	InputManagerUI m_inputManager;
	MeshHierarchy m_meshHierarchy;
	RendererDebugger m_rendererDebugger;

	ChatSystem m_chatsystem;

	SceneOrderingWindow m_sceneOderingWindow;
public:
	PopupHelperWindow helper;
	struct TimedSequence
	{
		TimedSequence(std::function<void()>ins, float _duration):
			max_duration{ _duration },
			curr_duration{ _duration },
			instruction{ ins } {};
		std::function<void(void)> instruction;
		const float max_duration = 0;
		float curr_duration = 0;
	};
	void AddSequence(TimedSequence&& seq);
private:
	std::vector<TimedSequence> m_timedseq;
	void TimedUpdate();
};

