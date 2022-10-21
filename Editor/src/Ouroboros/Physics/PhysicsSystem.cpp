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

        myPhysx::physx_system::provideCurrentWorld(&m_physicsWorld);
    }
    
    void PhysicsSystem::RuntimeUpdate(Timestep deltaTime)
    {
        TRACY_PROFILE_SCOPE_NC(physics_update, tracy::Color::PeachPuff);

        m_accumulator += deltaTime;
        
        //avoids spiral of death.
        if (m_accumulator > AccumulatorLimit)
            m_accumulator = AccumulatorLimit;

        while (m_accumulator > FixedDeltaTime)
        {
            TRACY_PROFILE_SCOPE_NC(physics_fixed_update, tracy::Color::PeachPuff1);
            PhysicsTickEvent e;
            e.DeltaTime = FixedDeltaTime;
            EventManager::Broadcast(&e);

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

        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::EditorUpdate(Timestep deltaTime)
    {
        TRACY_PROFILE_SCOPE_NC(physics_update_editor, tracy::Color::PeachPuff);

        // Update Duplicated Objects
        {
            static Ecs::Query duplicated_rb_query = Ecs::make_raw_query<RigidbodyComponent, TransformComponent, DuplicatedComponent>();
            m_world->for_each(duplicated_rb_query, [&](RigidbodyComponent& rbComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeRigidbody(rbComp);
            });

            static Ecs::Query duplicated_rb_with_box_query = Ecs::make_raw_query<RigidbodyComponent, BoxColliderComponent, TransformComponent, DuplicatedComponent>();
            m_world->for_each(duplicated_rb_with_box_query, [&](RigidbodyComponent& rbComp, BoxColliderComponent& bcComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeBoxCollider(rbComp);
            });

            static Ecs::Query duplicated_rb_with_capsule_query = Ecs::make_raw_query<RigidbodyComponent, CapsuleColliderComponent, TransformComponent, DuplicatedComponent>();
            m_world->for_each(duplicated_rb_with_capsule_query, [&](RigidbodyComponent& rbComp, CapsuleColliderComponent& ccComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeCapsuleCollider(rbComp);
            });
        }


        // Update physics World's objects position and Orientation
        static Ecs::Query rb_query = Ecs::make_query<GameObjectComponent, TransformComponent, RigidbodyComponent>();
        m_world->for_each(rb_query, [&](GameObjectComponent& goc, TransformComponent& tf, RigidbodyComponent& rb)
            {
                auto pos = tf.GetGlobalPosition();
                auto quat = tf.GetGlobalRotationQuat();
                rb.SetPosOrientation(pos + rb.Offset, quat);
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

                // set box size
                rb.object.setBoxProperty(bc.GlobalHalfExtents.x, bc.GlobalHalfExtents.y, bc.GlobalHalfExtents.z);

                // test and set trigger boolean
                if (rb.object.getTrigger() != bc.IsTrigger)
                    rb.object.setTriggerShape(bc.IsTrigger);
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

                // set box size
                rb.object.setCapsuleProperty(GlobalHalfExtents.x * 2, GlobalHalfExtents.y * 2);
            });

        // Update global bounds of all objects
        //UpdateGlobalBounds();
        
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateDynamics(Timestep deltaTime)
    {
        //TODO: Should remove eventually (perhaps?)
        EditorUpdate(deltaTime);
        
        // update the physics world using fixed dt.
        m_physicsWorld.updateScene(static_cast<float>(FixedDeltaTime));

        static Ecs::Query rb_query = Ecs::make_query< TransformComponent, RigidbodyComponent>();
        
        // set position and orientation
        m_world->for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
        {
            auto pos = rb.GetPositionInPhysicsWorld();
            tf.SetGlobalPosition(pos - rb.Offset);

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

            // calculate global bounds and half extents
            bc.GlobalHalfExtents = { bc.HalfExtents * bc.Size * scale };
            auto physicsPos = pos + rb.Offset;

            // set box position and orientation
            rb.object.setPosOrientation({ physicsPos.x, physicsPos.y, physicsPos.z }, { quat.value.w, quat.value.x, quat.value.y, quat.value.z });
            // set box size
            rb.object.setBoxProperty(bc.GlobalHalfExtents.x, bc.GlobalHalfExtents.y, bc.GlobalHalfExtents.z);
        });

    }

    void PhysicsSystem::UpdatePhysicsResolution(Timestep deltaTime)
    {
        UpdateCallbacks();
    }

    void PhysicsSystem::UpdateCallbacks()
    {
        auto trigger_queue = m_physicsWorld.getTriggerData();
        while (!trigger_queue->empty())
        {
            myPhysx::TriggerManifold trigger_manifold = trigger_queue->front();
            
            ASSERT_MSG(m_physicsToGameObjectLookup.contains(trigger_manifold.triggerID) == false, "This should never happen");
            ASSERT_MSG(m_physicsToGameObjectLookup.contains(trigger_manifold.otherID) == false, "This should never happen");

            // retrieve their gameobject id counterpart.
            UUID trigger_go_id = m_physicsToGameObjectLookup.at(trigger_manifold.triggerID);
            UUID other_go_id = m_physicsToGameObjectLookup.at(trigger_manifold.otherID);

            //broadcast trigger event.
            PhysicsTriggerEvent pte;
            pte.TriggerID = trigger_go_id;
            pte.OtherID = other_go_id;
            switch (trigger_manifold.status)
            {
            case myPhysx::trigger::none:
                pte.State = TriggerState::NONE;
                break;
            case myPhysx::trigger::onTriggerEnter:
                pte.State = TriggerState::ENTER;
                break;
            case myPhysx::trigger::onTriggerStay:
                pte.State = TriggerState::STAY;
                break;
            case myPhysx::trigger::onTriggerExit:
                pte.State = TriggerState::EXIT;
                break;
            }
            EventManager::Broadcast(&pte);

            trigger_queue->pop();
        }
        m_physicsWorld.clearTriggerData();
    }

    void PhysicsSystem::PostUpdate()
    {
    }


    void PhysicsSystem::RenderDebugColliders()
    {
        TRACY_PROFILE_SCOPE_NC(physics_debug_draw, tracy::Color::PeachPuff);
        //TODO : Toggle to enable/disable debug drawing of bounds.
       
        //Updating box collider's bounds and debug drawing
        // Assumes every data is updated and no need for any calculations.
        static Ecs::Query boxColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, BoxColliderComponent>();
        m_world->for_each(boxColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, BoxColliderComponent& bc)
        {
            auto pos = rb.GetPositionInPhysicsWorld();
            auto quat = rb.GetOrientationInPhysicsWorld();

            //Debug draw the bounds
            DebugDraw::AddAABB({ pos + bc.GlobalHalfExtents  , pos - bc.GlobalHalfExtents }, oGFX::Colors::GREEN);
        });

        //Updating capsule collider's bounds and debug drawing
        static Ecs::Query capsuleColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, CapsuleColliderComponent>();
        m_world->for_each(capsuleColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, CapsuleColliderComponent& cc)
        {
            auto pos = rb.GetPositionInPhysicsWorld();
            auto scale = tf.GetGlobalScale();
            auto quat = rb.GetOrientationInPhysicsWorld();

            // calculate global bounds of capsule
            glm::vec3 GlobalHalfExtents = { cc.Radius * scale.x, cc.HalfHeight * scale.y, cc.Radius * scale.z };

            //Debug draw the bounds
            DebugDraw::AddAABB({ pos + GlobalHalfExtents , pos - GlobalHalfExtents }, oGFX::Colors::GREEN);
            // draw top sphere
            DebugDraw::AddSphere({ pos + vec3{ 0, GlobalHalfExtents.y, 0}, GlobalHalfExtents.x }, oGFX::Colors::GREEN);
            // draw bottom sphere
            DebugDraw::AddSphere({ pos - vec3{ 0, GlobalHalfExtents.y, 0}, GlobalHalfExtents.x }, oGFX::Colors::GREEN);
        });

        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::SetFixedDeltaTime(Timestep NewFixedTime) 
    { 
        FixedDeltaTime = NewFixedTime; 
        AccumulatorLimit = FixedDeltaTime * MaxIterations; 
    }

    PhysicsSystem::Timestep PhysicsSystem::GetFixedDeltaTime()
    {
        return FixedDeltaTime; 
    }

    void PhysicsSystem::OnRigidbodyAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        InitializeRigidbody(rb->component);
        if (m_physicsToGameObjectLookup.contains(rb->component.object.id) == false)
        {
            auto goc = m_world->get_component<GameObjectComponent>(rb->entityID);
            m_physicsToGameObjectLookup.insert({ rb->component.object.id, goc.Id });
        }
    }

    void PhysicsSystem::OnRigidbodyRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        // Remove Data from lookup table
        ASSERT_MSG(m_physicsToGameObjectLookup.contains(rb->component.object.id) == false, "This should never happen!");

        if (m_physicsToGameObjectLookup.contains(rb->component.object.id))
        {
            m_physicsToGameObjectLookup.erase(rb->component.object.id);
        }

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
        InitializeBoxCollider(rb_comp);
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
        InitializeCapsuleCollider(rb_comp);
    }

    void PhysicsSystem::OnCapsuleColliderRemove(Ecs::ComponentEvent<CapsuleColliderComponent>* cc)
    {
        auto& rb_comp = m_world->get_component<RigidbodyComponent>(cc->entityID);
        rb_comp.object.setShape(myPhysx::shape::none);
    }

    void PhysicsSystem::InitializeRigidbody(RigidbodyComponent& rb)
    {
        rb.object = m_physicsWorld.createInstance();
        rb.SetStatic(false);   // default to dynamic object.
        //default initialize material
        rb.object.setMaterial(PhysicsMaterial{});
    }

    void PhysicsSystem::InitializeBoxCollider(RigidbodyComponent& rb)
    {
        // create box
        rb.object.setShape(myPhysx::shape::box);
    }

    void PhysicsSystem::InitializeCapsuleCollider(RigidbodyComponent& rb)
    {
        // create box
        rb.object.setShape(myPhysx::shape::capsule);
    }

   /* void PhysicsSystem::OnSphereColliderAdd(Ecs::ComponentEvent<SphereColliderComponent>* rb)
    {
    }

    void PhysicsSystem::OnSphereColliderRemove(Ecs::ComponentEvent<SphereColliderComponent>* rb)
    {

    }*/

}
