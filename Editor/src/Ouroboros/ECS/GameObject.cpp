/************************************************************************************//*!
\file           GameObject.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes a gameobject which is the interface for everything ECS related

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "GameObject.h"

//#include "Ouroboros/Transform/TransformComponent.h"
//#include "Ouroboros/Transform/TransformSystem.h"

#include "Ouroboros/ECS/GameObjectDebugComponent.h"
#include "Ouroboros/ECS/DuplicatedComponent.h"
#include "Ouroboros/ECS/JustCreatedComponent.h"
#include "Ouroboros/ECS/GameObjectDisabledComponent.h"

#include "Ouroboros/Scripting/ScriptComponent.h"

#include "Ouroboros/EventSystem/EventManager.h"

namespace oo
{
    /*---------------------------------------------------------------------------------*/
    /* Static Functions                                                                */
    /*---------------------------------------------------------------------------------*/
    GameObject GameObject::Duplicate()
    {
        ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
        return *m_scene->DuplicateGameObject(*this);
    }

    // Create is a dummy type
    GameObject::GameObject(Scene& scene)
        : GameObject{ oo::UUID{}, scene }
    {
    }

    GameObject::GameObject(Scene& scene, GameObject& target)
        : m_scene{ &scene }
        , m_entity{ scene.GetWorld().duplicate_entity(target.m_entity) }
    {
        oo::UUID new_uuid {};
        SetupGo(new_uuid, m_entity);
        // mark item as duplicated. duplicated items will be ignored for the first frame to get it properly set up
        m_scene->GetWorld().add_component<oo::DuplicatedComponent>(m_entity);
    }

    GameObject::GameObject(oo::UUID uuid, Scene& scene)
        : m_scene { &scene }
        , m_entity{ scene.GetWorld().new_entity<GameObjectComponent, TransformComponent, ScriptComponent, JustCreatedComponent>() }
    {
        SetupGo(uuid, m_entity);
    }

    //Conversion from entt to gameobject
    GameObject::GameObject(Entity entt, Scene& scene)
        : m_scene{ &scene }
        , m_entity{ entt }
    {
    }

    void GameObject::Destroy()
    {
        ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
        ASSERT_MSG(m_scene->IsValid(*this) == false, " gameobject does not belong to this scene, how did you create this gameobject??");
        m_scene->DestroyGameObject(*this);
    }

    void GameObject::AddChild(GameObject const& child, bool preserveTransforms) const
    {
        TransformComponent& child_tf = child.Transform();
        mat4 og_parent_tf = child.GetParent().Transform().GlobalTransform;
        scenenode::shared_pointer parentNode = GetSceneNode().lock();
        scenenode::shared_pointer childNode = child.GetSceneNode().lock();
        // if parent and child are valid and parent successfully added child
        if (parentNode && childNode && parentNode->add_child(childNode))
        {
            // notify child node that its parent has changed.
            child_tf.GlobalMatrixDirty = true;

            if (preserveTransforms)
            {
                mat4 new_parent_inv = glm::affineInverse(Transform().GlobalTransform.Matrix);
                child_tf.SetLocalTransform(new_parent_inv * og_parent_tf * child_tf.LocalTransform.Matrix);
            }

            // Properly propagate parent gameobject's active downwards
            SetActive(IsActive());
        }
        
    }

    void GameObject::AddChild(std::initializer_list<GameObject> gos, bool preserveTransforms) const
    {
        ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
        ASSERT_MSG(m_scene->IsValid(*this) == false, " gameobject does not belong to this scene, how did you create this gameobject??");

        for (auto go : gos)
        {
            AddChild(go, preserveTransforms);
        }
    }

    bool GameObject::HasChild() const
    {
        return GetChildren().size() != 0;
    }

    bool GameObject::HasValidParent() const
    {
        return TryGetParent() != nullptr;
    }

    std::size_t GameObject::GetChildCount() const
    {
        return GetSceneNode().lock()->get_total_child_count();
    }

    std::size_t GameObject::GetDirectChildCount() const
    {
        return GetSceneNode().lock()->get_direct_child_count();
    }

    std::size_t GameObject::GetComponentCount() const
    {
        return m_scene->GetWorld().get_num_components(m_entity);
    }

    Scene::go_ptr GameObject::TryGetParent() const
    {
        oo::UUID parent_uuid = GetParentUUID();
        return m_scene->FindWithInstanceID(parent_uuid);
    }

    GameObject GameObject::GetParent() const
    {
        oo::UUID parent_uuid = GetParentUUID();
        ASSERT_MSG(parent_uuid == scenenode::NOTFOUND, "this should never happen except for the root node");
        return *m_scene->FindWithInstanceID(parent_uuid);
    }

    std::vector<GameObject> GameObject::GetDirectChilds(bool includeItself) const
    {
        std::vector<GameObject> gos;
        
        for (auto& go : GetDirectChildsUUID(includeItself))
            gos.emplace_back(*m_scene->FindWithInstanceID(go));
        
        return gos;
    }

    std::vector<GameObject> GameObject::GetChildren(bool includeItself) const
    {
        std::vector<GameObject> gos;

        for (auto& go : GetChildrenUUID(includeItself))
            gos.emplace_back(*m_scene->FindWithInstanceID(go));

        return gos;
    }

    //void GameObject::SwapChildren(GameObject const& other)
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");
    //    ASSERT_MSG(IsValid(other), "Working on an invalid Entity");

    //    Swap(*this, other);
    //}

    oo::UUID GameObject::GetParentUUID() const
    {
        auto scenenode = GetSceneNode().lock();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        // parent handle is uuid : behaviours defined in scene when we set this value!
        return scenenode->get_parent_handle();
    }

    std::vector<oo::UUID> GameObject::GetDirectChildsUUID(bool includeItself) const
    {
        auto scenenode = GetSceneNode().lock();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        auto container = scenenode->get_direct_child_handles(includeItself);
        std::vector<oo::UUID> result;
        for (auto& elem : container)
            result.emplace_back(elem);
        return result;
    }

    std::vector<oo::UUID> GameObject::GetChildrenUUID(bool includeItself) const
    {
        auto scenenode = GetSceneNode().lock();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        auto container = scenenode->get_all_child_handles(includeItself);
        std::vector<oo::UUID> result;
        for (auto& elem : container)
            result.emplace_back(elem);
        return result;
    }


    // Helper Getters

    TransformComponent& GameObject::Transform() const 
    { 
        ASSERT_MSG(!HasComponent<TransformComponent>(), "Invalid ID");   
        return GetComponent<TransformComponent>(); 
    }

    bool GameObject::IsActive() const 
    {
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  
        return GetComponent<GameObjectComponent>().Active; 
    }


    bool GameObject::ActiveInHierarchy() const 
    { 
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");
        return GetComponent<GameObjectComponent>().ActiveInHierarchy; 
    }

    // only done after ecs able to return dynamic objects

    std::string& GameObject::Name() const 
    {
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  
        return GetComponent<GameObjectComponent>().Name; 
    }

    oo::UUID GameObject::GetInstanceID() const
    { 
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  
        return GetComponent<GameObjectComponent>().Id; 
    }

    scenenode::weak_pointer GameObject::GetSceneNode() const 
    { 
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  
        return GetComponent<GameObjectComponent>().Node; 
    }

    bool GameObject::GetIsPrefab() const 
    {
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  
        return GetComponent<GameObjectComponent>().IsPrefab; 
    }

    GameObject::Entity GameObject::GetEntity() const 
    { 
        return m_entity; 
    }

    Scene const* GameObject::GetScene() const 
    { 
        return m_scene; 
    }

    void GameObject::SetActive(bool active) const
    {
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");

        // Set individual active to false
        GetComponent<GameObjectComponent>().Active = active;

        //// Determine state of this object's active state in hierarchy
        bool hierarchyActive = active;
        
        if(HasValidParent())
        {
            hierarchyActive &= GetParent().ActiveInHierarchy();
        }

        CalculateHierarchyActive(*this, hierarchyActive);
    }

    void GameObject::SetName(std::string_view name) const
    {
        GetComponent<GameObjectComponent>().Name = name;
        auto scenenode_ptr = GetComponent<GameObjectComponent>().Node.lock();
        scenenode_ptr->set_debug_name(name);
    }

    void GameObject::SetIsPrefab(bool isprefab) const
    {
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");

        // tell everyone that this is a prefab
        GetComponent<GameObjectComponent>().IsPrefab = isprefab;
    }

    void GameObject::SetupGo(oo::UUID uuid, Ecs::EntityID entt)
    {
        // add debugging component
#if not defined OO_PRODUCTION
        oo::GameObjectDebugComponent comp{ this };
        m_scene->GetWorld().add_component<oo::GameObjectDebugComponent>(entt, comp);
#endif
        auto& goComp = m_scene->GetWorld().get_component<GameObjectComponent>(entt);
        goComp.Id = uuid;
        auto& jcComp = m_scene->GetWorld().get_component<JustCreatedComponent>(entt);
        jcComp.uuid = uuid;
        jcComp.entityID = entt;
    }

    void GameObject::SetHierarchyActive(GameObjectComponent& comp, bool active) const
    {
        if (comp.ActiveInHierarchy != active)
        {
            // check previous state to do appropriate callback
            bool previousState = comp.ActiveInHierarchy;
            if (previousState)
            {
                // if was active, call the disable event
                GameObjectComponent::OnDisableEvent onDisableEvent{ comp.Id };
                oo::EventManager::Broadcast(&onDisableEvent);
                //LOG_CORE_INFO("GameObjectComponent OnDisable Invoke");

            }
            else
            {
                // if was inactive, call the enable event
                GameObjectComponent::OnEnableEvent onEnableEvent{ comp.Id };
                oo::EventManager::Broadcast(&onEnableEvent);
                //LOG_CORE_INFO("GameObjectComponent OnEnable Invoke");
            }
            
            comp.ActiveInHierarchy = active;
        }
    }

    void GameObject::CalculateHierarchyActive(GameObject parent, bool IsActiveInHierarchy) const
    {
        // Determine state of this object's active state in hierarchy
        bool hierarchyActive = IsActiveInHierarchy && parent.IsActive();

        // Set this object's active state in hierarchy
        SetHierarchyActive(parent.GetComponent<GameObjectComponent>(), hierarchyActive);

        if (parent.ActiveInHierarchy())
        {
            // let's remove gameobject disabled component (since its now active)
            parent.TryRemoveComponent<GameObjectDisabledComponent>();
        }
        else
        {
            // let's add gameobject disabled component (since its now inactive)
            parent.EnsureComponent<GameObjectDisabledComponent>();
        }

        // Set active In Hierarchy for children to be
        for (auto& child : parent.GetDirectChilds())
        {
            CalculateHierarchyActive(child, hierarchyActive);
        }
    }

    bool GameObject::operator==(GameObject rhs)
    {
        return m_entity.value == rhs.GetEntity().value && m_scene == rhs.m_scene;
    }

    // equality comparison
    bool GameObject::operator<(GameObject rhs)
    {
        return m_entity.value < rhs.m_entity.value && m_scene == rhs.m_scene;
    }

    // equality comparison
    bool GameObject::operator>(GameObject rhs)
    {
        return !(rhs < *this);
    }
}