#pragma once

#include <Scripting/Scripting.h>
#include "ScriptInfo.h"

#include <SceneManagement/include/SceneManager.h>
#include "Ouroboros/Scene/Scene.h"

namespace oo
{
    // Global stuff that transfers between scenes, go to ScriptSystem for scene specific stuff
    class ScriptManager final
    {
    public:
        static SceneManager const* s_SceneManager;

    private:
        static std::string s_BuildPath;
        static std::string s_ProjectPath;

        static std::vector<ScriptClassInfo> s_ScriptList;

    public:
        static void LoadProject(std::string const& buildPath, std::string const& projectPath);
        static bool Compile();
        static void Load();

        static bool DisplayWarnings();
        static bool DisplayErrors();

        static inline std::shared_ptr<Scene> GetScene(Scene::ID_type sceneID)
        {
            std::weak_ptr scene_weak = s_SceneManager->GetScene(sceneID);
            if (scene_weak.expired())
            {
                LOG_ERROR("scene with id ({0}) does not exist", sceneID);
                ScriptEngine::ThrowNullException();
            }
            return std::dynamic_pointer_cast<Scene>(scene_weak.lock());
        }

        static inline std::shared_ptr<GameObject> GetObjectFromScene(Scene::ID_type sceneID, UUID uuid)
        {
            std::shared_ptr<Scene> scene = GetScene(sceneID);
            std::shared_ptr<GameObject> obj = scene->FindWithInstanceID(uuid);
            if (obj == nullptr)
            {
                ScriptEngine::ThrowNullException();
            }
            return obj;
        }

        static inline std::vector<ScriptClassInfo> const& GetScriptList()
        {
            return s_ScriptList;
        }

        template<typename T>
        static void RegisterComponent(std::string const& name_space, std::string const& name)
        {
            ComponentDatabase::ComponentAction add = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                if (scene_weak.expired())
                    return;
                Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                std::shared_ptr<GameObject> obj = scene.FindWithInstanceID(uuid);
                obj->AddComponent<T>();
            };
            ComponentDatabase::ComponentCheck has = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                if (scene_weak.expired())
                    return false;
                Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                std::shared_ptr<GameObject> obj = scene.FindWithInstanceID(uuid);
                return obj->HasComponent<T>();
            };
            ComponentDatabase::ComponentSetAction setEnabled = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid, bool isEnabled)
            {
                //std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                //if (scene_weak.expired())
                //    return;
                //Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                //std::shared_ptr<GameObject> obj = scene.FindWithInstanceID(uuid);
                //obj->GetComponent<T>().SetActive(isEnabled);
            };
            ComponentDatabase::ComponentCheck isEnabled = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                //std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                //if (scene_weak.expired())
                //    return false;
                //Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                //std::shared_ptr<GameObject> obj = scene.FindWithInstanceID(uuid);
                //return obj->GetComponent<T>().IsActive();
                return true;
            };
            ComponentDatabase::ComponentAction remove = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                if (scene_weak.expired())
                    return;
                Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                std::shared_ptr<GameObject> obj = scene.FindWithInstanceID(uuid);
                return obj->RemoveComponent<T>();
            };
            ComponentDatabase::RegisterComponent(name_space, name, add, remove, has, setEnabled, isEnabled);
        }

    };
}