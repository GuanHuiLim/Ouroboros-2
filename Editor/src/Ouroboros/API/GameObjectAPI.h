/************************************************************************************//*!
\file           GameObjectAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to perform actions related to GameObjects, like their creation, deletion,
                checking if it exists, and checking its active state

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <Scripting/Scripting.h>
#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"
#include "Ouroboros/Scripting/ScriptSystem.h"

#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"

#include "Project.h"
#include "App/Editor/Serializer.h"

#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/EventSystem/EventManager.h"

namespace oo
{
    SCRIPT_API uint64_t CreateEntity(uint32_t sceneID)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        std::shared_ptr<GameObject> instance = scene->CreateGameObjectDeferred();

        oo::UUID uuid = instance->GetInstanceID();
        scene->GetWorld().Get_System<ScriptSystem>()->SetUpObject(uuid);

        // Testing, normally fresh GameObjects dont have scripts
        scene->GetWorld().Get_System<ScriptSystem>()->InvokeForObject(uuid, "Awake");
        scene->GetWorld().Get_System<ScriptSystem>()->InvokeForObject(uuid, "Start");
        return uuid.GetUUID();
    }

    SCRIPT_API bool CheckValidPrefabPath(const char* filePath)
    {
        return std::filesystem::exists(Project::GetPrefabFolder() / filePath);
    }

    SCRIPT_API ComponentDatabase::IntPtr InstantiatePrefab(Scene::ID_type sceneID, const char* filePath, oo::UUID parentID)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        std::shared_ptr<GameObject> parent = ScriptManager::GetObjectFromScene(sceneID, parentID);
        UUID uuid = Serializer::LoadPrefab(Project::GetPrefabFolder().string() + "/" + filePath, parent, *scene);

        oo::PrefabSpawnedEvent evnt{};
        evnt.gameobjectUUID = uuid;
        oo::EventManager::Broadcast(&evnt);

        ScriptSystem* ss = scene->GetWorld().Get_System<ScriptSystem>();
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        std::vector<UUID> children = obj->GetChildrenUUID(true);

        // set up all C# stuff for all objects first, may be relied on in inspector variables, Awake and Start
        for (UUID childUUID : children)
        {
            ss->SetUpObject(childUUID);
        }
        // update all C# inspector fields, may be relied on in Awake and Start
        for (UUID childUUID : children)
        {
            ss->UpdateObjectFieldsWithInfo(childUUID);
        }
        // Invoke all Awake first, Start may rely on Awake running first
        for (UUID childUUID : children)
        {
            ss->InvokeForObject(childUUID, "Awake");
        }
        // Invoke all Start last
        for (UUID childUUID : children)
        {
            ss->InvokeForObject(childUUID, "Start");
        }

        return ss->GetGameObject(uuid);
    }

    SCRIPT_API ComponentDatabase::IntPtr DuplicateGameObject(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<Scene> scene = ScriptManager::GetScene(sceneID);
        ScriptSystem* ss = scene->GetWorld().Get_System<ScriptSystem>();

        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        GameObject dup = obj->Duplicate();

        std::vector<UUID> children = dup.GetChildrenUUID(true);

        // set up all C# stuff for all objects first, may be relied on in inspector variables, Awake and Start
        for (UUID childUUID : children)
        {
            ss->SetUpObject(childUUID);
        }
        // update all C# inspector fields, may be relied on in Awake and Start
        for (UUID childUUID : children)
        {
            ss->UpdateObjectFieldsWithInfo(childUUID);
        }
        // Invoke all Awake first, Start may rely on Awake running first
        for (UUID childUUID : children)
        {
            ss->InvokeForObject(childUUID, "Awake");
        }
        // Invoke all Start last
        for (UUID childUUID : children)
        {
            ss->InvokeForObject(childUUID, "Start");
        }

        return ss->GetGameObject(dup.GetInstanceID());
    }

    //SCRIPT_API uint32_t InstantiateEntity(Entity src)
    //{
    //    GameObject source{ src };
    //    GameObject instance = SceneManager::GetActiveScene().CreateGameObject();
    //    if (source.HasComponent<PrefabComponent>())
    //    {
    //        SceneManager::GetActiveWorld().GetSystem<PrefabComponentSystem>()->InstantiateFromPrefab(source, instance);
    //    }
    //    else
    //    {
    //        //instance = GameObject::Instantiate(srcObj);
    //        //SceneManager::GetActiveWorld().DuplicateEntity(source, instance); // doesn't copy children
    //        LOG_WARN("Engine currently does not support GameObject Duplication");
    //        return 0;
    //    }
    //    auto children = instance.GetChildren(true);
    //    for (GameObject obj : children)
    //    {
    //        obj.GetComponent<Scripting>().SetUpPlay();
    //    }
    //    for (GameObject obj : children)
    //    {
    //        obj.GetComponent<Scripting>().StartPlay(false);
    //    }
    //    for (GameObject obj : children)
    //    {
    //        obj.GetComponent<Scripting>().InvokeFunctionAll("Awake");
    //    }
    //    return instance.GetComponent<Scripting>().GetGameObjectPtr();
    //}

    SCRIPT_API void DestroyEntity(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        obj->Destroy();
    }

    SCRIPT_API bool CheckEntityExists(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::weak_ptr<IScene> scene_weak = ScriptManager::s_SceneManager->GetScene(sceneID);
        if (scene_weak.expired())
        {
            return false;
        }
        std::shared_ptr<Scene> scene = std::dynamic_pointer_cast<Scene>(scene_weak.lock());
        std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(uuid);
        return obj != nullptr;
    }

    SCRIPT_API ScriptDatabase::IntPtr GameObject_GetName(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        std::string const& name = obj->GetComponent<GameObjectComponent>().Name;
        MonoString* string = ScriptEngine::CreateString(name.c_str());
        return mono_gchandle_new((MonoObject*)string, false);
    }

    SCRIPT_API void GameObject_SetName(Scene::ID_type sceneID, oo::UUID uuid, const char* newName)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        obj->GetComponent<GameObjectComponent>().Name = newName;
    }

    SCRIPT_API bool GameObject_GetActive(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        return obj->IsActive();
    }

    SCRIPT_API bool GameObject_GetActiveInHierarchy(Scene::ID_type sceneID, oo::UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        return obj->ActiveInHierarchy();
    }


    SCRIPT_API void GameObject_SetActive(Scene::ID_type sceneID, oo::UUID uuid, bool value)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        obj->SetActive(value);
    }

    //SCRIPT_API unsigned GameObject_GetLayer(Entity id)
    //{
    //    GameObject obj{ id };
    //    return static_cast<unsigned>(obj.GetComponent<GameObjectComponent>().Layer.to_ulong());
    //}

    //SCRIPT_API void GameObject_SetLayer(Entity id, unsigned newLayer)
    //{
    //    GameObject obj{ id };
    //    obj.GetComponent<GameObjectComponent>().Layer = newLayer;
    //}

    ///*-----------------------------------------------------------------------------*/
    ///* Prefab Function                                                             */
    ///*-----------------------------------------------------------------------------*/

    //SCRIPT_API uint32_t Prefab_LoadFromFile(const char* filePath)
    //{
    //    std::filesystem::path fullPath = EditorProject::GetActiveConfig().PrefabFolderPath() + "\\" + filePath;
    //    if (fullPath.extension() != ".prefab" || !std::filesystem::exists(fullPath))
    //        return 0;
    //    GameObject prefab = SceneManager::GetActiveWorld().GetSystem<PrefabComponentSystem>()->AddPrefab(fullPath.string());
    //    Scripting& prefabScripting = prefab.GetComponent<Scripting>();
    //    if (prefabScripting.GetGameObjectPtr() == 0)
    //    {
    //        auto children = prefab.GetChildren(true);
    //        for (GameObject obj : children)
    //        {
    //            obj.GetComponent<Scripting>().SetUpPlay();
    //        }
    //        for (GameObject obj : children)
    //        {
    //            obj.GetComponent<Scripting>().StartPlay(false);
    //        }
    //    }
    //    return prefabScripting.GetGameObjectPtr();
    //}
}
