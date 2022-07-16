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

namespace oo
{
    /*---------------------------------------------------------------------------------*/
    /* Static Functions                                                                */
    /*---------------------------------------------------------------------------------*/
    //GameObject GameObject::Instantiate(GameObject source)
    //{
    //    return static_cast<GameObject>(WorldManager::GetActiveWorld().DuplicateEntity(source));
    //}

    // Create is a dummy type
    GameObject::GameObject(Scene& scene)
        : GameObject{ UUID{}, scene }
    {
    }

    GameObject::GameObject(UUID uuid, Scene& scene)
        : m_scene { &scene }
        , m_entity{ scene.GetWorld().new_entity<GameObjectComponent>() }
    {
        // Order matters, dont swap it for no reason!
        auto& goComp = m_scene->GetWorld().get_component<GameObjectComponent>(m_entity);
        goComp.Id = uuid;
        //GetComponent<GameObjectComponent>().Id = uuid;
        
        //AddComponent<Transform3D>();
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
        // TODO!
        //GetSceneNode()->add_child(child.GetSceneNode());
        
        // perform some immediate transformation if required.
    }

    //void GameObject::AddChild(std::initializer_list<GameObject> gos, bool preserveTransforms) const
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");

    //    for (auto go : gos)
    //    {
    //        AddChild(go, preserveTransforms);
    //    }
    //}

    //void GameObject::SwapChildren(GameObject const& other)
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");
    //    ASSERT_MSG(IsValid(other), "Working on an invalid Entity");

    //    Swap(*this, other);
    //}

    UUID GameObject::GetParentUUID() const
    {
        auto scenenode = GetSceneNode();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        // parent handle is uuid : behaviours defined in scene when we set this value!
        return scenenode->get_parent_handle();
    }

    std::vector<UUID> GameObject::GetDirectChildsUUID(bool includeItself) const
    {
        auto scenenode = GetSceneNode();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        auto container = scenenode->get_direct_child_handles();
        std::vector<UUID> result;
        for (auto& elem : container)
            result.emplace_back(elem);
        return result;
    }

    std::vector<UUID> GameObject::GetChildrenUUID(bool includeItself) const
    {
        auto scenenode = GetSceneNode();
        ASSERT_MSG((scenenode == nullptr), "Invalid scenenode!");
        auto container = scenenode->get_all_child_handles();
        std::vector<UUID> result;
        for (auto& elem : container)
            result.emplace_back(elem);
        return result;
    }

    void GameObject::SetActive(bool active) const
    {
        ASSERT_MSG(HasComponent<GameObjectComponent>(), "Invalid ID");

        // Set individual active to false
        GetComponent<GameObjectComponent>().Active = active;

        //// Determine state of this object's active state in hierarchy
        //bool hierarchyActive = GetParent().ActiveInHierarchy() && active;
        //CalculateHierarchyActive(*this, hierarchyActive);
    }

    /*void GameObject::SetName(std::string_view name) const
    {
        GetComponent<GameObjectComponent>().Name = name;
    }*/

    //void GameObject::CalculateHierarchyActive(GameObject parent, bool IsActiveInHierarchy) const
    //{
    //    // Determine state of this object's active state in hierarchy
    //    bool hierarchyActive = IsActiveInHierarchy && parent.IsActive();

    //    // Set this object's active state in hierarchy
    //    parent.GetComponent<GameObjectComponent>().SetHierarchyActive(hierarchyActive);

    //    // Set active In Hierarchy for children to be
    //    for (auto& child : parent.GetDirectChilds())
    //    {
    //        CalculateHierarchyActive(child, hierarchyActive);
    //    }
    //}
}