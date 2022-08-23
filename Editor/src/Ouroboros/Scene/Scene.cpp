/************************************************************************************//*!
\file           Scene.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 31, 2022
\brief          Scene defines the basis of what makes a scene and most of its core
                features such as tracking all the gameobjects, having access to the
                scene graph and root gameobject.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Scene.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Core/Log.h"

#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/Transform/TransformSystem.h"

#include "App/Editor/Events/LoadSceneEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/ECS/DifferedSystem.h"

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
    #define PRINT(name) std::cout << "[" << (name) << "] : " << __FUNCTION__ << std::endl;
#else
    #define PRINT(name) 
#endif

namespace oo
{
    Scene::Scene(std::string_view name) 
        : m_name{ name }
        , m_filepath{ "unassigned filepath" }
        //, m_createList {}
        , m_removeList {}
        , m_lookupTable {}
        , m_gameObjects{}
        , m_ecsWorld { nullptr }
        , m_scenegraph { nullptr }
        , m_rootGo { nullptr }
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::Init()
    {
        constexpr const char* const scene_init = "Scene_init";
        {
            TRACY_TRACK_PERFORMANCE(scene_init);
            TRACY_PROFILE_SCOPE_NC(scene_init, tracy::Color::Seashell1);

            Scene::OnInitEvent e;
            EventManager::Broadcast(&e);
        
            // Initialize Default Systems
            {
                m_ecsWorld->Add_System<oo::DifferedSystem>();
                m_ecsWorld->Get_System<oo::DifferedSystem>()->Link(this);

                m_ecsWorld->Add_System<oo::TransformSystem>();
                m_ecsWorld->Get_System<oo::TransformSystem>()->Link(this);
            }

            PRINT(m_name);
            
            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_DISPLAY_PERFORMANCE_SELECTED(scene_init);
    }
    
    void Scene::Update()
    {
        // Update Systems
        {
            m_ecsWorld->Get_System<oo::TransformSystem>()->Run(m_ecsWorld.get());
        }

        PRINT(m_name);
    }
    
    void Scene::LateUpdate()
    {
        PRINT(m_name);
    }
    
    void Scene::Render()
    {
        PRINT(m_name);
    }
    
    void Scene::EndOfFrameUpdate()
    {
        constexpr const char* const scene_end_of_frame_update = "Scene_end_of_frame_update";
        {
            TRACY_TRACK_PERFORMANCE(scene_end_of_frame_update);
            TRACY_PROFILE_SCOPE_NC(scene_end_of_frame_update, tracy::Color::Seashell2);

            PRINT(m_name);

            //// go through all things to create and add at the end of frame and do so.
            //for (auto& [go, callback] : m_createList)
            //{
            //    CreateGameObjectImmediate(go);
            //    callback(go);
            //}
            //m_createList.clear();
            m_ecsWorld->Get_System<oo::DifferedSystem>()->Run(m_ecsWorld.get());

            // go through all things to remove at the end of frame and do so.
            for (auto& uuid : m_removeList)
            {
                auto go_ptr = FindWithInstanceID(uuid);
                ASSERT_MSG
                (
                    (go_ptr == nullptr),
                    "Attempting to delete an object that's already been removed"
                );

                // Actual Deletion
                RemoveGameObject(go_ptr);
            }
            m_removeList.clear();


            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_DISPLAY_PERFORMANCE_SELECTED(scene_end_of_frame_update);
    }
    
    void Scene::Exit()
    {
        PRINT(m_name);
    }
    
    void Scene::LoadScene()
    {
        constexpr const char* const scene_loading = "scene_loading";
        {
            TRACY_TRACK_PERFORMANCE(scene_loading);
            TRACY_PROFILE_SCOPE_NC(scene_loading, tracy::Color::Seashell3);

            PRINT(m_name);
            
            //m_createList.clear();
            m_removeList.clear();
            m_lookupTable.clear();
            m_gameObjects.clear();
            m_ecsWorld = std::make_unique<Ecs::ECSWorld>();
            m_scenegraph = std::make_unique<scenegraph>("scenegraph");
            m_rootGo = nullptr;

            // Creation of root node
            {
                // differed to initialization after itself exist.
                auto root_handle = m_scenegraph->get_root()->get_handle();
                m_rootGo = std::make_shared<GameObject>(root_handle, *this);
                InsertGameObject(m_rootGo);
                m_rootGo->GetComponent<GameObjectComponent>().Node = m_scenegraph->get_root();
            }

            ASSERT_MSG((!IsValid(*m_rootGo)), "Sanity check, root created should be from this scene.");

            // Broadcast event to load scene
            LoadSceneEvent lse{ this };
            EventManager::Broadcast<LoadSceneEvent>(&lse);

            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_DISPLAY_PERFORMANCE_SELECTED(scene_loading);
    }
    
    void Scene::UnloadScene()
    {
        constexpr const char* const scene_unload = "scene_unload";
        {
            TRACY_PROFILE_SCOPE_NC(scene_unload, tracy::Color::Seashell4);
            TRACY_TRACK_PERFORMANCE(scene_unload);

            PRINT(m_name);

            EndOfFrameUpdate();
            m_lookupTable.clear();
            m_gameObjects.clear();
            m_rootGo.reset();
            m_scenegraph.reset();
            m_ecsWorld.reset();
            
            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_DISPLAY_PERFORMANCE_SELECTED(scene_unload);
    }
    
    void Scene::ReloadScene()
    {
        PRINT(m_name);
        UnloadScene();
        LoadScene();
    }

    LoadStatus Scene::GetProgress() const
    {
        PRINT(m_name);
        return LoadStatus();
    }
    
    void Scene::SetFilePath(std::string_view filepath)
    {
        m_filepath = filepath;
    }
    
    std::string Scene::GetFilePath() const 
    { 
        return m_filepath; 
    }
    
    void Scene::SetSceneName(std::string_view name) 
    { 
        m_name = name; 
    }
    
    std::string Scene::GetSceneName() const 
    { 
        return m_name; 
    }

    Scene::go_ptr Scene::CreateGameObjectDiffered()
    {
        LOG_INFO("Creating Differed Game Object");

        Scene::go_ptr newObjectPtr = std::make_shared<GameObject>(*this);
        //m_createList.emplace_back(std::make_pair(newObjectPtr, onCreationCallback));
        newObjectPtr = CreateGameObjectImmediate(newObjectPtr);
        // add differed component and set the entity ID to be itself
        newObjectPtr->AddComponent<DifferedComponent>().entityID = newObjectPtr->GetEntity();
        return newObjectPtr;
    }

    Scene::go_ptr Scene::CreateGameObjectImmediate()
    {
        Scene::go_ptr newObjectPtr = std::make_shared<GameObject>(*this);
        return CreateGameObjectImmediate(newObjectPtr);
    }

    Scene::go_ptr Scene::FindWithInstanceID(UUID uuid) const
    {
        //LOG_INFO("Finding gameobject of instance ID {0}", uuid);

        if (m_lookupTable.contains(uuid))
            return m_lookupTable.at(uuid);

        return nullptr;
    }

    bool Scene::IsValid(UUID uuid) const
    {
        return m_lookupTable.contains(uuid);
    }

    bool Scene::IsValid(GameObject go) const
    {
        // a valid gameobject will not have its ID as NotFound, will have gameobject component(minimally)
        // and parent will not be equals to NOParent
        return go.GetScene() == this && 
            std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [&](std::shared_ptr<oo::GameObject> elem) { return *elem == go; }) != m_gameObjects.end();
    }

    void Scene::DestroyGameObject(GameObject go)
    {
        ASSERT_MSG(IsValid(go) == false, "Working on an invalid GameObject");

        // safety check
        if (FindWithInstanceID(go.GetInstanceID()) == nullptr)
        {
            LOG_CORE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID of {1}", go.GetEntity().value, go.GetInstanceID());
            return;
        }

        // Queue selected node for deletion
        m_removeList.emplace(go.GetInstanceID());

        // Then continue to destroy all its children
        for (auto const& childuuid : go.GetChildrenUUID())
        {
            auto childobj = FindWithInstanceID(childuuid);
            if(childobj)
                DestroyGameObject(*childobj);
            else            
                LOG_CORE_ERROR("attempting to remove an invalid uuid {0} from scene {1}", childuuid, m_name);
        }
    }

    void Scene::DestroyGameObjectImmediate(GameObject go)
    {
        ASSERT_MSG(IsValid(go), "Working on an invalid GameObject");

        Scene::go_ptr target = FindWithInstanceID(go.GetInstanceID());
        // safety check
        if (target == nullptr)
        {
            LOG_CORE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID of {1}", go.GetEntity().value, go.GetInstanceID());
            return;
        }

        // Actual Deletion [Immediate]
        RemoveGameObject(target);
    }

    Scene::go_ptr Scene::InstatiateGameObject(GameObject go)
    {
        Scene::go_ptr new_instance = std::make_shared<GameObject>(go.Duplicate());
        return CreateGameObjectImmediate(new_instance);
    }
    
    void Scene::LoadFromFile()
    {
    }

    void Scene::SaveToFile()
    {
    }

    Scene::go_ptr Scene::CreateGameObjectImmediate(Scene::go_ptr new_go)
    {
        Scene::go_ptr newObjectPtr = new_go;
        InsertGameObject(newObjectPtr);

        // IMPT: we are using the uuid to retrieve back the gameobject as well!
        auto& name = newObjectPtr->Name();
        //auto name = "Just a fake default name for now until ecs is fixed";
        auto shared_ptr = m_scenegraph->create_new_child(name, newObjectPtr->GetInstanceID());
        newObjectPtr->GetComponent<GameObjectComponent>().Node = shared_ptr;

        ASSERT_MSG((!IsValid(*newObjectPtr)), "Sanity check, object created should comply");

        return newObjectPtr;
    }

    void Scene::InsertGameObject(Scene::go_ptr go_ptr)
    {
        m_gameObjects.emplace(go_ptr);
        m_lookupTable.emplace(go_ptr->GetInstanceID(), go_ptr);
    }

    void Scene::RemoveGameObject(Scene::go_ptr go_ptr)
    {
        // remove from scenegraph first. Timing is important here.
        if (auto scenegraph_go = go_ptr->GetSceneNode().lock())
        {
            scenegraph_go->detach();
        }

        m_lookupTable.erase(go_ptr->GetInstanceID());
        m_gameObjects.erase(go_ptr);

        // actual deletion : Immediate.
        m_ecsWorld->destroy(go_ptr->GetEntity());
    }
    
    Ecs::ECSWorld& Scene::GetWorld()
    {
        return *m_ecsWorld;
    }

    scenegraph const Scene::GetGraph() const
    {
        return *m_scenegraph;
    }

    Scene::go_ptr Scene::GetRoot() const
    {
        return m_rootGo;
    }

}
