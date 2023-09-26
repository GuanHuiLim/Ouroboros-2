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
#include "Project.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Core/Log.h"

#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/Transform/TransformSystem.h"

#include "App/Editor/Events/LoadSceneEvent.h"
#include "App/Editor/Events/UnloadSceneEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/ECS/DeferredSystem.h"
#include "Ouroboros/ECS/DuplicatedSystem.h"
#include "Ouroboros/ECS/JustCreatedSystem.h"

#include "Ouroboros/Scripting/ScriptSystem.h"

#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Vulkan/VulkanContext.h"

#include "Ouroboros/Vulkan/RendererSystem.h"
#include "Ouroboros/Vulkan/SkinRendererSystem.h"
#include "Ouroboros/Vulkan/ParticleRendererSystem.h"
#include "Ouroboros/UI/UIComponent.h"

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
        //, m_graphicsWorld { nullptr } TODO: temporarily have one static world
        , m_ecsWorld { nullptr }
        , m_scenegraph { nullptr }
        , m_rootGo { nullptr }
        , m_mainCamera { nullptr }
    { 
        EventManager::Subscribe<Scene, GameObjectComponent::OnEnableEvent>(this, &Scene::OnEnableGameObject);
        EventManager::Subscribe<Scene, GameObjectComponent::OnDisableEvent>(this, &Scene::OnDisableGameObject);
    }

    Scene::~Scene()
    {
        EventManager::Unsubscribe<Scene, GameObjectComponent::OnEnableEvent>(this, &Scene::OnEnableGameObject);
        EventManager::Unsubscribe<Scene, GameObjectComponent::OnDisableEvent>(this, &Scene::OnDisableGameObject);
    }

    void Scene::Init()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_init, tracy::Color::Seashell1);
        OPTICK_EVENT();
        Scene::OnInitEvent e;
        EventManager::Broadcast(&e);

        // Load assets
        //Project::GetAssetManager()->Load stuff
        
        // Initialize Default Systems
        {
            m_ecsWorld->Add_System<oo::JustCreatedSystem>(this);
            m_ecsWorld->Add_System<oo::DeferredSystem>(this);
            m_ecsWorld->Add_System<oo::DuplicatedSystem>(this);
            m_ecsWorld->Add_System<oo::TransformSystem>(this);
            m_ecsWorld->Add_System<oo::ScriptSystem>(*this, *m_scriptDatabase, *m_componentDatabase);
            m_ecsWorld->Add_System<oo::AudioSystem>(this)->Init();
            //rendering system initialization
            // temporarily initialize number of cameras to 2 [0 is Scene camera] [1 is Editor Camera]
#if defined OO_EXECUTABLE
            m_graphicsWorld->numCameras = 1;
#elif OO_EDITOR
            m_graphicsWorld->numCameras = 2;
#endif
            m_ecsWorld->Add_System<oo::RendererSystem>(m_graphicsWorld.get(), this)->Init();
            m_ecsWorld->Add_System<oo::ParticleRendererSystem>(m_graphicsWorld.get(), this)->Init();
            Application::Get().GetWindow().GetVulkanContext()->getRenderer()->InitWorld(m_graphicsWorld.get());
            m_ecsWorld->Add_System<oo::SkinMeshRendererSystem>(m_graphicsWorld.get(), this)->Init();
        }

        PRINT(m_name);
            
        TRACY_PROFILE_SCOPE_END();
    }
    
    void Scene::Update()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_update, tracy::Color::Seashell2);
        OPTICK_EVENT();

        PRINT(m_name);

        TRACY_PROFILE_SCOPE_END();
    }
    
    void Scene::LateUpdate()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_late_update, tracy::Color::Seashell3);
        OPTICK_EVENT();

        m_ecsWorld->Get_System<oo::RendererSystem>()->UpdateCameras(m_mainCamera);
        PRINT(m_name);

        TRACY_PROFILE_SCOPE_END();
    }
    
    void Scene::Render()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_rendering, tracy::Color::Seashell3);
        OPTICK_EVENT();

        GetWorld().Get_System<oo::RendererSystem>()->Run(m_ecsWorld.get());
        GetWorld().Get_System<oo::SkinMeshRendererSystem>()->Run(m_ecsWorld.get());
        GetWorld().Get_System<oo::ParticleRendererSystem>()->Run(m_ecsWorld.get());
        PRINT(m_name);
        
        TRACY_PROFILE_SCOPE_END();
    }
    
    void Scene::EndOfFrameUpdate()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_end_of_frame_update, tracy::Color::Seashell4);
        OPTICK_EVENT();

        PRINT(m_name);

        //// go through all things to create and add at the end of frame and do so.
        //for (auto& [go, callback] : m_createList)
        //{
        //    CreateGameObjectImmediate(go);
        //    callback(go);
        //}
        //m_createList.clear();
        
        m_ecsWorld->Get_System<oo::DuplicatedSystem>()->Run(m_ecsWorld.get());
        m_ecsWorld->Get_System<oo::DeferredSystem>()->Run(m_ecsWorld.get());
        m_ecsWorld->Get_System<oo::JustCreatedSystem>()->Run(m_ecsWorld.get());

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
    
    void Scene::Exit()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_exit, tracy::Color::Seashell3);
        OPTICK_EVENT();

        // Broadcast event to unload scene
        UnloadSceneEvent use{ this };
        EventManager::Broadcast<UnloadSceneEvent>(&use);

        PRINT(m_name);
        
        TRACY_PROFILE_SCOPE_END();
    }
    
    void Scene::LoadScene()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_load_scene, tracy::Color::Seashell3);
        OPTICK_EVENT();

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
                
            ASSERT_MSG((!IsValid(*m_rootGo)), "Sanity check, root created should be from this scene.");
        }

        // TODO: Solution To tie graphics world to rendering context for now!
        static VulkanContext* vkContext = Application::Get().GetWindow().GetVulkanContext();
        vkContext->getRenderer()->SetWorld(m_graphicsWorld.get());

        TRACY_PROFILE_SCOPE_END();
    }
    
    void Scene::UnloadScene()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_unload_scene, tracy::Color::Seashell4);
        OPTICK_EVENT();

        PRINT(m_name);

        GetWorld().Get_System<oo::RendererSystem>()->SaveEditorCamera();
        EndOfFrameUpdate();

        // TODO: Temporarily remove destroying the world on load
        m_graphicsWorld->ClearLightInstances();
        m_graphicsWorld->ClearObjectInstances();
        m_graphicsWorld->ClearEmitterInstances();
        m_graphicsWorld->ClearUIInstances();
        m_pickingIdToUUID.clear();
        m_uuidToPickingId.clear();

        // kill the graphics world
        Application::Get().GetWindow().GetVulkanContext()->getRenderer()->DestroyWorld(m_graphicsWorld.get());

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
    
    void Scene::ReloadScene()
    {
        OPTICK_EVENT();
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

    Scene::go_ptr Scene::CreateGameObjectDeferred(oo::UUID uuid)
    {
        //LOG_INFO("Creating Deferred Game Object");

        Scene::go_ptr newObjectPtr = std::make_shared<GameObject>(uuid , *this);
        //m_createList.emplace_back(std::make_pair(newObjectPtr, onCreationCallback));
        newObjectPtr = CreateGameObjectImmediate(newObjectPtr);
        // add deferred component and set the entity ID to be itself
        newObjectPtr->AddComponent<DeferredComponent>().entityID = newObjectPtr->GetEntity();
        return newObjectPtr;
    }

    Scene::go_ptr Scene::CreateGameObjectImmediate(oo::UUID uuid)
    {
        Scene::go_ptr newObjectPtr = std::make_shared<GameObject>(uuid, *this);
        return CreateGameObjectImmediate(newObjectPtr);
    }

    Scene::go_ptr Scene::FindWithInstanceID(oo::UUID uuid) const
    {
        //LOG_INFO("Finding gameobject of instance ID {0}", uuid);

        if (m_lookupTable.contains(uuid))
            return m_lookupTable.at(uuid);

        return nullptr;
    }

    bool Scene::IsValid(oo::UUID uuid) const
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

        // Then continue to queue destruction for all its children
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

        TRACY_PROFILE_SCOPE_NC(duplicate_gameobject, tracy::Color::Seashell4);
        OPTICK_EVENT();

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

            scenenode::shared_pointer new_parent = parent_stack.top();
            parent_stack.pop();

            // iterate through original parent's childs
            for (auto iter = curr->begin(); iter != curr->end(); ++iter)
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
        
        TRACY_PROFILE_SCOPE_END();

        return dupObjectHead;
    }
    
    void Scene::LoadFromFile()
    {
        TRACY_PROFILE_SCOPE_NC(base_scene_load_from_file, tracy::Color::Seashell4);
        OPTICK_EVENT();

        // Broadcast event to load scene
        LoadSceneEvent lse{ this };
        EventManager::Broadcast<LoadSceneEvent>(&lse);

        // Set Active Event for all objects
        for (auto& go : m_gameObjects)
        {
            if (go->ActiveInHierarchy())
            {
                GameObjectComponent::OnEnableEvent goOnEnableEvent{ go->GetInstanceID() };
                EventManager::Broadcast<GameObjectComponent::OnEnableEvent>(&goOnEnableEvent);
            }
            else
            {
                go->EnsureComponent<GameObjectDisabledComponent>();

                GameObjectComponent::OnDisableEvent goOnDisableEvent{ go->GetInstanceID() };
                EventManager::Broadcast<GameObjectComponent::OnDisableEvent>(&goOnDisableEvent);
            }
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void Scene::SaveToFile()
    {
    }

    Scene::go_ptr Scene::CreateGameObjectImmediate(Scene::go_ptr new_go)
    {
        TRACY_PROFILE_SCOPE_NC(create_gameobject_immediate, tracy::Color::Seashell4);
        OPTICK_EVENT();

        Scene::go_ptr newObjectPtr = new_go;
        InsertGameObject(newObjectPtr);

        // IMPT: we are using the uuid to retrieve back the gameobject as well!
        auto& name = newObjectPtr->Name();
        //auto name = "Just a fake default name for now until ecs is fixed";
        auto shared_ptr = m_scenegraph->create_new_child(name, GetInstanceID(*newObjectPtr));
        newObjectPtr->GetComponent<GameObjectComponent>().Node = shared_ptr;

        ASSERT_MSG(IsValid(*newObjectPtr) == false, "Sanity check, object created should comply");

        TRACY_PROFILE_SCOPE_END();

        return newObjectPtr;
    }

    void Scene::InsertGameObject(Scene::go_ptr go_ptr)
    {
        m_gameObjects.emplace(go_ptr);
        m_lookupTable.emplace(GetInstanceID(*go_ptr), go_ptr);
    }

    void Scene::RemoveGameObject(Scene::go_ptr go_ptr)
    {
        ASSERT(go_ptr == nullptr);
        
        TRACY_PROFILE_SCOPE_NC(remove_gameobject, tracy::Color::Seashell4);
        OPTICK_EVENT();

        // one final broadcast to cleanup anything you need to
        GameObject::OnDestroy e;
        e.go = go_ptr.get();
        EventManager::Broadcast<GameObject::OnDestroy>(&e);

        // remove from scenegraph first. Timing is important here.
        if (auto scenegraph_go = go_ptr->GetSceneNode().lock())
        {
            scenegraph_go->detach();
        }

        m_lookupTable.erase(GetInstanceID(*go_ptr));
        m_gameObjects.erase(go_ptr);

        // actual deletion : Immediate.
        m_ecsWorld->destroy(go_ptr->GetEntity());

        TRACY_PROFILE_SCOPE_END();
    }

    oo::UUID Scene::GetInstanceID(GameObject const& go) const
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

    void Scene::OnEnableGameObject(GameObjectComponent::OnEnableEvent* e)
    {
        // graphics specific gameobject detected.
        if (m_uuidToPickingId.contains(e->Id))
        {
            // On Enable we have to check!
            auto go = FindWithInstanceID(e->Id);
            bool renderObject = go->IsActive();

            // if its not UI, it must be mesh or skin mesh
            if (!go->HasComponent<UIComponent>())
            {
                bool castShadows = false;
                
                int32_t graphicsID = -1;
                if (go->HasComponent<MeshRendererComponent>())
                {
                    auto const& comp = go->GetComponent<MeshRendererComponent>();
                    graphicsID = comp.GraphicsWorldID;
                    castShadows = comp.CastShadows;
                }
                else if (go->HasComponent<SkinMeshRendererComponent>())
                {
                    auto const& comp = go->GetComponent<SkinMeshRendererComponent>();
                    graphicsID = comp.graphicsWorld_ID;
                    castShadows = comp.CastShadows;
                }

                auto& actualObject = m_graphicsWorld->GetObjectInstance(graphicsID);
                actualObject.SetRenderEnabled(renderObject);
                actualObject.SetShadowEnabled(castShadows);
                
            }
            else if (go->HasComponent<UIComponent>())
            {
                // ui not supported right now.
                auto& ui = m_graphicsWorld->GetUIInstance(go->GetComponent<UIComponent>().UI_ID);
                //ui.SetRenderEnabled()
                ui.SetRenderEnabled(renderObject);
            }

        }
    }

    void Scene::OnDisableGameObject(GameObjectComponent::OnDisableEvent* e)
    {
        // graphics specific logic
        if (m_uuidToPickingId.contains(e->Id))
        {
            // On Disable we get go
            auto go = FindWithInstanceID(e->Id);

            // if its not UI, it must be mesh or skin mesh
            if (!go->HasComponent<UIComponent>())
            {
                int32_t graphicsID = -1;
                if (go->HasComponent<MeshRendererComponent>())
                {
                    auto const& comp = go->GetComponent<MeshRendererComponent>();
                    graphicsID = comp.GraphicsWorldID;
                }
                else if (go->HasComponent<SkinMeshRendererComponent>())
                {
                    auto const& comp = go->GetComponent<SkinMeshRendererComponent>();
                    graphicsID = comp.graphicsWorld_ID;
                }

                auto& actualObject = m_graphicsWorld->GetObjectInstance(graphicsID);
                actualObject.SetRenderEnabled(false);
                actualObject.SetShadowEnabled(false);

            }
            else if (go->HasComponent<UIComponent>())
            {
                // ui not supported right now.
                auto& ui = m_graphicsWorld->GetUIInstance(go->GetComponent<UIComponent>().UI_ID);
                //ui.SetRenderEnabled()
                ui.SetRenderEnabled(false);
            }
        }
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

    GraphicsWorld* Scene::GetGraphicsWorld() const 
    { 
        return m_graphicsWorld.get(); 
    }
    
    Scene::go_ptr Scene::GetMainCameraObject() const
    {
        return m_mainCamera;
    }

    Camera Scene::MainCamera() const
    {
        return m_graphicsWorld->cameras[m_mainCamera->GetComponent<CameraComponent>().GraphicsWorldIndex];
    }

    UUID Scene::GetUUIDFromPickingId(std::uint32_t pickingID) const
    {
        if (m_pickingIdToUUID.contains(pickingID))
            return m_pickingIdToUUID.at(pickingID);

        return UUID::Invalid;
    }

    std::uint32_t Scene::GeneratePickingID(UUID uuid)
    {
        // ensure theres enough ids to generate
        ASSERT_MSG(m_pickingIdToUUID.size() == std::numeric_limits<std::uint16_t>::max(), " ran out of ids, oh no!");

        // need to find a way to generate a 32 bit picking ID uniquely
        std::uint32_t pickingID = static_cast<std::uint32_t>(UUID16{});
        while (m_pickingIdToUUID.contains(pickingID))
        {
            pickingID = static_cast<std::uint32_t>(UUID16{});
        }
        
        ASSERT_MSG(m_pickingIdToUUID.contains(pickingID) == true, " this picking id should be new!");
        ASSERT_MSG(m_uuidToPickingId.contains(uuid) == true, " this uuid should not already exist");
        
        // map graphics id to uuid of gameobject
        m_pickingIdToUUID.insert({ pickingID, uuid });
        m_uuidToPickingId.insert({ uuid, pickingID });
        
        return pickingID;
    }

    void Scene::RemovePickingID(std::uint32_t pickingID)
    {
        ASSERT_MSG(m_pickingIdToUUID.contains(pickingID) == false, " this picking id should exist! Did you use the id created from the generate function above?");
        
        auto& uuid = m_pickingIdToUUID.at(pickingID);
        ASSERT_MSG(m_uuidToPickingId.contains(uuid) == false, " this uuid should exist");

        // remove graphics id to uuid of gameobject
        m_pickingIdToUUID.erase(pickingID);
        m_uuidToPickingId.erase(uuid);
    }
}
