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