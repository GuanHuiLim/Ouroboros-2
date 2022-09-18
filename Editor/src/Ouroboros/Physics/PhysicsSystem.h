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
namespace oo
{
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

        void Init();
        void RuntimeUpdate(Timestep deltaTime);
        void EditorUpdate(Timestep deltaTime);

        // Global gravity
        vec3 Gravity = { 0, -9.81f, 0 };

        static constexpr std::uint64_t MaxIterations = 50;
        static constexpr Timestep FixedDeltaTime = 1.0/100.0;                         // physics updates at 100 fps
        static constexpr Timestep AccumulatorLimit = FixedDeltaTime * MaxIterations;  // To prevent spiral of death

        // Layering Bitmask Determines collision
        static LayerMatrix PhysicsBitMask;

    private:
        void UpdateDynamics(Timestep deltaTime);
        void UpdatePhysicsCollision();
        void UpdatePhysicsResolution(Timestep deltaTime);

        void IntegrateForces(Timestep deltaTime);
        void IntegratePositions(Timestep deltaTime);
        void ResetForces();

        void UpdateGlobalBounds();
        void UpdateDynamicGlobalBounds();

        void BroadPhase();
        void NarrowPhase();
        void UpdateCallbacks();
        void ResolvePhysicsResolution();
        void PostUpdate();

#if PHYSICS_DEBUG_MSG && OO_DEBUG || PHYSICS_DEBUG_MSG && OO_RELEASE
        std::uint64_t m_collisionChecks = 0, m_actualCollisions = 0;
#endif  

        PhysxWorld m_physicsWorld;

        //time accumulator
        double m_accumulator;

        void OnRigidbodyAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb);
        void OnRigidbodyRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb);

        /*void OnColliderAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb);
        void OnColliderRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb);

        void OnColliderAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb);
        void OnColliderRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb);*/
    };


}
