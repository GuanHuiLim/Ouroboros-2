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
#include "Ouroboros/Scripting/ExportAPI.h"

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
        std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(uuid);
        if (obj == nullptr)
            ScriptEngine::ThrowNullException();
        obj->GetComponent<ScriptComponent>().AddScriptInfo(ScriptClassInfo{ name_space, name });

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

    SCRIPT_API MonoArray* GetScriptsInChildren(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name, bool includeSelf)
    {
        ScriptSystem* ss = ScriptManager::GetScene(sceneID)->GetWorld().Get_System<ScriptSystem>();
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        std::vector<UUID> childList = obj->GetChildrenUUID(includeSelf);

        std::vector<MonoObject*> scriptList;
        for (UUID child : childList)
        {
            ScriptDatabase::IntPtr ptr = ss->GetScript(child, name_space, name);
            if (ptr == 0)
                continue;
            scriptList.emplace_back(mono_gchandle_get_target(ptr));
        }
        
        MonoArray* arr = ScriptEngine::CreateArray(ScriptEngine::GetClass("Scripting", name_space, name), scriptList.size());
        for(size_t i = 0; i < scriptList.size(); ++i)
        {
            mono_array_set(arr, MonoObject*, i, scriptList[i]);
        }
        return arr;
    }

    SCRIPT_API void RemoveScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(uuid);
        if (obj == nullptr)
            ScriptEngine::ThrowNullException();
        obj->GetComponent<ScriptComponent>().RemoveScriptInfo(ScriptClassInfo{ name_space, name });

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

    SCRIPT_API MonoArray* GetComponentsInChildrenFromScript(Scene::ID_type sceneID, UUID uuid, const char* name_space, const char* name, bool includeSelf)
    {
        ScriptSystem* ss = ScriptManager::GetScene(sceneID)->GetWorld().Get_System<ScriptSystem>();
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        std::vector<UUID> childList = obj->GetChildrenUUID(includeSelf);

        std::vector<MonoObject*> scriptList;
        for (UUID child : childList)
        {
            ScriptDatabase::IntPtr ptr = ss->GetComponent(child, name_space, name);
            if (ptr == 0)
                continue;
            scriptList.emplace_back(mono_gchandle_get_target(ptr));
        }

        MonoArray* arr = ScriptEngine::CreateArray(ScriptEngine::GetClass("ScriptCore", name_space, name), scriptList.size());
        for (size_t i = 0; i < scriptList.size(); ++i)
        {
            mono_array_set(arr, MonoObject*, i, scriptList[i]);
        }
        return arr;
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