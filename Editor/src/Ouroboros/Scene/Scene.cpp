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
#include "App/Editor/Events/UnloadSceneEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/ECS/DeferredSystem.h"

#include "Ouroboros/Scripting/ScriptSystem.h"

#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Vulkan/VulkanContext.h"

#include "Ouroboros/Vulkan/RendererSystem.h"

#include "Ouroboros/Audio/AudioSystem.h"

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
        , m_graphicsWorld { nullptr }
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
                m_ecsWorld->Add_System<oo::DeferredSystem>(this);
                //m_ecsWorld->Get_System<oo::DeferredSystem>()->Link(this);

                m_ecsWorld->Add_System<oo::TransformSystem>(this);
                //m_ecsWorld->Get_System<oo::TransformSystem>()->Link(this);

                m_ecsWorld->Add_System<oo::ScriptSystem>(*this, *m_scriptDatabase, *m_componentDatabase);

                //rendering system initialization
                m_ecsWorld->Add_System<oo::MeshRendererSystem>(m_graphicsWorld.get())->Init();

                m_ecsWorld->Add_System<oo::AudioSystem>(this);
            }

            // Broadcast event to load scene
            LoadSceneEvent lse{ this };
            EventManager::Broadcast<LoadSceneEvent>(&lse);

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
            m_ecsWorld->Get_System<oo::AudioSystem>()->Run(m_ecsWorld.get());
            constexpr const char* const scripts_update = "Scripts Update";
            {
                TRACY_PROFILE_SCOPE(scripts_update);
                GetWorld().Get_System<ScriptSystem>()->InvokeForAllEnabled("Update");
                TRACY_PROFILE_SCOPE_END();
            }
            // m_ecsWorld->Get_System<oo::ScriptSystem>()->InvokeForAll("Update");
        }

        PRINT(m_name);
    }
    
    void Scene::LateUpdate()
    {
        m_ecsWorld->Get_System<oo::ScriptSystem>()->InvokeForAll("LateUpdate");
        PRINT(m_name);
    }
    
    void Scene::Render()
    {
        PRINT(m_name);

        GetWorld().Get_System<oo::MeshRendererSystem>()->Run(m_ecsWorld.get());
        
        //VulkanContext* vkContext = reinterpret_cast<VulkanContext*>(Application::Get().GetWindow().GetRenderingContext());
        //vkContext->getRenderer()->SetWorld(m_graphicsWorld.get());
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
            m_ecsWorld->Get_System<oo::DeferredSystem>()->Run(m_ecsWorld.get());

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
        // Broadcast event to unload scene
        UnloadSceneEvent use{ this };
        EventManager::Broadcast<UnloadSceneEvent>(&use);

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
            m_graphicsWorld = std::make_unique<GraphicsWorld>();
            m_scenegraph = std::make_unique<scenegraph>("scenegraph");
            m_rootGo = nullptr;

            m_scriptDatabase = std::make_unique<ScriptDatabase>();
            m_componentDatabase = std::make_unique<ComponentDatabase>(GetID());

            // Creation of root node
            {
                // deferred to initialization after itself exist.
                auto root_handle = m_scenegraph->get_root()->get_handle();
                m_rootGo = std::make_shared<GameObject>(root_handle, *this);
                InsertGameObject(m_rootGo);
                m_rootGo->GetComponent<GameObjectComponent>().Node = m_scenegraph->get_root();
            }

            ASSERT_MSG((!IsValid(*m_rootGo)), "Sanity check, root created should be from this scene.");

            // TODO: Solution To tie graphics world to rendering context for now!
            static VulkanContext* vkContext = Application::Get().GetWindow().GetVulkanContext();
            // comment because cannot 
            vkContext->getRenderer()->SetWorld(m_graphicsWorld.get());

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
            m_graphicsWorld.reset();

            m_scriptDatabase.reset();
            m_componentDatabase.reset();
            
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

    Scene::go_ptr Scene::CreateGameObjectDeferred(UUID uuid)
    {
        LOG_INFO("Creating Deferred Game Object");

        Scene::go_ptr newObjectPtr = std::make_shared<GameObject>(uuid , *this);
        //m_createList.emplace_back(std::make_pair(newObjectPtr, onCreationCallback));
        newObjectPtr = CreateGameObjectImmediate(newObjectPtr);
        // add deferred component and set the entity ID to be itself
        newObjectPtr->AddComponent<DeferredComponent>().entityID = newObjectPtr->GetEntity();
        return newObjectPtr;
    }

    Scene::go_ptr Scene::CreateGameObjectImmediate(UUID uuid)
    {
        Scene::go_ptr newObjectPtr = std::make_shared<GameObject>(uuid, *this);
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
        return go.GetScene() == this && IsValid(GetInstanceID(go));
        //std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [&](std::shared_ptr<oo::GameObject> elem) { return *elem == go; }) != m_gameObjects.end();
    }

    void Scene::DestroyGameObject(GameObject go)
    {
        ASSERT_MSG(IsValid(go) == false, "Working on an invalid GameObject");

        // safety check
        if (FindWithInstanceID(GetInstanceID(go)) == nullptr)
        {
            LOG_CORE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID of {1}", go.GetEntity().value, GetInstanceID(go));
            return;
        }

        // Queue selected node for deletion
        m_removeList.emplace(GetInstanceID(go));

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
        ASSERT_MSG(IsValid(go) == false, "Working on an invalid GameObject");

        Scene::go_ptr target = FindWithInstanceID(GetInstanceID(go));
        // safety check
        if (target == nullptr)
        {
            LOG_CORE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID of {1}", go.GetEntity().value, GetInstanceID(go));
            return;
        }

        // Actual Deletion [Immediate]
        RemoveGameObject(target);
    }

    Scene::go_ptr Scene::InstatiateGameObject(GameObject go)
    {
        return DuplicateGameObject(go);
        /*Scene::go_ptr new_instance = std::make_shared<GameObject>(go.Duplicate());
        return CreateGameObjectImmediate(new_instance);*/
    }

    Scene::go_ptr Scene::DuplicateGameObject(GameObject target)
    {
        ASSERT_MSG(IsValid(target) == false, "Working on an invalid GameObject");
        
        //Recursive Method
        {
            //// we construct a vector of all the newly created objects
            //std::queue<Scene::go_ptr> new_objects;

            //// first we clone the first object [this is what we return to the user]
            //// make a duplicate explicitly
            //Scene::go_ptr dupObjectHead = std::make_shared<GameObject>(*this, target);
            //CreateGameObjectImmediate(dupObjectHead);
            //new_objects.push(dupObjectHead);
            //// we than go through each child object skipping target itself
            //
            //for (auto& curr_obj : target.GetChildren(true))
            //{
            //    // make a duplicate explicitly
            //    Scene::go_ptr newObjectPtr = std::make_shared<GameObject>(*this, curr_obj);
            //    auto go = CreateGameObjectImmediate(newObjectPtr);
            //    new_objects.push(newObjectPtr);
            //}

            //// than we link up their scene graphs using the target's as reference
            //
            //RecusriveLinkScenegraph(target, new_objects);
        }



        // Create Parent node
        Scene::go_ptr dupObjectHead = std::make_shared<GameObject>(*this, target);
        dupObjectHead = CreateGameObjectImmediate(dupObjectHead);
        //auto new_parent = dupObjectHead->GetSceneNode().lock();

        std::stack<scenenode::raw_pointer> s;
        std::stack<scenenode::shared_pointer> parent_stack;
        auto og_scenenode = target.GetSceneNode().lock();
        scenenode::raw_pointer curr = og_scenenode.get(); //m_scenegraph->get_root().get();
        s.push(curr);
        parent_stack.push(dupObjectHead->GetSceneNode().lock());
        while (!s.empty())
        {
            curr = s.top();
            s.pop();

            auto& new_parent = parent_stack.top();
            parent_stack.pop();

            // iterate through original parent's childs
            for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
            {
                scenenode::shared_pointer child = *iter;
                s.push(child.get());

                auto childGO = FindWithInstanceID(child->get_handle());
                Scene::go_ptr dupObjectChild = std::make_shared<GameObject>(*this, *childGO);
                dupObjectChild = CreateGameObjectImmediate(dupObjectChild);

                auto new_child = dupObjectChild->GetSceneNode().lock();
                new_parent->add_child(new_child);
                parent_stack.push(new_child);

            }
        }

        return dupObjectHead;
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
        auto shared_ptr = m_scenegraph->create_new_child(name, GetInstanceID(*newObjectPtr));
        newObjectPtr->GetComponent<GameObjectComponent>().Node = shared_ptr;

        ASSERT_MSG(IsValid(*newObjectPtr) == false, "Sanity check, object created should comply");

        return newObjectPtr;
    }

    void Scene::InsertGameObject(Scene::go_ptr go_ptr)
    {
        m_gameObjects.emplace(go_ptr);
        m_lookupTable.emplace(GetInstanceID(*go_ptr), go_ptr);
    }

    void Scene::RemoveGameObject(Scene::go_ptr go_ptr)
    {
        // remove from scenegraph first. Timing is important here.
        if (auto scenegraph_go = go_ptr->GetSceneNode().lock())
        {
            scenegraph_go->detach();
        }

        m_lookupTable.erase(GetInstanceID(*go_ptr));
        m_gameObjects.erase(go_ptr);

        // actual deletion : Immediate.
        m_ecsWorld->destroy(go_ptr->GetEntity());
    }

    UUID Scene::GetInstanceID(GameObject const& go) const
    {
        return m_ecsWorld->get_component<GameObjectComponent>(go.GetEntity()).Id;
    }

    // Old method
    //void Scene::RecusriveLinkScenegraph(GameObject original_parent_go, std::queue<Scene::go_ptr> new_objects)
    //{
    //    if (new_objects.size() == 0)
    //        return;

    //    auto og_parent = original_parent_go.GetSceneNode().lock();
    //    auto new_parent = new_objects.front()->GetSceneNode().lock();
    //    new_objects.pop();

    //    for (auto& og_child  : original_parent_go.GetChildren(false))
    //    {
    //        if (new_objects.size() == 0)
    //            return;

    //        auto new_child = new_objects.front()->GetSceneNode().lock();
    //        //new_objects.pop();

    //        new_parent->add_child(new_child);
    //        RecusriveLinkScenegraph(og_child, new_objects);
    //    }
    //}
    
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

    GraphicsWorld* Scene::GetGraphicsWorld() const 
    { 
        return m_graphicsWorld.get(); 
    }
}
