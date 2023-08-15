/************************************************************************************//*!
\file           SceneManagerAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the scene manager, like getting the currently active scene,
                getting specific scene information, like its name, and transitioning into a new scene

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"

#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Scripting/ScriptManager.h"

#include <SceneManagement/include/SceneManager.h>
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Scene/RuntimeController.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* C# SceneManager                                                             */
    /*-----------------------------------------------------------------------------*/
    //SCRIPT_API void SceneManager_LoadSceneByName(const char* sceneName)
    //{
    //    RuntimeController::ChangeRuntimeScene(sceneName);
    //}

    //SCRIPT_API void SceneManager_LoadSceneByBuildIndex(int buildIndex)
    //{
    //    RuntimeController::ChangeRuntimeScene(buildIndex);
    //}

    SCRIPT_API Scene::ID_type SceneManager_GetActiveScene()
    {
        return ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetID();
    }

    SCRIPT_API bool SceneManager_LoadSceneByIndex(Scene::ID_type sceneID)
    {
        return ScriptManager::s_SceneManager->ChangeScene(sceneID);
    }
    SCRIPT_API bool SceneManager_LoadSceneByName(const char* sceneName)
    {
        return ScriptManager::s_SceneManager->ChangeScene(sceneName);
    }

    SCRIPT_API ScriptDatabase::IntPtr SceneManager_PreloadSceneByIndex(Scene::ID_type sceneID)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        AssetManager::LoadProgressPtr ptr = Serializer::PreloadScene(*scene);
        return ScriptManager::TrackLoadingProgress(ptr);
    }

    SCRIPT_API ScriptDatabase::IntPtr SceneManager_PreloadSceneByName(const char* sceneName)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneName);
        AssetManager::LoadProgressPtr ptr = Serializer::PreloadScene(*scene);
        return ScriptManager::TrackLoadingProgress(ptr);
    }

    /*-----------------------------------------------------------------------------*/
    /* C# Scene                                                                    */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API ScriptDatabase::IntPtr Scene_GetName(Scene::ID_type sceneID)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        std::string const& name = scene->GetSceneName();
        MonoString* string = ScriptEngine::CreateString(name.c_str());
        return mono_gchandle_new((MonoObject*)string, false);
    }

    SCRIPT_API ScriptDatabase::IntPtr Scene_GetPath(Scene::ID_type sceneID)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        std::string const& name = scene->GetFilePath();
        MonoString* string = ScriptEngine::CreateString(name.c_str());
        return mono_gchandle_new((MonoObject*)string, false);
    }

    SCRIPT_API bool Scene_IsLoaded(Scene::ID_type sceneID)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        return scene->IsLoaded();
    }

    SCRIPT_API bool Scene_IsValid(Scene::ID_type sceneID)
    {
        return ScriptManager::s_SceneManager->HasScene(sceneID);
    }
}
