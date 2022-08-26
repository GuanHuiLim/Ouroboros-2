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

//#include "Ouroboros/Transform/Transform3D.h"
//#include "Ouroboros/Transform/TransformSystem.h"

#include "Ouroboros/ECS/GameObjectDebugComponent.h"

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
        : GameObject{ UUID{}, scene }
    {
    }

    GameObject::GameObject(Scene& scene, GameObject& target)
        : m_scene{ &scene }
        , m_entity{ scene.GetWorld().duplicate_entity(target.m_entity) }
    {
        UUID new_uuid {};
        SetupGo(new_uuid, m_entity);
    }

    GameObject::GameObject(UUID uuid, Scene& scene)
        : m_scene { &scene }
        , m_entity{ scene.GetWorld().new_entity<GameObjectComponent, Transform3D>() }
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
        // TODO! [preserve transforms needs to be used]
        scenenode::shared_pointer parentNode = GetSceneNode().lock();
        scenenode::shared_pointer childNode = child.GetSceneNode().lock();
        if (parentNode && childNode)
        {
            parentNode->add_child(childNode);
            // notify child node that its parent has changed.
            child.Transform().ParentChanged();
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

    GameObject GameObject::GetParent() const
    {
        return *m_scene->FindWithInstanceID(GetParentUUID());
    }

    std::vector<GameObject> GameObject::GetDirectChilds(bool includeItself) const
    {
        std::vector<GameObject> gos;
        
        for (auto& go : GetDirectChildsUUID())
            gos.emplace_back(*m_scene->FindWithInstanceID(go));
        
        return gos;
    }

    std::vector<GameObject> GameObject::GetChildren(bool includeItself) const
    {
        std::vector<GameObject> gos;

        for (auto& go : GetChildrenUUID())
            gos.emplace_back(*m_scene->FindWithInstanceID(go));

        return gos;
    }

    //void GameObject::SwapChildren(GameObject const& other)
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");
    //    ASSERT_MSG(IsValid(other), "Working on an invalid Entity");

    //    Swap(*this, other);
    //}

    UUID GameObject::GetParentUUID() const
    {
        auto scenenode = GetSceneNode().lock();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        // parent handle is uuid : behaviours defined in scene when we set this value!
        return scenenode->get_parent_handle();
    }

    std::vector<UUID> GameObject::GetDirectChildsUUID(bool includeItself) const
    {
        auto scenenode = GetSceneNode().lock();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        auto container = scenenode->get_direct_child_handles();
        std::vector<UUID> result;
        for (auto& elem : container)
            result.emplace_back(elem);
        return result;
    }

    std::vector<UUID> GameObject::GetChildrenUUID(bool includeItself) const
    {
        auto scenenode = GetSceneNode().lock();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        auto container = scenenode->get_all_child_handles();
        std::vector<UUID> result;
        for (auto& elem : container)
            result.emplace_back(elem);
        return result;
    }

    void GameObject::SetActive(bool active) const
    {
        ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");

        // Set individual active to false
        GetComponent<GameObjectComponent>().Active = active;

        //// Determine state of this object's active state in hierarchy
        bool hierarchyActive = GetParent().ActiveInHierarchy() && active;
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

    void GameObject::SetupGo(UUID uuid, Ecs::EntityID entt)
    {
        // add debugging component
#if not define OO_PRODUCTION
        oo::GameObjectDebugComponent comp{ this };
        m_scene->GetWorld().add_component<oo::GameObjectDebugComponent>(m_entity, comp);
#endif
        auto& goComp = m_scene->GetWorld().get_component<GameObjectComponent>(m_entity);
        goComp.Id = uuid;
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
                GameObjectComponent::OnDisableEvent onDisableEvent;
                oo::EventManager::Broadcast(&onDisableEvent);
                LOG_CORE_INFO("GameObjectComponent OnDisable Invoke");
            }
            else
            {
                // if was inactive, call the enable event
                GameObjectComponent::OnEnableEvent onEnableEvent;
                oo::EventManager::Broadcast(&onEnableEvent);
                LOG_CORE_INFO("GameObjectComponent OnEnable Invoke");
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

        // Set active In Hierarchy for children to be
        for (auto& child : parent.GetDirectChilds())
        {
            CalculateHierarchyActive(child, hierarchyActive);
        }
    }
}