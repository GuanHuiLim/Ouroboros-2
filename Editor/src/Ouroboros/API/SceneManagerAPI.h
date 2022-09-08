#pragma once
#include <Scripting/ExportAPI.h>

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
