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
#include "Ouroboros/Transform/TransformSystem.h"

#include "OO_Vulkan/src/DebugDraw.h"
#include "Ouroboros/ECS/ECS.h"

// test
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"
namespace oo
{
    PhysicsSystem::~PhysicsSystem()
    {
        EventManager::Unsubscribe<PhysicsSystem, GameObjectComponent::OnEnableEvent>(this, &PhysicsSystem::OnGameObjectEnable);
        EventManager::Unsubscribe<PhysicsSystem, GameObjectComponent::OnDisableEvent>(this, &PhysicsSystem::OnGameObjectDisable);
    }

    void PhysicsSystem::Init(Scene* scene)
    {
        m_scene = scene;

        EventManager::Subscribe<PhysicsSystem, GameObjectComponent::OnEnableEvent>(this, &PhysicsSystem::OnGameObjectEnable);
        EventManager::Subscribe<PhysicsSystem, GameObjectComponent::OnDisableEvent>(this, &PhysicsSystem::OnGameObjectDisable);

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

        m_world->SubscribeOnAddComponent<PhysicsSystem, SphereColliderComponent>(
            this, &PhysicsSystem::OnSphereColliderAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, SphereColliderComponent>(
            this, &PhysicsSystem::OnSphereColliderRemove);

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
        UpdateDuplicatedObjects();

        // Update global bounds of all objects
        UpdateGlobalBounds();
        
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateDynamics(Timestep deltaTime)
    {
        //TODO: Should remove eventually (perhaps?)
        EditorUpdate(deltaTime);
        
        // update the physics world using fixed dt.
        m_physicsWorld.updateScene(static_cast<float>(FixedDeltaTime));

        static Ecs::Query rb_query = Ecs::make_query<GameObjectComponent, TransformComponent, RigidbodyComponent>();
        
        // set position and orientation of our scene's dynamic rigidbodies with updated physics results
        m_world->for_each(rb_query, [&](GameObjectComponent& goc, TransformComponent& tf, RigidbodyComponent& rb)
        {
            // IMPORTANT NOTE!
            // position retrieve is presumably global position in physics world.
            // but we must remember to consider the scenegraph hierarchy 
            // when calculating its final transform.
            // therefore we find the delta change and apply it to its local transform.

            // we only update dynamic colliders
            if (rb.IsStatic() || rb.IsTrigger()) 
                return;
            
            // probably better to just constantly set this 3 instead of checking
            rb.object.lockPositionX(rb.LockXAxisPosition);
            rb.object.lockPositionY(rb.LockYAxisPosition);
            rb.object.lockPositionZ(rb.LockZAxisPosition);

            rb.object.lockRotationX(rb.LockXAxisRotation);
            rb.object.lockRotationY(rb.LockYAxisRotation);
            rb.object.lockRotationZ(rb.LockZAxisRotation);

            auto pos = rb.GetPositionInPhysicsWorld();
            auto delta_position = pos - tf.GetGlobalPosition() - rb.Offset; // Note: we minus offset here too to compensate!
            tf.SetGlobalPosition(tf.GetGlobalPosition() + delta_position);

            auto orientation = rb.GetOrientationInPhysicsWorld();
            auto delta_orientation = orientation - tf.GetGlobalRotationQuat().value;
            
            //LOG_TRACE("orientation {0},{1},{2},{3}", orientation.x, orientation.y, orientation.z, orientation.w);
            //LOG_TRACE("global orientation {0},{1},{2},{3}", tf.GetGlobalRotationQuat().value.x, tf.GetGlobalRotationQuat().value.y, tf.GetGlobalRotationQuat().value.z, tf.GetGlobalRotationQuat().value.w);
            //LOG_TRACE("delta orientation {0},{1},{2},{3}", delta_orientation.x, delta_orientation.y, delta_orientation.z, delta_orientation.w);
            //LOG_TRACE("local orientation {0},{1},{2},{3}", tf.GetRotationQuat().value.x, tf.GetRotationQuat().value.y, tf.GetRotationQuat().value.z, tf.GetRotationQuat().value.w);
           
            tf.SetGlobalOrientation({ tf.GetGlobalRotationQuat().value + delta_orientation });
            //tf.SetOrientation(orientation);

            m_world->Get_System<TransformSystem>()->UpdateSubTree(*m_scene->FindWithInstanceID(goc.Id), true);
        });

    }

    void PhysicsSystem::UpdatePhysicsResolution(Timestep deltaTime)
    {
        UpdateCallbacks();
    }

    void PhysicsSystem::UpdateDuplicatedObjects()
    {
        static Ecs::Query duplicated_rb_query = Ecs::make_raw_query<RigidbodyComponent, GameObjectComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_rb_query, [&](RigidbodyComponent& rbComp, GameObjectComponent& goc, DuplicatedComponent& dupComp)
            {
                InitializeRigidbody(rbComp);
                AddToLookUp(rbComp, goc);
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

        static Ecs::Query duplicated_rb_with_sphere_query = Ecs::make_raw_query<RigidbodyComponent, SphereColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_rb_with_sphere_query, [&](RigidbodyComponent& rbComp, SphereColliderComponent& scComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeSphereCollider(rbComp);
            });
    }

    void PhysicsSystem::UpdateGlobalBounds()
    {
        // Update physics World's objects position and Orientation
        static Ecs::Query rb_query = Ecs::make_query<TransformComponent, RigidbodyComponent>();
        m_world->for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
            {
                // only update for transformthat have changed
                //if (tf.HasChangedThisFrame)
                {
                    auto pos = tf.GetGlobalPosition();
                    auto quat = tf.GetGlobalRotationQuat();
                    rb.SetPosOrientation(pos + rb.Offset, quat);
                }

                // test and set trigger boolean based on serialize value
                if (rb.object.isTrigger() != rb.IsTrigger())
                    rb.object.setTriggerShape(rb.IsTrigger());
            });

        //Updating box collider's bounds 
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
            });

        //Updating capsule collider's bounds 
        static Ecs::Query capsuleColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, CapsuleColliderComponent>();
        m_world->for_each(capsuleColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, CapsuleColliderComponent& cc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate global radius and half height for capsule collider
                cc.GlobalRadius = cc.Radius * scale.y;          // for now lets just use y-axis
                cc.GlobalHalfHeight = cc.HalfHeight * scale.y;  // for now lets just use y axis

                // set capsule size
                rb.object.setCapsuleProperty(cc.GlobalRadius * 2, cc.GlobalHalfHeight * 2);
            });

        //Updating sphere collider's bounds 
        static Ecs::Query sphereColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, SphereColliderComponent>();
        m_world->for_each(sphereColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, SphereColliderComponent& sc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate global bounds of capsule
                //sc.GlobalBounds.center = sc.Bounds.center + pos;
                sc.GlobalRadius= sc.Radius * std::max(std::max(scale.x, scale.y), scale.z);

                // set capsule size
                rb.object.setSphereProperty(sc.GlobalRadius);
            });
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
                pte.State = PhysicsEventState::NONE;
                break;
            case myPhysx::trigger::onTriggerEnter:
                if(DebugMessges)
                    LOG_TRACE("Trigger Enter Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::ENTER;
                break;
            case myPhysx::trigger::onTriggerStay:
                if (DebugMessges)
                    LOG_TRACE("Trigger Stay Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::STAY;
                break;
            case myPhysx::trigger::onTriggerExit:
                if (DebugMessges) 
                    LOG_TRACE("Trigger Exit Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::EXIT;
                break;
            }
            EventManager::Broadcast(&pte);

            trigger_queue->pop();
        }
        m_physicsWorld.clearTriggerData();


        auto collision_queue = m_physicsWorld.getCollisionData();
        while (!collision_queue->empty())
        {
            myPhysx::ContactManifold contact_manifold = collision_queue->front();

            ASSERT_MSG(m_physicsToGameObjectLookup.contains(contact_manifold.shape1_ID) == false, "This should never happen");
            ASSERT_MSG(m_physicsToGameObjectLookup.contains(contact_manifold.shape2_ID) == false, "This should never happen");

            // retrieve their gameobject id counterpart.
            UUID collider1_go_id = m_physicsToGameObjectLookup.at(contact_manifold.shape1_ID);
            UUID collider2_go_id = m_physicsToGameObjectLookup.at(contact_manifold.shape2_ID);
            
            //broadcast trigger event.
            PhysicsCollisionEvent pce;
            pce.Collider1 = collider1_go_id;
            pce.Collider2 = collider2_go_id;
            pce.ContactCount = contact_manifold.contactCount;
            for(auto& elem : contact_manifold.m_contactPoint)
                pce.ContactPoints.emplace_back(oo::ContactPoint{ {elem.normal.x,elem.normal.y, elem.normal.z} , {elem.point.x, elem.point.y, elem.point.z} , {elem.impulse.x, elem.impulse.y, elem.impulse.z} });

            switch (contact_manifold.status)
            {
            case myPhysx::collision::none:
                pce.State = PhysicsEventState::NONE;
                break;
            case myPhysx::collision::onCollisionEnter:
                if (DebugMessges)LOG_TRACE("Collision Enter Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::ENTER;
                break;
            case myPhysx::collision::onCollisionStay:
                if(DebugMessges) LOG_TRACE("Collision Stay Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::STAY;
                break;
            case myPhysx::collision::onCollisionExit:
                if (DebugMessges) LOG_TRACE("Collision Exit Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::EXIT;
                break;
            }
            EventManager::Broadcast(&pce);

            collision_queue->pop();
        }
        m_physicsWorld.clearCollisionData();
    }

    void PhysicsSystem::PostUpdate()
    {
        // Update entire subtree once every Fixed update so physics changes are properly reflected.
        //m_world->Get_System<TransformSystem>()->UpdateSubTree(*m_scene->GetRoot(), false);
    }


    void PhysicsSystem::RenderDebugColliders()
    {
        if (ColliderDebugDraw == false)
            return;

        TRACY_PROFILE_SCOPE_NC(physics_debug_draw, tracy::Color::PeachPuff);
       
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
            glm::vec3 GlobalHalfExtents = { cc.GlobalRadius, cc.GlobalHalfHeight , cc.GlobalRadius };
            
            //Debug draw the bounds
            DebugDraw::AddAABB({ pos + GlobalHalfExtents  , pos - GlobalHalfExtents }, oGFX::Colors::GREEN);
            // draw top sphere
            DebugDraw::AddSphere({ pos + vec3{ 0, GlobalHalfExtents.y, 0}, cc.GlobalRadius }, oGFX::Colors::GREEN);
            // draw bottom sphere
            DebugDraw::AddSphere({ pos - vec3{ 0, GlobalHalfExtents.y, 0}, cc.GlobalRadius }, oGFX::Colors::GREEN);
        });

        //Updating capsule collider's bounds and debug drawing
        static Ecs::Query sphereColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, SphereColliderComponent>();
        m_world->for_each(sphereColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, SphereColliderComponent& sc)
            {
                auto pos = rb.GetPositionInPhysicsWorld();
                auto scale = tf.GetGlobalScale();
                auto quat = rb.GetOrientationInPhysicsWorld();

                // debug Draw the sphere collider
                DebugDraw::AddSphere({ pos, sc.GlobalRadius }, oGFX::Colors::GREEN);
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
        AddToLookUp(rb->component, m_world->get_component<GameObjectComponent>(rb->entityID));
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
        if(m_world->has_component<SphereColliderComponent>(rb->entityID))
            m_world->remove_component<SphereColliderComponent>(rb->entityID);
        if(m_world->has_component<CapsuleColliderComponent>(rb->entityID))
            m_world->remove_component<CapsuleColliderComponent>(rb->entityID);

        // finally we remove the physics object
        m_physicsWorld.removeInstance(rb->component.object);

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
        // need this safeguard to be sure. otherwise crash.
        if (m_world->has_component<RigidbodyComponent>(bc->entityID))
        {
            auto& rb_comp = m_world->get_component<RigidbodyComponent>(bc->entityID);
            rb_comp.object.removeShape();
            //rb_comp.object.setShape(myPhysx::shape::none);
        }
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
        // need this safeguard to be sure. otherwise crash.
        if (m_world->has_component<RigidbodyComponent>(cc->entityID))
        {
            auto& rb_comp = m_world->get_component<RigidbodyComponent>(cc->entityID);
            rb_comp.object.removeShape();
            //rb_comp.object.setShape(myPhysx::shape::none);
        }
    }

    void PhysicsSystem::OnSphereColliderAdd(Ecs::ComponentEvent<SphereColliderComponent>* sc)
    {
        // if sphere collider is directly added, ensure we add rigidbody too.
        if (m_world->has_component<RigidbodyComponent>(sc->entityID) == false)
        {
            m_world->add_component<RigidbodyComponent>(sc->entityID);
        }

        auto& rb_comp = m_world->get_component<RigidbodyComponent>(sc->entityID);
        InitializeSphereCollider(rb_comp);
    }

    void PhysicsSystem::OnSphereColliderRemove(Ecs::ComponentEvent<SphereColliderComponent>* sc)
    {
        // need this safeguard to be sure. otherwise crash.
        if (m_world->has_component<RigidbodyComponent>(sc->entityID))
        {
            auto& rb_comp = m_world->get_component<RigidbodyComponent>(sc->entityID);
            rb_comp.object.removeShape();
            //rb_comp.object.setShape(myPhysx::shape::none);
        }
    }

    void PhysicsSystem::InitializeRigidbody(RigidbodyComponent& rb)
    {
        rb.object = m_physicsWorld.createInstance();
        rb.SetStatic(true); // default to static objects. Most things in the world should be static.
        //rb.EnableGravity(); // most things in the world should have gravity enabled (?)
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
        // create capsule
        rb.object.setShape(myPhysx::shape::capsule);
    }

    void PhysicsSystem::InitializeSphereCollider(RigidbodyComponent& rb)
    {
        // create sphere
        rb.object.setShape(myPhysx::shape::sphere);
    }

    void PhysicsSystem::AddToLookUp(RigidbodyComponent& rb, GameObjectComponent& goc)
    {
        if (m_physicsToGameObjectLookup.contains(rb.object.id) == false)
        {
            m_physicsToGameObjectLookup.insert({ rb.object.id, goc.Id });
        }
    }

    void PhysicsSystem::OnGameObjectEnable(GameObjectComponent::OnEnableEvent* e)
    {
        auto go = m_scene->FindWithInstanceID(e->Id);
        if (go->HasComponent<RigidbodyComponent>())
        {
            auto& rb = go->GetComponent<RigidbodyComponent>();
            rb.EnableCollider();
        }
    }

    void PhysicsSystem::OnGameObjectDisable(GameObjectComponent::OnDisableEvent* e)
    {
        auto go = m_scene->FindWithInstanceID(e->Id);
        if (go->HasComponent<RigidbodyComponent>())
        {
            auto& rb = go->GetComponent<RigidbodyComponent>();
            rb.DisableCollider();
        }
    }


}
