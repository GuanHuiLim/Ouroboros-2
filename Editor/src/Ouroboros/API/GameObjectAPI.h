#pragma once
#include <Scripting/Scripting.h>
#include <Scripting/ExportAPI.h>
#include "Ouroboros/Scripting/ScriptSystem.h"

#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
    SCRIPT_API uint64_t CreateEntity(uint32_t sceneID)
    {
        std::shared_ptr<Scene> scene = ScriptSystem::GetScene(sceneID);
        std::shared_ptr<GameObject> instance = scene->CreateGameObjectDeferred();

        UUID uuid = instance->GetComponent<GameObjectComponent>().Id;
        scene->GetWorld().Get_System<ScriptSystem>()->SetUpObject(uuid);

        // Testing, normally fresh GameObjects dont have scripts
        scene->GetWorld().Get_System<ScriptSystem>()->InvokeForObject(uuid, "Awake");
        scene->GetWorld().Get_System<ScriptSystem>()->InvokeForObject(uuid, "Start");
        return uuid.GetUUID();
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

    //SCRIPT_API void DestroyEntity(Entity id)
    //{
    //    GameObject obj{ id };
    //    obj.Destroy();
    //}

    SCRIPT_API bool CheckEntityExists(Scene::ID_type sceneID, UUID uuid)
    {
        std::weak_ptr<IScene> scene_weak = ScriptSystem::s_SceneManager->GetScene(sceneID);
        if (scene_weak.expired())
        {
            return false;
        }
        std::shared_ptr<Scene> scene = std::dynamic_pointer_cast<Scene>(scene_weak.lock());
        std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(uuid);
        return obj != nullptr;
    }

    SCRIPT_API ScriptDatabase::IntPtr GameObject_GetName(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        std::string const& name = obj->GetComponent<GameObjectComponent>().Name;
        MonoString* string = ScriptEngine::CreateString(name.c_str());
        return mono_gchandle_new((MonoObject*)string, false);
    }

    SCRIPT_API void GameObject_SetName(Scene::ID_type sceneID, UUID uuid, const char* newName)
    {
        std::shared_ptr<GameObject> obj = ScriptSystem::GetObjectFromScene(sceneID, uuid);
        obj->GetComponent<GameObjectComponent>().Name = newName;
    }

    //SCRIPT_API bool GameObject_GetActive(Entity id)
    //{
    //    GameObject obj{ id };
    //    return obj.GetComponent<GameObjectComponent>().Active;
    //}

    //SCRIPT_API bool GameObject_GetActiveInHierarchy(Entity id)
    //{
    //    GameObject obj{ id };
    //    return obj.GetComponent<GameObjectComponent>().ActiveInHierarchy;
    //}


    //SCRIPT_API void GameObject_SetActive(Entity id, bool value)
    //{
    //    GameObject obj{ id };
    //    obj.SetActive(value);
    //}

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
