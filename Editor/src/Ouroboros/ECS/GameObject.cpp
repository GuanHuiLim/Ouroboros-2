/************************************************************************************//*!
\file           GameObject.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2021
\brief          Describes a gameobject which is the interface for everything ECS related

Copyright (C) 2021 DigiPen Institute of Technology.
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
    /* Static Variables                                                                */
    /*---------------------------------------------------------------------------------*/
    std::unordered_map<UUID, GameObject::Entity> GameObject::s_lookupTable;

    //std::set<std::pair<GameObject::Entity, UUID>> GameObject::s_detachList; // List of head entities to applied pre-deletion step

    std::set<std::pair<GameObject::Entity, UUID>> GameObject::s_removeList; // List of gameobjects to remove

    //std::unordered_map<std::string, LayerField> GameObject::s_layersTable;

    /*---------------------------------------------------------------------------------*/
    /* Static Functions                                                                */
    /*---------------------------------------------------------------------------------*/
    //GameObject GameObject::Instantiate(GameObject source)
    //{
    //    return static_cast<GameObject>(WorldManager::GetActiveWorld().DuplicateEntity(source));
    //}

    void GameObject::Destroy(GameObject go)
    {
        ASSERT_MSG(IsValid(go), "Working on an invalid GameObject");

        // safety check
        /*if (FindWithInstanceID(go.GetInstanceID()) == false)
        {
            LOG_CORE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID od {1}", go, go.GetInstanceID());
            return;
        }*/

        // Queue selected node for deletion
        //s_removeList.emplace(go.GetEntity(), go.GetInstanceID());

        //// Then continue to destroy all its children
        //for (auto const& child : go.GetChildren())
        //{
        //    Destroy(child);
        //}
    }

    //void GameObject::Destroy(GameObject go, std::vector<std::shared_ptr<oo::DeletedGameObject>>& deleteGos)
    //{
    //    // Store root entity
    //    s_detachList.emplace(go.GetEntity(), go.GetInstanceID());

    //    DestroyAndSave(go, deleteGos);
    //}

    //void GameObject::DestroyAndSave(GameObject go, std::vector<std::shared_ptr<oo::DeletedGameObject>>& deleteGos)
    //{
    //    if (FindWithInstanceID(go.GetInstanceID()) == false)
    //    {
    //        LOG_ENGINE_ERROR("Attempting to remove an invalid gameobject {0} with instance ID od {1}", go, go.GetInstanceID());
    //        return;
    //    }

    //    // Queue selected node for deletion
    //    s_removeList.emplace(go.GetEntity(), go.GetInstanceID());

    //    // should only be performed in editor mode.
    //    deleteGos.emplace_back(WorldManager::GetActiveWorld().StoreAsDeleted(go));

    //    // Then continue to destroy all its children
    //    for (auto const& child : go.GetDirectChilds())
    //    {
    //        DestroyAndSave(child, deleteGos);
    //    }
    //}

    //GameObject GameObject::Restore(std::vector<std::shared_ptr<oo::DeletedGameObject>> deletedGos)
    //{
    //    bool doOnce = true;
    //    GameObject rootRestore;
    //    for (auto& redoGo : deletedGos)
    //    {
    //        auto ent = WorldManager::GetActiveWorld().RestoreFromDeleted(*redoGo);
    //        if (doOnce)
    //        {
    //            // no choice gotta do it here
    //            rootRestore = static_cast<GameObject>(ent);
    //            rootRestore.Transform().OnAttach();
    //            doOnce = false;
    //        }
    //        s_lookupTable.emplace(static_cast<GameObject>(ent).GetInstanceID(), ent);
    //    }

    //    //WorldManager::GetActiveWorld().GetSystem<oo::TransformSystem>()->UseDenseArrayAsHierarchy();
    //    WorldManager::GetActiveWorld().GetSystem<oo::TransformSystem>()->ReconstructHierarchy();

    //    return rootRestore;
    //}

    //void GameObject::ProcessDeletion()
    //{
    //    for (auto& [go, id] : s_detachList)
    //    {
    //        ASSERT_MSG
    //        (
    //            (GameObject::FindWithInstanceID(id) != GameObject::NOTFOUND),
    //            "Attempting to perform pre-deletion on an object that's already been removed"
    //        );

    //        // Actual Pre-Deletion Step
    //        // no choice gotta do it here. Only perform this once on the selected node.
    //        GameObject{ go }.Transform().OnDetach();
    //    }
    //    s_detachList.clear();

    //    for (auto& [go, id] : s_removeList)
    //    {
    //        ASSERT_MSG
    //        (
    //            (GameObject::FindWithInstanceID(id) != GameObject::NOTFOUND),
    //            "Attempting to delete an object that's already been removed"
    //        );

    //        // Actual Deletion
    //        s_lookupTable.erase(id);
    //        WorldManager::GetActiveWorld().DestroyEntity(go);
    //    }
    //    s_removeList.clear();
    //}

    //void GameObject::Swap(GameObject childA, GameObject childB)
    //{
    //    ENGINE_ASSERT_CUSTOM_MSG(IsValid(childA), "Working on an invalid Entity {0}", childA);
    //    ENGINE_ASSERT_CUSTOM_MSG(IsValid(childB), "Working on an invalid Entity {0}", childB);

    //    if (childA == GameObject::ROOT || childB == GameObject::ROOT)
    //    {
    //        LOG_ENGINE_ERROR("Swapping with the root is not allowed!");
    //        return;
    //    }

    //    if (childA == childB)
    //    {
    //        LOG_ENGINE_ERROR("Swapping with yourself does nothing!");
    //        return;
    //    }

    //    if (childA.Transform().GetParentUUID() != childB.Transform().GetParentUUID())
    //    {
    //        LOG_ENGINE_ERROR("Attempting to swap two gameobjects ({0},{1}) that doesnt have the same parent!", childA, childB);
    //        LOG_ENGINE_ERROR("Function not currently Supported!");
    //        return;
    //    }

    //    auto tfSystem = WorldManager::GetActiveWorld().GetSystem<TransformSystem>();
    //    tfSystem->SwapChild(childA, childB);
    //    tfSystem->ReconstructHierarchy();
    //}

    //GameObject GameObject::FindWithInstanceID(UUID uuid)
    //{
    //    if (s_lookupTable.find(uuid) != s_lookupTable.end())
    //        return s_lookupTable[uuid];

    //    return NOTFOUND;
    //}

    //void GameObject::AddLayer(std::string const& name)
    //{
    //    if (s_layersTable.find(name) != s_layersTable.end())
    //    {
    //        LOG_ENGINE_INFO("Already have layer of name {0}", name);
    //    }
    //    else if (s_layersTable.size() >= MAX_LAYER_COUNT)
    //    {
    //        LOG_ENGINE_INFO("Maximum number of layers reached, please remove some before adding", name);
    //    }
    //    else
    //    {
    //        s_layersTable.emplace(name, LayerField{ 1ull << s_layersTable.size() });
    //    }
    //}

    //void GameObject::RemoveLayer(std::string const& name)
    //{
    //    if (s_layersTable.find(name) == s_layersTable.end())
    //    {
    //        LOG_ENGINE_ERROR("Attempting to remove non-existent layer of name {0}", name);
    //    }
    //    else
    //    {
    //        s_layersTable.erase(name);
    //    }
    //}



    // Create is a dummy type
    GameObject::GameObject(Ecs::ECSWorld* ecsWorld)
        : GameObject{ UUID{}, ecsWorld }
    {
    }

    GameObject::GameObject(UUID uuid, Ecs::ECSWorld* ecsWorld)
        : m_ecsworld { ecsWorld }
        , m_entity{ ecsWorld->new_entity() }
    {
        // Order matters, dont swap it for no reason!
        //AddComponent<GameObjectComponent>().ID = uuid;
        //AddComponent<Transform3D>();
        s_lookupTable.emplace(uuid, m_entity);
    }

    GameObject::GameObject(Entity entt, Ecs::ECSWorld* ecsWorld)
        : m_ecsworld { ecsWorld }
        , m_entity{ entt }
    {
    }

    bool GameObject::IsValid(GameObject go)
    {
        // a valid gameobject will not have its ID as NotFound, will have gameobject component(minimally)
        // and parent will not be equals to NOParent
        //return !(go == NOTFOUND || go.HasComponent<GameObjectComponent>() == false || go.Transform().GetParentUUID() == NOPARENT);

        return !(go.GetEntity().value == GameObject::NOTFOUND.value || go.GetWorld() == nullptr);
    }

    void GameObject::Destroy()
    {
        ASSERT_MSG(IsValid(*this), "Working on an invalid Entity");

        // Differ this to later Store root entity
        //s_detachList.emplace(GetEntity(), GetInstanceID());

        Destroy(*this);
    }

    //void GameObject::AddChild(GameObject const& child, bool preserveTransforms) const
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");
    //    //ASSERT_MSG(IsValid(child), "Working on an invalid child");

    //    WorldManager::GetActiveWorld().GetSystem<oo::TransformSystem>()->Attach(child, this->m_entity, preserveTransforms);

    //    // Properly propagate parent gameobject's active downwards
    //    SetActive(IsActive());
    //}

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

    //GameObject GameObject::GetParent() const
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");
    //    return static_cast<GameObject>(Transform().GetParentId());
    //}

    //std::vector<Entity> GameObject::GetDirectChilds(bool includeItself) const
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");

    //    auto const& childTfs = WorldManager::GetActiveWorld().GetSystem<oo::TransformSystem>()->GetDirectChildren(Transform());

    //    std::vector<Entity> entities;
    //    entities.reserve(childTfs.size() + includeItself);

    //    if (includeItself)
    //        entities.emplace_back(m_entity);

    //    for (auto const& childTf : childTfs)
    //        entities.emplace_back(childTf.GetEntity());

    //    return entities;
    //}

    //std::vector<Entity> GameObject::GetChildren(bool includeItself) const
    //{
    //    ASSERT_MSG(IsValid(m_entity), "Working on an invalid Entity");

    //    auto const& childTfs = WorldManager::GetActiveWorld().GetSystem<oo::TransformSystem>()->GetChildren(Transform());
    //    std::vector<Entity> entities;
    //    entities.reserve(childTfs.size() + includeItself);

    //    if (includeItself)
    //        entities.emplace_back(m_entity);

    //    for (auto const& childTf : childTfs)
    //        entities.emplace_back(childTf.GetEntity());

    //    return entities;
    //}

    //void GameObject::SetActive(bool active) const
    //{
    //    ASSERT_MSG(HasComponent<GameObjectComponent>(), "Invalid ID");

    //    // Set individual active to false
    //    GetComponent<GameObjectComponent>().Active = active;

    //    // Determine state of this object's active state in hierarchy
    //    bool hierarchyActive = GetParent().ActiveInHierarchy() && active;
    //    CalculateHierarchyActive(*this, hierarchyActive);
    //}

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