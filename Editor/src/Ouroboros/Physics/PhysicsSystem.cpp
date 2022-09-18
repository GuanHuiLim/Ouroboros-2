/************************************************************************************//*!
\file          PhysicsSystem.cpp
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
#include "pch.h"
#include "PhysicsSystem.h"

#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Ouroboros/EventSystem/EventManager.h"

namespace oo
{
    void PhysicsSystem::Init()
    {
    }
    
    void PhysicsSystem::RuntimeUpdate(Timestep deltaTime)
    {
        constexpr const char* const physics_update = "physics_update";
        constexpr const char* const physics_fixed_update = "physics_fixed_update";
        constexpr const char* const physics_collision = "physics_collision";
        constexpr const char* const physics_resolution = "physics_resolution";
        constexpr const char* const physics_dynamics = "physics_dynamics";
        constexpr const char* const physics_post_update = "physics_post_update";
        {
            TRACY_PROFILE_SCOPE_NC(physics_update, tracy::Color::PeachPuff);

            m_accumulator += deltaTime;

            //avoids spiral of death.
            if (m_accumulator > AccumulatorLimit)
                m_accumulator = AccumulatorLimit;

            // Update global bounds of all DYNAMIC objects
            if (m_accumulator > FixedDeltaTime)
                UpdateDynamicGlobalBounds();

            while (m_accumulator > FixedDeltaTime)
            {
                TRACY_PROFILE_SCOPE_NC(physics_fixed_update, tracy::Color::PeachPuff1);
                //LOG_ENGINE_CRITICAL("Physics On Physics Tick Begin");
                PhysicsTickEvent e;
                e.deltaTime = FixedDeltaTime;
                EventManager::Broadcast(&e);
                //LOG_ENGINE_CRITICAL("Physics On Physics Tick End");

                {
                    TRACY_PROFILE_SCOPE_NC(physics_collision, tracy::Color::PeachPuff2);
                    UpdatePhysicsCollision();
                    TRACY_PROFILE_SCOPE_END();
                }

                {
                    TRACY_PROFILE_SCOPE_NC(physics_resolution, tracy::Color::PeachPuff3);
                    UpdatePhysicsResolution(FixedDeltaTime);
                    TRACY_PROFILE_SCOPE_END();
                }

                {
                    TRACY_PROFILE_SCOPE_NC(physics_dynamics, tracy::Color::PeachPuff4);
                    UpdateDynamics(FixedDeltaTime);
                    TRACY_PROFILE_SCOPE_END();
                }
                //Warm starting
                //ResolvePhysicsResolution();

                m_accumulator -= FixedDeltaTime;
                TRACY_PROFILE_SCOPE_END();
            }

            {
                TRACY_PROFILE_SCOPE_NC(physics_post_update, tracy::Color::PeachPuff);
                PostUpdate();
                TRACY_PROFILE_SCOPE_END();
            }

            TRACY_PROFILE_SCOPE_END();
        }
    }

    void PhysicsSystem::EditorUpdate(Timestep deltaTime)
    {
        constexpr const char* const physics_update = "physics_update";
        TRACY_PROFILE_SCOPE_NC(physics_update, tracy::Color::PeachPuff);
        // Update global bounds of all objects
        UpdateGlobalBounds();
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateDynamics(Timestep deltaTime)
    {
        // Update dynamics
        IntegrateForces(deltaTime);
        IntegratePositions(deltaTime);
        ResetForces();

        //TODO : If required launch an event instead
        // Recognise all transforms has been updated. [this does make physics system
        // reliant on transform system existing!]
        //m_ECS_Manager.GetSystem<oo::TransformSystem>()->UpdateTransform();

        // Update global bounds of all DYNAMIC objects
        UpdateDynamicGlobalBounds();
    }

    void PhysicsSystem::UpdatePhysicsCollision()
    {
        constexpr const char* const physics_broadphase = "physics_broadphase";
        constexpr const char* const physics_narrowphase = "physics_narrowphase";
        constexpr const char* const physics_callbacks = "physics_callbacks";
        {
            TRACY_PROFILE_SCOPE_NC(physics_broadphase, tracy::Color::PeachPuff);

            //Broadphase Collection & Culling
            BroadPhase();

            TRACY_PROFILE_SCOPE_END();
        }

        {
            TRACY_PROFILE_SCOPE_NC(physics_narrowphase, tracy::Color::PeachPuff1);

            //NarrowPhase Stringest Test
            NarrowPhase();

            TRACY_PROFILE_SCOPE_END();
        }

        {
            TRACY_PROFILE_SCOPE_NC(physics_callbacks, tracy::Color::PeachPuff2);

            //Update Callbacks
            UpdateCallbacks();

            TRACY_PROFILE_SCOPE_END();
            //Generate Manifold : Rigidbody only. If trigger : 2 flags and set them
            //_mm_rsqrt_ss
        }
    }

    void PhysicsSystem::UpdatePhysicsResolution(Timestep deltaTime)
    {
    }

    void PhysicsSystem::IntegrateForces(Timestep deltaTime)
    {
    }

    void PhysicsSystem::IntegratePositions(Timestep deltaTime)
    {
    }

    void PhysicsSystem::ResetForces()
    {
    }

    void PhysicsSystem::UpdateGlobalBounds()
    {
    }

    void PhysicsSystem::UpdateDynamicGlobalBounds()
    {
    }

    void PhysicsSystem::BroadPhase()
    {
    }

    void PhysicsSystem::NarrowPhase()
    {
    }

    void PhysicsSystem::UpdateCallbacks()
    {
    }

    void PhysicsSystem::ResolvePhysicsResolution()
    {
    }

    void PhysicsSystem::PostUpdate()
    {
    }
}