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
        , m_removeList {}
        , m_lookupTable {}
        , m_gameObjects{}
        , m_ecsWorld {}
        , m_scenegraph { std::make_unique<scenegraph>("scenegraph") }
        , m_rootGo{ nullptr }
    {
        // Creation of root node
        {
            // differed to initialization after itself exist.
            auto root_handle = m_scenegraph->get_root()->get_handle();
            m_rootGo = std::make_shared<GameObject>(root_handle , *this);
            InsertGameObject(m_rootGo);
        }
        
        /*if (scenegraph::shared_pointer root = m_scenegraph->get_root())
        {
            auto comp = m_rootGo->TryGetComponent<GameObjectComponent>();
            if (comp)
            {
                scenenode::shared_pointer node = comp->Node.lock();
                node = root;
                if (node == nullptr)
                {
                    LOG_ERROR("Shouldn't be null");
                }
                else
                {
                    LOG_INFO("SUCCESS!");
                }
            }
        }*/

        ASSERT_MSG((!IsValid(*m_rootGo)), "Sanity check, root created should be from this scene.");

    }

    Scene::~Scene()
    {
        //delete m_transformSystem;
    }

    void Scene::Init()
    {
        Scene::OnInitEvent e;
        EventManager::Broadcast(&e);
        
        m_transformSystem = std::make_unique<TransformSystem>();

        /*{
            m_ecsWorld.Add_System<oo::TransformSystem>();
        }*/

        PRINT(m_name);
    }
    
    void Scene::Update()
    {
        /*{
            m_ecsWorld.Get_System<oo::TransformSystem>()->Run(&m_ecsWorld);
        }*/
        m_transformSystem->Run(&m_ecsWorld);
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
        PRINT(m_name);

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
    }
    
    void Scene::Exit()
    {
        PRINT(m_name);
        EndOfFrameUpdate();
        m_lookupTable.clear();
        
        for (auto& [key, go] : m_lookupTable)
            go.reset();

        m_gameObjects.clear();
        m_rootGo.reset();
        m_scenegraph.release();
        m_transformSystem.release();
    }
    
    void Scene::LoadScene()
    {
        PRINT(m_name);

        LoadSceneEvent lse{ this };
        EventManager::Broadcast<LoadSceneEvent>(&lse);
    }
    
    void Scene::UnloadScene()
    {
        PRINT(m_name);
        //delete m_transformSystem;
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

    std::shared_ptr<GameObject> Scene::CreateGameObject()
    {
        std::shared_ptr<GameObject> newObjectPtr = std::make_shared<GameObject>(*this);
        return CreateGameObject(newObjectPtr);
    }

    std::shared_ptr<GameObject> Scene::FindWithInstanceID(UUID uuid)
    {
        //LOG_INFO("Finding gameobject of instance ID {0}", uuid);

        if (m_lookupTable.contains(uuid))
            return m_lookupTable.at(uuid);

        return nullptr;
    }

    bool Scene::IsValid(GameObject go) const
    {
        // a valid gameobject will not have its ID as NotFound, will have gameobject component(minimally)
        // and parent will not be equals to NOParent
        return go.GetScene() == this && 
            std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [&](std::shared_ptr<oo::GameObject> elem) { return *elem == go; }) != m_gameObjects.end();
        
        //m_lookupTable.contains(go.GetInstanceID());

        //return !(go.GetEntity().value == GameObject::NOTFOUND.value
        //    || go.HasComponent<GameObjectComponent>() == false
        //    || go.GetWorld() == nullptr
        //    || go.GetWorld() != &m_ecsWorld);
    }

    void Scene::DestroyGameObject(GameObject go)
    {
        ASSERT_MSG(IsValid(go), "Working on an invalid GameObject");

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

        std::shared_ptr<GameObject> target = FindWithInstanceID(go.GetInstanceID());
        // safety check
        if (target == nullptr)
        {
            LOG_CORE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID of {1}", go.GetEntity().value, go.GetInstanceID());
            return;
        }

        // Actual Deletion [Immediate]
        RemoveGameObject(target);
    }

    std::shared_ptr<GameObject> Scene::InstatiateGameObject(GameObject go)
    {
        std::shared_ptr<GameObject> new_instance = std::make_shared<GameObject>(go.Duplicate());
        return CreateGameObject(new_instance);
    }
    
    void Scene::LoadFromFile()
    {
    }

    void Scene::SaveToFile()
    {
    }

    std::shared_ptr<GameObject> Scene::CreateGameObject(std::shared_ptr<GameObject> new_go)
    {
        std::shared_ptr<GameObject> newObjectPtr = new_go;
        InsertGameObject(newObjectPtr);

        // IMPT: we are using the uuid to retrieve back the gameobject as well!
        auto& name = newObjectPtr->Name();
        //auto name = "Just a fake default name for now until ecs is fixed";
        auto shared_ptr = m_scenegraph->create_new_child(name, newObjectPtr->GetInstanceID());
        newObjectPtr->GetComponent<GameObjectComponent>().Node = shared_ptr;

        ASSERT_MSG((!IsValid(*newObjectPtr)), "Sanity check, object created should comply");

        return newObjectPtr;
    }

    void Scene::InsertGameObject(std::shared_ptr<GameObject> go_ptr)
    {
        m_gameObjects.emplace(go_ptr);
        m_lookupTable.emplace(go_ptr->GetInstanceID(), go_ptr);
    }

    void Scene::RemoveGameObject(std::shared_ptr<GameObject> go_ptr)
    {
        m_lookupTable.erase(go_ptr->GetInstanceID());
        m_gameObjects.erase(go_ptr);
        
        // actual deletion : Immediate.
        m_ecsWorld.destroy(go_ptr->GetEntity());
    }
    
    Ecs::ECSWorld& Scene::GetWorld()
    {
        return m_ecsWorld;
    }

    scenegraph const Scene::GetGraph() const
    {
        return *m_scenegraph;
    }

    std::shared_ptr<GameObject> Scene::GetRoot() const
    {
        return m_rootGo;
    }

}
