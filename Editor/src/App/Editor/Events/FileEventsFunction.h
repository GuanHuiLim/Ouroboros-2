/************************************************************************************//*!
\file          FileEventsFunction.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Some of the events for opening a file related to OpenFileEvent.h


Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "App/Editor/Events/OpenFileEvent.h"
#include "App/Editor/Events/LoadSceneEvent.h"

#include "Ouroboros/EventSystem/EventManager.h"

#include <Windows.h>
#include <shellapi.h>



static void OpenAnimationEvent(OpenFileEvent* e)
{
	if (e->m_type != OpenFileEvent::FileType::ANIMATION)
		return;
	//EditorCallbacks::Open_AnimationFile(e->m_filepath);
}
static void OpenSpriteEvent(OpenFileEvent* e)
{
	if (e->m_type != OpenFileEvent::FileType::IMAGE)
		return;
	//SpriteEditorView::SetCurrentSprite(oo::AssetManager::GetAssetHandleFromFilePath(e->m_filepath));
}
//when u open folder trigger this event
static void OpenFolderEvent(OpenFileEvent* e)
{
	if (e->m_type != OpenFileEvent::FileType::FOLDER)
		return;
	//SpriteEditorView::SetCurrentSprite(oo::AssetManager::GetAssetHandleFromFilePath(e->m_filepath));
}
static void OpenOthersEvent(OpenFileEvent* e)
{
	if (e->m_type != OpenFileEvent::FileType::OTHERS)
		return;
	ShellExecuteA(NULL, "open", e->m_filepath.string().c_str(), NULL, NULL, SW_SHOW);
}