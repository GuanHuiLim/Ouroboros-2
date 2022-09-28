/************************************************************************************//*!
\file           ScriptingAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to perform actions related to other C# scripts or C# interfaces to
                C++ ECS Components. Such actions include adding, getting, and removing
                C# scripts and C++ ECS Components, and checking if they are enabled

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <Scripting/ExportAPI.h>

#include "Ouroboros/Scripting/ScriptManager.h"
#include "Ouroboros/Scripting/ScriptSystem.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
        /* Script Functions for C#                                                     */
        /*-----------------------------------------------------------------------------*/

    SCRIPT_API ScriptDatabase::IntPtr AddScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        ScriptDatabase::IntPtr ptr = scene->GetWorld().Get_System<ScriptSystem>()->AddScript(uuid, name_space, name);
        MonoObject* script = mono_gchandle_get_target(ptr);
        try
        {
            ScriptEngine::InvokeFunction(script, "Awake");
            ScriptEngine::InvokeFunction(script, "Start");
        }
        catch (std::exception const& e)
        {
            LOG_ERROR(e.what());
        }
        return ptr;
    }

    SCRIPT_API ScriptDatabase::IntPtr GetScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        return scene->GetWorld().Get_System<ScriptSystem>()->GetScript(uuid, name_space, name);
    }

    SCRIPT_API void RemoveScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        scene->GetWorld().Get_System<ScriptSystem>()->RemoveScript(uuid, name_space, name);
    }

    SCRIPT_API void SetScriptEnabled(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name, bool enabled)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        scene->GetWorld().Get_System<ScriptSystem>()->SetScriptEnabled(uuid, name_space, name, enabled);
    }

    SCRIPT_API bool CheckScriptEnabled(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        return scene->GetWorld().Get_System<ScriptSystem>()->CheckScriptEnabled(uuid, name_space, name);
    }



    SCRIPT_API uint32_t AddComponentFromScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        return scene->GetWorld().Get_System<ScriptSystem>()->AddComponent(uuid, name_space, name);
    }

    SCRIPT_API uint32_t GetComponentFromScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        return scene->GetWorld().Get_System<ScriptSystem>()->GetComponent(uuid, name_space, name);
    }

    SCRIPT_API void RemoveComponentFromScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        scene->GetWorld().Get_System<ScriptSystem>()->RemoveComponent(uuid, name_space, name);
    }

    SCRIPT_API bool CheckComponentEnabled(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        return scene->GetWorld().Get_System<ScriptSystem>()->CheckComponentEnabled(uuid, name_space, name);
    }

    SCRIPT_API void SetComponentEnabled(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name, bool active)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        if (scene->FindWithInstanceID(uuid) == nullptr)
            ScriptEngine::ThrowNullException();
        scene->GetWorld().Get_System<ScriptSystem>()->SetComponentEnabled(uuid, name_space, name, active);
    }
}