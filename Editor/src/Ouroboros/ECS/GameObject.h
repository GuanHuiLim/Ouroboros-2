/************************************************************************************//*!
\file           GameObject.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes a gameobject which is the basic unit and building block for
                every scene. This object provides the ability to tap on existing ECS
                systems and the ability to have scenegraph.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Archetypes_ECS/src/A_Ecs.h>
#include <Archetypes_ECS/src/EcsUtils.h>
#include <set>

#include "Ouroboros/Core/Base.h"
#include "Utility/UUID.h"
#include "GameObjectComponent.h"
#include "Ouroboros/Transform/TransformComponent.h"

#include "Ouroboros/Scene/Scene.h"

namespace oo
{
    // WARNING: Entity is no longer directly convertible to Gameobject.
    class GameObject final
    {
    public:
        using Entity = Ecs::EntityID;

        static constexpr uint64_t   NOPARENT = std::numeric_limits<uint64_t>::max();
        static constexpr Entity     NOTFOUND = std::numeric_limits<Entity>::max();

        static constexpr uint64_t   ROOTID = std::numeric_limits<uint64_t>::min();
        static constexpr Entity     ROOT = std::numeric_limits<Entity>::min();

        //Events

        // will be broadcasted right before the removal.
        // potentially dangerous, make sure you know what you're doing.
        struct OnDestroy : public Event
        {
            GameObject* go;
        };

    private:
        Scene* m_scene = nullptr;
        Entity m_entity = NOTFOUND; //default to not found

    /*---------------------------------------------------------------------------------*/
    /* Public Functions                                                                */
    /*---------------------------------------------------------------------------------*/
    public:
        // Helper Getters
        TransformComponent& Transform()             const;
        bool IsActive()                             const;
        bool ActiveInHierarchy()                    const;

        // only done after ecs able to return dynamic objects
        std::string& Name()                         const;
        oo::UUID GetInstanceID()                        const;
        scenenode::weak_pointer GetSceneNode()      const;
        bool GetIsPrefab()                          const;
        Entity GetEntity()                          const;
        Scene const* GetScene()                     const;

        // Setters
        void SetActive(bool active) const;
        void SetName(std::string_view name) const;

        void SetIsPrefab(bool isprefab) const;

        /*---------------------------------------------------------------------------------*/
        /* Constructors                                                                    */
        /*---------------------------------------------------------------------------------*/

        // Rule of 5
        GameObject() = default;
        GameObject(GameObject const& copy) = default;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject const&) = default;
        GameObject& operator=(GameObject&&) = default;
        ~GameObject() = default;

        // Explicit Instantiation constructor
        explicit GameObject(Scene& scene);

        // Explicit Instantiation From Another Existing Gameobject constructor
        explicit GameObject(Scene& scene, GameObject& target);

        // Traditional Construct GameObject Based on UUID
        GameObject(UUID uuid, Scene& scene);

        // Non-Traditional Copy Construct GameObject Based on Entity
        GameObject(Entity entt, Scene& scene);

        // equality comparison
        bool operator==(GameObject rhs);

        // equality comparison
        bool operator<(GameObject rhs);

        // equality comparison
        bool operator>(GameObject rhs);

        // Destroy
        void Destroy();

        /*---------------------------------------------------------------------------------*/
        /* Scene Graph Functions                                                           */
        /*---------------------------------------------------------------------------------*/
        void AddChild(GameObject const& child, bool preserveTransforms = false) const;
        void AddChild(std::initializer_list<GameObject> gameObjs, bool preserveTransforms = false) const;
        /*void SwapChildren(GameObject const& other);*/
        
        Scene::go_ptr TryGetParent() const;
        GameObject GetParent() const;
        std::vector<GameObject> GetDirectChilds(bool includeItself = false) const;
        std::vector<GameObject> GetChildren(bool includeItself = false) const;
        
        oo::UUID GetParentUUID() const;
        std::vector<oo::UUID> GetDirectChildsUUID(bool includeItself = false) const;
        std::vector<oo::UUID> GetChildrenUUID(bool includeItself = false) const;
        
        bool HasChild() const;
        bool HasValidParent() const;
        std::size_t GetChildCount() const;
        std::size_t GetDirectChildCount() const;

        /*---------------------------------------------------------------------------------*/
        /* Static Functions                                                                */
        /*---------------------------------------------------------------------------------*/
        GameObject Duplicate();

        /*---------------------------------------------------------------------------------*/
        /* Queries                                                                         */
        /*---------------------------------------------------------------------------------*/
        
        std::size_t GetComponentCount() const;

        template<typename Component>
        Component& GetComponent() const;

        template<typename Component>
        Component* TryGetComponent() const;

        template<typename Component>
        bool HasComponent() const;

        /*---------------------------------------------------------------------------------*/
        /* Manipulation                                                                    */
        /*---------------------------------------------------------------------------------*/

        template<typename Component, typename... Args>
        Component& AddComponent(Args... args) const;

        template<typename Component>
        void RemoveComponent() const;
        
        template<typename Component>
        void TryRemoveComponent() const;

        template<>
        void RemoveComponent<TransformComponent>() const;

        template<typename Component, typename...Args>
        Component& EnsureComponent(Args...args) const;

    private:
        void SetupGo(UUID uuid, Ecs::EntityID entt);

        void SetHierarchyActive(GameObjectComponent& comp, bool active) const;
        void CalculateHierarchyActive(GameObject parent, bool IsActiveInHierarchy) const;
    };


    template<typename Component>
    inline Component& GameObject::GetComponent() const
    {
        ASSERT_MSG(HasComponent<Component>() == false, "Use TryGet instead if youre Unsure.");
        return m_scene->GetWorld().get_component<Component>(m_entity);
    }

    template<typename Component>
    inline Component* GameObject::TryGetComponent() const
    {
        return HasComponent<Component>() ? &GetComponent<Component>() : nullptr;
    }

    template<typename Component>
    inline bool GameObject::HasComponent() const
    {
        ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
        ASSERT_MSG(m_scene->IsValid(*this) == false, " gameobject does not belong to this scene, how did you create this gameobject??");
        return m_scene->GetWorld().has_component<Component>(m_entity);
    }

    template<typename Component, typename ...Args>
    inline Component& GameObject::AddComponent(Args ...args) const
    {
        ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
        ASSERT_MSG(m_scene->IsValid(*this) == false, " gameobject does not belong to this scene, how did you create this gameobject??");
        m_scene->GetWorld().add_component<Component>(m_entity, args...);
        return GetComponent<Component>();
    }

    template<typename Component>
    inline void GameObject::TryRemoveComponent() const
    {
        if(HasComponent<Component>())
            RemoveComponent<Component>();
    }

    template<typename Component>
    inline void GameObject::RemoveComponent() const
    {
        ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
        ASSERT_MSG(m_scene->IsValid(*this) == false, " gameobject does not belong to this scene, how did you create this gameobject??");
        m_scene->GetWorld().remove_component<Component>(m_entity);
    }

    template<>
    inline void GameObject::RemoveComponent<TransformComponent>() const
    {
        throw "Cannot remove the transform component!";
    }

    template<typename Component, typename ...Args>
    inline Component& GameObject::EnsureComponent(Args ...args) const
    {
        return HasComponent<Component>() ? GetComponent<Component>() : AddComponent<Component>(args...);
    }
}