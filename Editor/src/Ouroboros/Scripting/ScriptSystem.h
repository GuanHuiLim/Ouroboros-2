#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include <Scripting/Scripting.h>

#include <SceneManagement/include/SceneManager.h>
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
    class ScriptSystem final
    {
        // Global stuff that transfers between scenes
    public:
        static SceneManager const* s_SceneManager;

    private:
        static std::string s_BuildPath;
        static std::string s_ProjectPath;

        static bool s_IsPlaying;
        static std::vector<std::string> s_ScriptList;

    public:
        static void LoadProject(std::string const& buildPath, std::string const& projectPath);
        static bool Compile();
        static void Load();

        static bool DisplayWarnings();

        static bool DisplayErrors();

        static inline std::vector<std::string> const& GetScriptList()
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
                GameObject& obj = *(scene.FindWithInstanceID(uuid));
                obj.AddComponent<T>();
            };
            ComponentDatabase::ComponentCheck has = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                if (scene_weak.expired())
                    return false;
                Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                GameObject& obj = *(scene.FindWithInstanceID(uuid));
                return obj.HasComponent<T>();
            };
            ComponentDatabase::ComponentSetAction setEnabled = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid, bool isEnabled)
            {
                //std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                //if (scene_weak.expired())
                //    return;
                //Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                //GameObject& obj = *(scene.FindWithInstanceID(uuid));
                //obj.GetComponent<T>().SetActive(isEnabled);
            };
            ComponentDatabase::ComponentCheck isEnabled = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                //std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                //if (scene_weak.expired())
                //    return false;
                //Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                //GameObject& obj = *(scene.FindWithInstanceID(uuid));
                //return obj.GetComponent<T>().IsActive();
                return true;
            };
            ComponentDatabase::ComponentAction remove = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                if (scene_weak.expired())
                    return;
                Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                GameObject& obj = *(scene.FindWithInstanceID(uuid));
                return obj.RemoveComponent<T>();
            };
            ComponentDatabase::RegisterComponent(name_space, name, add, remove, has, setEnabled, isEnabled);
        }

        // Scene specific script stuff
    public:
        ScriptSystem(Scene& scene);
        ~ScriptSystem();

        bool StartPlay();

        bool StopPlay();

        void InvokeForAll(const char* functionName, int paramCount = 0, void** params = NULL);
        void InvokeForAllEnabled(const char* functionName, int paramCount = 0, void** params = NULL);

    private:
        Scene& scene;
        ScriptDatabase scriptDatabase;
        ComponentDatabase componentDatabase;
    };
}