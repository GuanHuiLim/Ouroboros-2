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

//#include <A_Ecs.h>
//#include <EcsUtils.h>
#include <set>
#include "Ouroboros/Scene/Scene.h"

#include "Ouroboros/Core/Base.h"
#include "Utility/UUID.h"
#include "GameObjectComponent.h"
#include "Ouroboros/Transform/TransformComponent.h"

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
        Scene* m_scene      = nullptr;
        //Ecs::ECSWorld* m_ecsworld   = nullptr;
        Entity m_entity     = NOTFOUND; //default to not found

    /*---------------------------------------------------------------------------------*/
    /* Public Functions                                                                */
    /*---------------------------------------------------------------------------------*/
    public:
        // Helper Getters
        Transform3D& Transform()                    const { ASSERT_MSG(!HasComponent<Transform3D>(), "Invalid ID");          return GetComponent<Transform3D>(); };
        bool IsActive()                             const { ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  return GetComponent<GameObjectComponent>().Active; }
        bool ActiveInHierarchy()                    const { ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  return GetComponent<GameObjectComponent>().ActiveInHierarchy; }
        std::string& Name()                         const { ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  return GetComponent<GameObjectComponent>().Name; }
        UUID GetInstanceID()                        const { ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  return GetComponent<GameObjectComponent>().Id; }
        scenenode::shared_pointer GetSceneNode()    const { ASSERT_MSG(!HasComponent<GameObjectComponent>(), "Invalid ID");  return GetComponent<GameObjectComponent>().Node.lock(); }
        Entity GetEntity()                          const { return m_entity; }
        //Ecs::ECSWorld const GetWorld()              const { ASSERT_MSG(m_scene == nullptr, "GameObject has invalid scene"); return m_scene->GetWorld(); } // ptr to read-only ECS World
        Scene const* GetScene()                     const { return m_scene; } 

        // Setters
        void SetActive(bool active) const;
        void SetName(std::string_view name) const;

        // Rule of 5
        GameObject() = default;
        GameObject(GameObject const& copy) = default;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject const&) = default;
        GameObject& operator=(GameObject&&) = default;

        // Explicit Instantiation constructor
        explicit GameObject(Scene* scene);

        // Traditional Construct GameObject Based on UUID
        GameObject(UUID uuid, Scene* scene);

        // Non-Traditional Copy Construct GameObject Based on Entity
        GameObject(Entity entt, Scene* scene);

        // Scene-graph Related Functions
        void AddChild(GameObject const& child, bool preserveTransforms = false) const;
        /*void AddChild(std::initializer_list<GameObject> gameObjs, bool preserveTransforms = false) const;
        void SwapChildren(GameObject const& other);*/
        UUID GetParentUUID() const;
        std::vector<UUID> GetDirectChildsUUID(bool includeItself = false) const;
        std::vector<UUID> GetChildrenUUID(bool includeItself = false) const;

        /*---------------------------------------------------------------------------------*/
        /* Static Functions                                                                */
        /*---------------------------------------------------------------------------------*/
        //GameObject GameObject::Instantiate(GameObject source);

        /*---------------------------------------------------------------------------------*/
        /* Queries                                                                         */
        /*---------------------------------------------------------------------------------*/
        template<typename Component>
        Component& GetComponent() const
        {
            ASSERT_MSG(HasComponent<Component>() == false, "Use TryGet instead if youre Unsure.");
            return m_scene->GetWorld().get_component<Component>(m_entity);
        }

        template<typename Component>
        Component* TryGetComponent() const
        {
            return HasComponent<Component>() ? GetComponent<Component>(): nullptr;
        }

        template<typename Component>
        bool HasComponent() const
        {
            ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
            return m_scene->GetWorld().has_component<Component>(m_entity);
        }

        /*---------------------------------------------------------------------------------*/
        /* Manipulation                                                                    */
        /*---------------------------------------------------------------------------------*/

        template<typename Component, typename... Args>
        Component& AddComponent(Args... args) const
        {
            ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
            m_scene->GetWorld().add_component<Component>(m_entity, args...);
            return GetComponent<Component>();
        }

        template<typename Component>
        void RemoveComponent() const
        {
            ASSERT_MSG(m_scene == nullptr, " scene shouldn't be null! Likely created gameobject wrongly");
            m_scene->GetWorld().remove_component<Component>(m_entity);
        }

        template<typename Component, typename...Args>
        Component& EnsureComponent(Args...args) const
        {
            return HasComponent<Component>() ? GetComponent<Component>() : AddComponent<Component>(args...);
        }
    };

}
