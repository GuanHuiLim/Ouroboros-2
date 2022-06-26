/************************************************************************************//*!
\file           GameObject.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2021
\brief          Describes a gameobject which is the basic unit and building block for
                every scene. This object provides the ability to tap on existing ECS
                systems and the ability to have scenegraph.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <A_Ecs.h>
#include <EcsUtils.h>

#include "Ouroboros/Core/Base.h"
#include "Utility/UUID.h"

#include <set>

namespace oo
{
    // Note Gameobject is no longer directly convertible to Entity.
    class GameObject final
    {
    public:
        using Entity = Ecs::EntityID;

        static constexpr uint64_t   NOPARENT = std::numeric_limits<uint64_t>::max();
        static constexpr Entity     NOTFOUND = std::numeric_limits<Entity>::max();

        static constexpr uint64_t   ROOTID  = std::numeric_limits<uint64_t>::min();
        static constexpr Entity     ROOT    = std::numeric_limits<Entity>::min();

    private:
        Ecs::ECSWorld* m_ecsworld = nullptr;
        Entity m_entity = NOTFOUND; //default to not found

    /*---------------------------------------------------------------------------------*/
    /* Public Functions                                                                */
    /*---------------------------------------------------------------------------------*/
    public:
        // Helper Getters
        /*Transform3D& Transform()    const { ASSERT_MSG(HasComponent<Transform3D>(), "Invalid ID"); return GetComponent<Transform3D>(); };
        bool IsActive()             const { ASSERT_MSG(HasComponent<GameObjectComponent>(), "Invalid ID"); return GetComponent<GameObjectComponent>().Active; }
        bool ActiveInHierarchy()    const { ASSERT_MSG(HasComponent<GameObjectComponent>(), "Invalid ID"); return GetComponent<GameObjectComponent>().GetActiveInHierarchy(); }
        std::string& Name()         const { ASSERT_MSG(HasComponent<GameObjectComponent>(), "Invalid ID"); return GetComponent<GameObjectComponent>().Name; }
        UUID GetInstanceID()        const { ASSERT_MSG(HasComponent<GameObjectComponent>(), "Invalid ID"); return GetComponent<GameObjectComponent>().ID; }
        LayerField GetLayer()       const { ASSERT_MSG(HasComponent<GameObjectComponent>(), "Invalid ID"); return GetComponent<GameObjectComponent>().Layer; }*/
        Entity GetEntity()                  const { return m_entity; }
        Ecs::ECSWorld const * GetWorld()    const { return m_ecsworld; } // ptr to read-only ECS World

        // Setters
        void SetActive(bool active) const;

        // Rule of 5
        GameObject() = default;
        GameObject(GameObject const& copy) = default;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject const&) = default;
        GameObject& operator=(GameObject&&) = default;

        // Explicit Instantiation constructor
        explicit GameObject(Ecs::ECSWorld* ecsWorld);

        // Traditional Construct GameObject Based on UUID
        GameObject(UUID uuid, Ecs::ECSWorld* ecsWorld);

        // Non-Traditional Copy Construct GameObject Based on Entity
        GameObject(Entity entt, Ecs::ECSWorld* ecsWorld);

        /*---------------------------------------------------------------------------------*/
        /* Static Functions                                                                */
        /*---------------------------------------------------------------------------------*/
        static bool IsValid(GameObject go);
        //// Duplicating objects
        //static GameObject Instantiate(GameObject source);
        static void Destroy(GameObject entt);
        //static void Destroy(GameObject entt, std::vector<std::shared_ptr<oo::DeletedGameObject>>& deletedObjs);
        //static GameObject Restore(std::vector<std::shared_ptr<oo::DeletedGameObject>> deletedObjs);

        //static void Swap(GameObject childA, GameObject childB);

        //static void ProcessDeletion();
        //static void ClearLookUp() { s_lookupTable.clear(); }

        // Attempts to search the lookup table with uuid.
        // returns the gameobject if it does
        // else returns max.
        /*static GameObject FindWithInstanceID(UUID uuid);

        static void AddLayer(std::string const& name);
        static void RemoveLayer(std::string const& name);*/


        //implicit cast operator
        operator Entity() const { return m_entity; }

        void Destroy();
        /*void AddChild(GameObject const& gameObj, bool preserveTransforms = false) const;
        void AddChild(std::initializer_list<GameObject> gameObjs, bool preserveTransforms = false) const;
        void SwapChildren(GameObject const& other);
        GameObject GetParent() const;
        std::vector<Entity> GetDirectChilds(bool includeItself = false) const;
        std::vector<Entity> GetChildren(bool includeItself = false) const;*/

        /*---------------------------------------------------------------------------------*/
        /* Queries                                                                         */
        /*---------------------------------------------------------------------------------*/
        template<typename Component>
        Component& GetComponent() const
        {
            ASSERT_MSG(HasComponent<Component>(), "Use TryGet instead if youre Unsure.");
            return m_ecsworld->get_component<Component>(m_entity);
        }

        /*template<typename... Components>
        decltype(auto) GetComponents() const
        {
            ASSERT_MSG(HasComponents<Components>(), "At Least one of the components is missing.");
            return WorldManager::GetActiveWorld().GetComponents<Components>(m_entity);
        }*/

        /*template<typename Component>
        Component* TryGetComponent() const
        {
            return WorldManager::GetActiveWorld().TryGetComponent<Component>(m_entity);
        }*/

        template<typename Component>
        bool HasComponent() const
        {
            ASSERT_MSG(m_ecsworld == nullptr, " ecs world shouldn't be null! Likely created gameobject wrongly");
            return m_ecsworld->has_component<Component>(m_entity);
        }

        /*template<typename Component>
        static ComponentType GetComponentType()
        {
            return WorldManager::GetActiveWorld().GetComponentType<Component>();
        }*/

        /*---------------------------------------------------------------------------------*/
        /* Manipulation                                                                    */
        /*---------------------------------------------------------------------------------*/

        /*template<typename Derived, typename... Args>
        std::enable_if_t<std::is_base_of_v<Component, Derived>, Derived& >
            AddComponent(Args... args) const
        {
            return WorldManager::GetActiveWorld().EmplaceComponent<Derived>(m_entity, true, args...);
        }*/

        /*template<typename NotDerived, typename... Args>
        std::enable_if_t<!std::is_base_of_v<Component, NotDerived>, NotDerived& >
            AddComponent(Args... args) const
        {
            return WorldManager::GetActiveWorld().EmplaceComponent<NotDerived>(m_entity, args...);
        }*/

        template<typename Component>
        Component& AddComponent(Component component) const
        {
            ASSERT_MSG(m_ecsworld == nullptr, " ecs world shouldn't be null! Likely created gameobject wrongly");
            return m_ecsworld->add_component<component>(m_entity, component);
            //return WorldManager::GetActiveWorld().AddComponent<Component>(m_entity, component);
        }

        /*template <typename Component>
        Component* AddComponent(GameObject const& src) const
        {
            CopyComponent<Component>(src);
            return TryGetComponent<Component>(m_entity);
        }*/

        template<typename Component>
        void RemoveComponent() const
        {
            ASSERT_MSG(m_ecsworld == nullptr, " ecs world shouldn't be null! Likely created gameobject wrongly");
            m_ecsworld->remove_component<Component>(m_entity);
            //WorldManager::GetActiveWorld().RemoveComponent<Component>(m_entity);
        }

        /*template<>
        void RemoveComponent<Transform3D>() const { throw "Cannot Remove Transform Component!"; }
        template<>
        void RemoveComponent<Collider2D>() const { throw "Cannot Remove Collider Component, Remove the Circle/Box/Convex Collider Instead!"; }*/

        /*template<typename Component>
        bool CopyComponent(GameObject const& src) const
        {
            return WorldManager::GetActiveWorld().CopyComponent<Component>(src.m_entity, m_entity);
        }

        template<typename Component>
        bool TransferComponent(GameObject const& src) const
        {
            return WorldManager::GetActiveWorld().TransferComponent<Component>(src.m_active, m_entity);
        }*/

        /*template<typename Component>
        bool AttemptRemoveComponent();

        template<typename Component>
        Component& AssertComponent();*/

        template<typename Component, typename...Args>
        Component& EnsureComponent(Args...args) const
        {
            return HasComponent<Component>() ? GetComponent<Component>() : AddComponent<Component>(args...);
        }

        /*GameObject& CopyGameObject(GameObject const& source)
        {
            return *this = WorldManager::GetActiveWorld().DuplicateEntity(source);
        }*/

    private:
        // List of head entities to applied pre-deletion step
        //static std::set<std::pair<Entity, UUID>> s_detachList;
        // set of ids to Remove 
        static std::set<std::pair<Entity, UUID>> s_removeList;
        // one copy of a lookup table for all gameobjects.
        static std::unordered_map<UUID, Entity> s_lookupTable;
        
        //// one map of all the layers information
        //static std::unordered_map<std::string, LayerField> s_layersTable;

        //static void DestroyAndSave(GameObject entt, std::vector<std::shared_ptr<oo::DeletedGameObject>>& deletedObjs);

        //void CalculateHierarchyActive(GameObject parent, bool IsActiveInHierarchy) const;
    };

}
