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

#include "Ouroboros/Physics/RigidbodyComponent.h"
#include "Ouroboros/Physics/ColliderComponents.h"

#include "Ouroboros/ECS/DeferredComponent.h"

#include "Ouroboros/Transform/TransformComponent.h"

#include "OO_Vulkan/src/DebugDraw.h"
#include "Ouroboros/ECS/ECS.h"

// test
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"
namespace oo
{
    PhysicsSystem::PhysicsSystem()
        : m_accumulator{0}
        , Gravity { 0, -0.981f, 0 }
        , m_physicsWorld{ PxVec3{Gravity.x, Gravity.y, Gravity.z} }
    {
        
    }

    void PhysicsSystem::Init(Scene* scene)
    {
        m_scene = scene;

        m_world->SubscribeOnAddComponent<PhysicsSystem, RigidbodyComponent>(
            this, &PhysicsSystem::OnRigidbodyAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, RigidbodyComponent>(
            this, &PhysicsSystem::OnRigidbodyRemove);
        
        m_world->SubscribeOnAddComponent<PhysicsSystem, BoxColliderComponent>(
            this, &PhysicsSystem::OnBoxColliderAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, BoxColliderComponent>(
            this, &PhysicsSystem::OnBoxColliderRemove);

        m_world->SubscribeOnAddComponent<PhysicsSystem, CapsuleColliderComponent>(
            this, &PhysicsSystem::OnCapsuleColliderAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, CapsuleColliderComponent>(
            this, &PhysicsSystem::OnCapsuleColliderRemove);
    }
    
    void PhysicsSystem::RuntimeUpdate(Timestep deltaTime)
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

            m_accumulator -= FixedDeltaTime;
            TRACY_PROFILE_SCOPE_END();
        }

        {
            TRACY_PROFILE_SCOPE_NC(physics_post_update, tracy::Color::PeachPuff);
            PostUpdate();
            TRACY_PROFILE_SCOPE_END();
        }

        // End of frame debug draw. Once per frame
        {
            TRACY_PROFILE_SCOPE_NC(physics_debug_draw, tracy::Color::PeachPuff);
            DrawDebugColliders();
            TRACY_PROFILE_SCOPE_END();
        }
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::EditorUpdate(Timestep deltaTime)
    {
        TRACY_PROFILE_SCOPE_NC(physics_update_editor, tracy::Color::PeachPuff);

        EditorCoreUpdate();

        // Update global bounds of all objects
        //UpdateGlobalBounds();
        
        // End of frame debug draw. Once per frame
        {
            TRACY_PROFILE_SCOPE_NC(physics_debug_draw, tracy::Color::PeachPuff);
            DrawDebugColliders();
            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateDynamics(Timestep deltaTime)
    {
        //TODO: Should remove eventually (perhaps?)
        EditorCoreUpdate();
        
        // update the physics world using fixed dt.
        m_physicsWorld.updateScene(static_cast<float>(FixedDeltaTime));

        static Ecs::Query rb_query = Ecs::make_query< TransformComponent, RigidbodyComponent>();
        
        // set position and orientation
        m_world->for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
        {
            auto pos = rb.GetPositionInPhysicsWorld();
            tf.SetGlobalPosition(pos);

            auto orientation = rb.GetOrientationInPhysicsWorld();
            tf.SetGlobalOrientation(orientation);
        });

        static Ecs::Query dynamicBoxColliderQuery = Ecs::make_query<TransformComponent, BoxColliderComponent, RigidbodyComponent>();

        //Updating Dynamic Box Collider Bounds
        m_world->for_each(dynamicBoxColliderQuery, [&](TransformComponent& tf, BoxColliderComponent& bc, RigidbodyComponent& rb)
        {
            auto pos = tf.GetGlobalPosition();
            auto scale = tf.GetGlobalScale();
            auto quat = tf.GetGlobalRotationQuat();

            // calculate local scale
            //bc.Bounds.min = (bc.Size * -0.5f) * scale;
            //bc.Bounds.max = (bc.Size *  0.5f) * scale;

            // calculate global bounds and half extents
            bc.GlobalHalfExtents = { bc.HalfExtents * bc.Size * scale };
            //bc.GlobalBounds = { bc.Bounds.min , bc.Bounds.max };
            //auto halfExtents = (bc.GlobalBounds.max - bc.GlobalBounds.min) * 0.5f;
            auto globalPos = pos + bc.Offset;

            // set box size
            rb.object.setBoxProperty(bc.GlobalHalfExtents.x, bc.GlobalHalfExtents.y, bc.GlobalHalfExtents.z);

            //phy.object.setposition({ globalPos.x, globalPos.y, globalPos.z });
            rb.object.setPosOrientation({ globalPos.x, globalPos.y, globalPos.z }, { quat.value.w, quat.value.x, quat.value.y, quat.value.z });
        });


        // Update dynamics
        IntegrateForces(deltaTime);
        IntegratePositions(deltaTime);
        ResetForces();

        // Update global bounds of all DYNAMIC objects
        UpdateDynamicGlobalBounds();
    }

    void PhysicsSystem::UpdatePhysicsCollision()
    {
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

    void PhysicsSystem::DrawDebugColliders()
    {
        //TODO : Toggle to enable/disable debug drawing of bounds.
       
        //Updating box collider's bounds and debug drawing
        static Ecs::Query boxColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, BoxColliderComponent>();
        m_world->for_each(boxColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, BoxColliderComponent& bc)
        {
            auto pos = tf.GetGlobalPosition();
            auto scale = tf.GetGlobalScale();
            auto quat = tf.GetGlobalRotationQuat();

            // calculate global bounds and half extents
            bc.GlobalHalfExtents = { bc.HalfExtents * bc.Size * scale };
            auto globalPos = pos + bc.Offset;

            //Debug draw the bounds
            DebugDraw::AddAABB({ globalPos + bc.GlobalHalfExtents  , globalPos - bc.GlobalHalfExtents }, oGFX::Colors::GREEN);
        });

        //Updating capsule collider's bounds and debug drawing
        static Ecs::Query capsuleColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, CapsuleColliderComponent>();
        m_world->for_each(capsuleColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, CapsuleColliderComponent& cc)
        {
            auto pos = tf.GetGlobalPosition();
            auto scale = tf.GetGlobalScale();
            auto quat = tf.GetGlobalRotationQuat();

            // calculate global bounds of capsule
            glm::vec3 GlobalHalfExtents = { cc.Radius * scale.x, cc.HalfHeight * scale.y, cc.Radius * scale.z };
            //cc.HalfHeight + cc.Radius;
            glm::vec3 globalPos = pos + cc.Offset;

            //Debug draw the bounds
            DebugDraw::AddAABB({ globalPos + GlobalHalfExtents , globalPos - GlobalHalfExtents }, oGFX::Colors::GREEN);
            // draw top sphere
            DebugDraw::AddSphere({ globalPos + vec3{ 0, GlobalHalfExtents.y, 0}, GlobalHalfExtents.x }, oGFX::Colors::GREEN);
            // draw bottom sphere
            DebugDraw::AddSphere({ globalPos - vec3{ 0, GlobalHalfExtents.y, 0}, GlobalHalfExtents.x }, oGFX::Colors::GREEN);
        });
    }

    void PhysicsSystem::EditorCoreUpdate()
    {
        // Update physics World's objects position and Orientation
        static Ecs::Query rb_query = Ecs::make_query<GameObjectComponent, TransformComponent, RigidbodyComponent>();
        m_world->for_each(rb_query, [&](GameObjectComponent& goc, TransformComponent& tf, RigidbodyComponent& rb)
            {
                auto pos = tf.GetGlobalPosition();
                auto quat = tf.GetGlobalRotationQuat();
                rb.SetPosOrientation(pos, quat);
            });

        //Updating box collider's bounds and debug drawing
        static Ecs::Query boxColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, BoxColliderComponent>();
        m_world->for_each(boxColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, BoxColliderComponent& bc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate global bounds and half extents
                bc.GlobalHalfExtents = { bc.HalfExtents * bc.Size * scale };
                auto globalPos = pos + bc.Offset;

                // set box size
                rb.object.setBoxProperty(bc.GlobalHalfExtents.x, bc.GlobalHalfExtents.y, bc.GlobalHalfExtents.z);
            });

        //Updating capsule collider's bounds and debug drawing
        static Ecs::Query capsuleColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, CapsuleColliderComponent>();
        m_world->for_each(capsuleColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, CapsuleColliderComponent& cc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate global bounds of capsule
                glm::vec3 GlobalHalfExtents = { cc.Radius * scale.x, cc.HalfHeight * scale.y, cc.Radius * scale.z };
                //cc.HalfHeight + cc.Radius;
                glm::vec3 globalPos = pos + cc.Offset;

                // set box size
                rb.object.setCapsuleProperty(GlobalHalfExtents.x * 2, GlobalHalfExtents.y * 2);
            });
    }



    void PhysicsSystem::OnRigidbodyAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        auto& rb_comp = m_world->get_component<RigidbodyComponent>(rb->entityID);
        rb_comp.object = m_physicsWorld.createInstance();
        rb_comp.SetStatic(false);   // default to dynamic object.
        //default initialize material
        rb_comp.object.setMaterial(PhysicsMaterial{});
    }

    void PhysicsSystem::OnRigidbodyRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        //Remove all other colliders as well

        if (m_world->has_component<BoxColliderComponent>(rb->entityID))
            m_world->remove_component<BoxColliderComponent>(rb->entityID);
        /*if(m_world->has_component<SphereColliderComponent>(rb->entityID))
            m_world->remove_component<BoxColliderComponent>(rb->entityID);*/
        if(m_world->has_component<CapsuleColliderComponent>(rb->entityID))
            m_world->remove_component<CapsuleColliderComponent>(rb->entityID);
    }

    void PhysicsSystem::OnBoxColliderAdd(Ecs::ComponentEvent<BoxColliderComponent>* bc)
    {
        // if box collider is directly added, ensure we add rigidbody too.
        if (m_world->has_component<RigidbodyComponent>(bc->entityID) == false)
        {
            m_world->add_component<RigidbodyComponent>(bc->entityID);
        }
        
        auto& rb_comp = m_world->get_component<RigidbodyComponent>(bc->entityID);

        // create box
        rb_comp.object.setShape(myPhysx::shape::box);
    }

    void PhysicsSystem::OnBoxColliderRemove(Ecs::ComponentEvent<BoxColliderComponent>* bc)
    {
        auto& rb_comp = m_world->get_component<RigidbodyComponent>(bc->entityID);
        rb_comp.object.setShape(myPhysx::shape::none);
    }

    void PhysicsSystem::OnCapsuleColliderAdd(Ecs::ComponentEvent<CapsuleColliderComponent>* cc)
    {
        // if box collider is directly added, ensure we add rigidbody too.
        if (m_world->has_component<RigidbodyComponent>(cc->entityID) == false)
        {
            m_world->add_component<RigidbodyComponent>(cc->entityID);
        }

        auto& rb_comp = m_world->get_component<RigidbodyComponent>(cc->entityID);

        // create box
        rb_comp.object.setShape(myPhysx::shape::capsule);
    }

    void PhysicsSystem::OnCapsuleColliderRemove(Ecs::ComponentEvent<CapsuleColliderComponent>* cc)
    {
        auto& rb_comp = m_world->get_component<RigidbodyComponent>(cc->entityID);
        rb_comp.object.setShape(myPhysx::shape::none);
    }

   /* void PhysicsSystem::OnSphereColliderAdd(Ecs::ComponentEvent<SphereColliderComponent>* rb)
    {
    }

    void PhysicsSystem::OnSphereColliderRemove(Ecs::ComponentEvent<SphereColliderComponent>* rb)
    {

    }*/

}
