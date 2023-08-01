/************************************************************************************//*!
\file           ScriptManager.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Declares the manager responsible for handling the global functionality of the
                scripting feature that is unique to each project but that carries over between scenes

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Scripting/Scripting.h>
#include "ScriptInfo.h"

#include <SceneManagement/include/SceneManager.h>
#include "Ouroboros/Scene/Scene.h"

#include "Ouroboros/Asset/AssetManager.h"

namespace oo
{
    // Global stuff that transfers between scenes, go to ScriptSystem for scene specific stuff
    class ScriptManager final
    {
    public:
        static SceneManager* s_SceneManager;

    private:
        static std::string s_BuildPath;
        static std::string s_ProjectPath;

        static std::vector<ScriptClassInfo> s_ScriptList;
        static std::vector<ScriptClassInfo> s_BeforeDefaultOrder;
        static std::vector<ScriptClassInfo> s_AfterDefaultOrder;

        static std::vector<std::pair<ScriptDatabase::IntPtr, AssetManager::LoadProgressPtr>> s_sceneLoadingTrackers;

    public:
        /*********************************************************************************//*!
        \brief      Used to set up all the required C# related variables to run scripting
                    when a new or different project is loaded in the editor

        \param      buildPath
                the file path where the scripting dll can be found and should be built to
        \param      projectPath
                the file path to the .csproj of the visual studio C# scripting project
        *//**********************************************************************************/
        static void LoadProject(std::string const& buildPath, std::string const& projectPath);
        /*********************************************************************************//*!
        \brief      attempts to compile all scripts in the loaded project into a dll for execution.
                    If this fails, the errors will be outputted to the editor's logger.

        \return     true if the scripts were successfully compiled, else false
        *//**********************************************************************************/
        static bool Compile();
        /*********************************************************************************//*!
        \brief      attempts to load a scripting dll for execution at the given path provided
                    by LoadProject
        *//**********************************************************************************/
        static void Load();

        /*********************************************************************************//*!
        \brief      outputs any warnings from the warnings file obtained from
                    the most recent compilation onto the engine's logger

        \return     true if the warnings file exists and there are warnings, else false
        *//**********************************************************************************/
        static bool DisplayWarnings();
        /*********************************************************************************//*!
        \brief  outputs any errors from the errors file obtained from
                the most recent compilation onto the engine's logger

        \return     true if the errors file exists and there are errors, else false
        *//**********************************************************************************/
        static bool DisplayErrors();

        /*********************************************************************************//*!
        \brief      Helper function used to get a scene by its ID. This is mainly done
                    in C++ functions meant for C# scripts to call, and has the added feature
                    of throwing a C# null exception if a scene with the provided ID cannot be found

        \param      sceneID
                the id of the requested scene
        \return     a shared_ptr to the requested scene, if it is found
        *//**********************************************************************************/
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

        /*********************************************************************************//*!
        \brief      Helper function used to get a scene by its name. This is mainly done
                    in C++ functions meant for C# scripts to call, and has the added feature
                    of throwing a C# null exception if a scene with the provided ID cannot be found

        \param      name
                the name of the requested scene
        \return     a shared_ptr to the requested scene, if it is found
        *//**********************************************************************************/
        static inline std::shared_ptr<Scene> GetScene(std::string_view name)
        {
            std::weak_ptr scene_weak = s_SceneManager->GetScene(name);
            if (scene_weak.expired())
            {
                LOG_ERROR("scene with name ({0}) does not exist", name);
                ScriptEngine::ThrowNullException();
            }
            return std::dynamic_pointer_cast<Scene>(scene_weak.lock());
        }

        /*********************************************************************************//*!
        \brief      Helper function used to get a specific GameObject from a scene by its ID.
                    This is mainly done in C++ functions meant for C# scripts to call,
                    and has the added feature of throwing a C# null exception if 
                    a scene or GameObject with the provided ID cannot be found

        \param      sceneID
                the id of the scene that the requested GameObject belongs to
        \param      uuid
                the id of the requested GameObject

        \return     a shared_ptr to the requested GameObject, if it is found
        *//**********************************************************************************/
        static inline std::shared_ptr<GameObject> GetObjectFromScene(Scene::ID_type sceneID, oo::UUID uuid)
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

        static inline std::vector<ScriptClassInfo> const& GetBeforeDefaultOrder()
        {
            return s_BeforeDefaultOrder;
        }
        static void InsertBeforeDefaultOrder(ScriptClassInfo const& classInfo);
        static void InsertBeforeDefaultOrder(ScriptClassInfo const& classInfo, size_t pos);
        static void InsertBeforeDefaultOrder(ScriptClassInfo const& classInfo, std::vector<ScriptClassInfo>::iterator iter);
        static void RemoveBeforeDefaultOrder(ScriptClassInfo const& classInfo);

        static inline std::vector<ScriptClassInfo> const& GetAfterDefaultOrder()
        {
            return s_AfterDefaultOrder;
        }
        static void InsertAfterDefaultOrder(ScriptClassInfo const& classInfo);
        static void InsertAfterDefaultOrder(ScriptClassInfo const& classInfo, size_t pos);
        static void InsertAfterDefaultOrder(ScriptClassInfo const& classInfo, std::vector<ScriptClassInfo>::iterator iter);
        static void RemoveAfterDefaultOrder(ScriptClassInfo const& classInfo);

        static std::vector<ScriptClassInfo> const GetScriptNamesByExecutionOrder();
        static std::vector<MonoClass*> const GetScriptExecutionOrder();
        static inline void ClearScriptExecutionOrder()
        {
            s_BeforeDefaultOrder.clear();
            s_AfterDefaultOrder.clear();
        }

        /*********************************************************************************//*!
        \brief      Helper function used to register C++ ECS components to the scripting
                    component database so that the C# interfaces can be linked to their
                    corresponding C++ ECS component

        \param      name_space
                the name space of the component's corresponding C# interface's class
        \param      name
                the name of the component's corresponding C# interface's class
        *//**********************************************************************************/
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
            ComponentDatabase::ComponentAction remove = [](ComponentDatabase::SceneID sceneID, ComponentDatabase::UUID uuid)
            {
                std::weak_ptr<IScene> scene_weak = s_SceneManager->GetScene(static_cast<IScene::ID_type>(sceneID));
                if (scene_weak.expired())
                    return;
                Scene& scene = *(std::dynamic_pointer_cast<Scene>(scene_weak.lock()).get());
                std::shared_ptr<GameObject> obj = scene.FindWithInstanceID(uuid);
                return obj->RemoveComponent<T>();
            };
            ComponentDatabase::RegisterComponent(name_space, name, add, remove, has);
        }

        static ScriptDatabase::IntPtr TrackLoadingProgress(AssetManager::LoadProgressPtr ptr);
        static void UpdateLoadingProgress();
        static void DeleteAllLoadingProgress();
    };
}