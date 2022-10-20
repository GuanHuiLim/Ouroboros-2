/************************************************************************************//*!
\file          PhysicsSystem.h
\project       Ouroboros
\author        Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par           email: c.tecklee\@digipen.edu
\date          Sept 02, 2022
\brief         Describes a Physics System that applies dynamics to all physics objects
               performs collision detection between physics objects and
               resolve Physics based Collision resolution.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Archetypes_Ecs/src/A_Ecs.h>
#include "Ouroboros/Physics/PhysicsFwd.h"
#include "Ouroboros/Core/Timer.h"

#include "Ouroboros/EventSystem/Event.h"
#include <Physics/Source/phy.h>

#include <bitset>
#include "Utility/UUID.h"

namespace oo
{
    struct Scene;

    static constexpr std::size_t s_MaxLayerCount = 8;
    using LayerField    = std::bitset<s_MaxLayerCount>;
    using LayerMask     = LayerField;
    using LayerMatrix   = std::unordered_map<LayerField, LayerMask>;

    //physics tick event.
    struct PhysicsTickEvent : public Event
    {
        double deltaTime;
    };

    class PhysicsSystem final : public Ecs::System
    {
    public:
        using Timestep = double;

        PhysicsSystem();
        virtual ~PhysicsSystem() = default;
        virtual void Run(Ecs::ECSWorld*) override {};

        void Init(Scene* m_scene);
        void RuntimeUpdate(Timestep deltaTime);
        void EditorUpdate(Timestep deltaTime);
        void RenderDebugColliders();

        // Global gravity
        vec3 Gravity = { 0, -9.81f, 0 };
        
        // Layering Bitmask Determines collision
        static LayerMatrix PhysicsBitMask;
        // Manupilating Fixed DT
        static void SetFixedDeltaTime(Timestep NewFixedTime);
        static Timestep GetFixedDeltaTime();
    
    private:
        inline static std::uint64_t MaxIterations = 100;
        inline static Timestep FixedDeltaTime = 1.0/MaxIterations;                 // physics updates at 100 fps
        inline static Timestep AccumulatorLimit = FixedDeltaTime * MaxIterations;  // To prevent spiral of death

        void UpdateDynamics(Timestep deltaTime);
        void UpdatePhysicsResolution(Timestep deltaTime);
        
        void UpdateCallbacks();
        void PostUpdate();

        Scene* m_scene;

        // need a way to track physics uuids to gameobject uuids
        std::map<phy_uuid::UUID, UUID> m_physicsToGameObjectLookup;

        //underlying physics world
        myPhysx::PhysxWorld m_physicsWorld;

        //time accumulator
        double m_accumulator;

        void OnRigidbodyAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb);
        void OnRigidbodyRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb);

        void OnBoxColliderAdd(Ecs::ComponentEvent<BoxColliderComponent>* bc);
        void OnBoxColliderRemove(Ecs::ComponentEvent<BoxColliderComponent>* bc);

        void OnCapsuleColliderAdd(Ecs::ComponentEvent<CapsuleColliderComponent>* cc);
        void OnCapsuleColliderRemove(Ecs::ComponentEvent<CapsuleColliderComponent>* cc);

        /*void OnSphereColliderAdd(Ecs::ComponentEvent<SphereColliderComponent>* rb);
        void OnSphereColliderRemove(Ecs::ComponentEvent<SphereColliderComponent>* rb);*/
    
        void InitializeRigidbody(RigidbodyComponent& rb);
        void InitializeBoxCollider(RigidbodyComponent& rb);
        void InitializeCapsuleCollider(RigidbodyComponent& rb);
    };


}
