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

#include "Ouroboros/Vulkan/MeshRendererComponent.h"

// test
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"
namespace oo
{
    PhysicsSystem::~PhysicsSystem()
    {
        EventManager::Unsubscribe<PhysicsSystem, GameObjectComponent::OnEnableEvent>(this, &PhysicsSystem::OnGameObjectEnable);
        EventManager::Unsubscribe<PhysicsSystem, GameObjectComponent::OnDisableEvent>(this, &PhysicsSystem::OnGameObjectDisable);
        EventManager::Unsubscribe<PhysicsSystem, RaycastEvent>(this, &PhysicsSystem::OnRaycastEvent);
        EventManager::Unsubscribe<PhysicsSystem, RaycastAllEvent>(this, &PhysicsSystem::OnRaycastAllEvent);
    }

    void PhysicsSystem::Init(Scene* scene)
    {
        m_scene = scene;

        EventManager::Subscribe<PhysicsSystem, GameObjectComponent::OnEnableEvent>(this, &PhysicsSystem::OnGameObjectEnable);
        EventManager::Subscribe<PhysicsSystem, GameObjectComponent::OnDisableEvent>(this, &PhysicsSystem::OnGameObjectDisable);
        EventManager::Subscribe<PhysicsSystem, RaycastEvent>(this, &PhysicsSystem::OnRaycastEvent);
        EventManager::Subscribe<PhysicsSystem, RaycastAllEvent>(this, &PhysicsSystem::OnRaycastAllEvent);

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

        m_world->SubscribeOnAddComponent<PhysicsSystem, MeshColliderComponent>(
            this, &PhysicsSystem::OnMeshColliderAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, MeshColliderComponent>(
            this, &PhysicsSystem::OnMeshColliderRemove);

        phy::physx_system::setCurrentWorld(&m_physicsWorld);
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

            {
                TRACY_PROFILE_SCOPE_NC(physics_fixed_dt_broadcast, tracy::Color::PeachPuff2);
                PhysicsTickEvent e;
                e.DeltaTime = FixedDeltaTime;
                EventManager::Broadcast(&e);
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
        
        {
            TRACY_PROFILE_SCOPE_NC(physX_backend_simulate, tracy::Color::Brown)
            // update the physics world using fixed dt.
            m_physicsWorld.updateWorld(static_cast<float>(FixedDeltaTime));
            TRACY_PROFILE_SCOPE_END();
        }

        // Transform System updates via the scenegraph because the order matters
        //auto const& graph = m_scene->GetGraph();
        //scenegraph::shared_pointer root_node = graph.get_root();
        //std::stack<scenenode::shared_pointer> s;
        //scenenode::shared_pointer curr = root_node;

        ///* Multithread Method of updating */

        //// Step 1. Extra Pre-Processing Overhead
        //{
        //    TRACY_PROFILE_SCOPE_NC(pre_process_overhead, tracy::Color::Gold3);

        //    for (auto& group : rigidbody_groups)
        //        group.clear();

        //    s.emplace(curr);
        //    while (!s.empty())
        //    {
        //        curr = s.top();
        //        s.pop();
        //        for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
        //        {
        //            scenenode::shared_pointer child = *iter;
        //            if (child->has_child())
        //                s.emplace(child);
        //        }
        //        auto childs = curr->get_direct_child();
        //        auto child_depth = curr->get_depth() + 1;
        //        for (auto& child : childs)
        //        {
        //            auto const go = m_scene->FindWithInstanceID(child->get_handle());
        //            if(go->HasComponent<RigidbodyComponent>())
        //                rigidbody_groups[child_depth].emplace_back(go);
        //        }
        //        assert(child_depth != 0);
        //    }

        //    TRACY_PROFILE_SCOPE_END();
        //}

        //// Step 2. processing.
        //for (auto& group : rigidbody_groups)
        //{
        //    if (group.size() <= 0)
        //        continue;

        //    jobsystem::job per_group_update{};

        //    for (auto& elem : group)
        //    {
        //        // submitting work.
        //        jobsystem::submit(per_group_update, [&]()
        //            {
        //                auto& rb = elem->GetComponent<RigidbodyComponent>();
        //                auto& tf = elem->GetComponent<TransformComponent>();

        //                // we only update dynamic colliders
        //                if (rb.IsStatic() || rb.IsTrigger())
        //                    return;

        //                // probably better to just constantly set this 3 instead of checking
        //                rb.object.lockPositionX(rb.LockXAxisPosition);
        //                rb.object.lockPositionY(rb.LockYAxisPosition);
        //                rb.object.lockPositionZ(rb.LockZAxisPosition);

        //                rb.object.lockRotationX(rb.LockXAxisRotation);
        //                rb.object.lockRotationY(rb.LockYAxisRotation);
        //                rb.object.lockRotationZ(rb.LockZAxisRotation);

        //                auto pos = rb.GetPositionInPhysicsWorld();
        //                auto delta_position = pos - tf.GetGlobalPosition() - rb.Offset; // Note: we minus offset here too to compensate!
        //                tf.SetGlobalPosition(tf.GetGlobalPosition() + delta_position);

        //                auto orientation = rb.GetOrientationInPhysicsWorld();
        //                auto delta_orientation = orientation - tf.GetGlobalRotationQuat().value;

        //                tf.SetGlobalOrientation({ tf.GetGlobalRotationQuat().value + delta_orientation });
        //            });
        //    }

        //    m_world->Get_System<TransformSystem>()->UpdateSubTree(*m_scene->GetRoot(), false);

        //    jobsystem::wait(per_group_update);
        //}

        ///* Single threaded method */
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

            //// probably better to just constantly set this 3 instead of checking
            //rb.object.lockPositionX(rb.LockXAxisPosition);
            //rb.object.lockPositionY(rb.LockYAxisPosition);
            //rb.object.lockPositionZ(rb.LockZAxisPosition);

            //rb.object.lockRotationX(rb.LockXAxisRotation);
            //rb.object.lockRotationY(rb.LockYAxisRotation);
            //rb.object.lockRotationZ(rb.LockZAxisRotation);

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

        });

        m_world->Get_System<TransformSystem>()->UpdateEntireTree();
    }

    void PhysicsSystem::UpdatePhysicsResolution(Timestep deltaTime)
    {
        UpdateCallbacks();
    }

    void PhysicsSystem::UpdateDuplicatedObjects()
    {
        TRACY_PROFILE_SCOPE_NC(physics_update_duplicated_objects, tracy::Color::PeachPuff);

        static Ecs::Query duplicated_rb_query = Ecs::make_raw_query<RigidbodyComponent, GameObjectComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_rb_query, [&](RigidbodyComponent& rbComp, GameObjectComponent& goc, DuplicatedComponent& dupComp)
            {
                DuplicateRigidbody(rbComp);
                AddToLookUp(rbComp, goc);
            });

        //jobsystem::job initialization_job{};

        static Ecs::Query duplicated_rb_with_box_query = Ecs::make_raw_query<RigidbodyComponent, BoxColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_rb_with_box_query, [&](RigidbodyComponent& rbComp, BoxColliderComponent& bcComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                /*jobsystem::submit(initialization_job, [&]() 
                {*/
                    InitializeBoxCollider(rbComp);
                /*});*/
            });

        static Ecs::Query duplicated_rb_with_capsule_query = Ecs::make_raw_query<RigidbodyComponent, CapsuleColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_rb_with_capsule_query, [&](RigidbodyComponent& rbComp, CapsuleColliderComponent& ccComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                /*jobsystem::submit(initialization_job, [&]()
                    {*/
                        InitializeCapsuleCollider(rbComp);
                    /*});*/
            });

        static Ecs::Query duplicated_rb_with_sphere_query = Ecs::make_raw_query<RigidbodyComponent, SphereColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_rb_with_sphere_query, [&](RigidbodyComponent& rbComp, SphereColliderComponent& scComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                /*jobsystem::submit(initialization_job, [&]()
                    {*/
                        InitializeSphereCollider(rbComp);
                    /*});*/
            });

        static Ecs::Query duplicated_rb_with_mesh_query = Ecs::make_raw_query<RigidbodyComponent, MeshColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_rb_with_mesh_query, [&](RigidbodyComponent& rbComp, MeshColliderComponent& mcComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                /*jobsystem::submit(initialization_job, [&]()
                    {*/
                InitializeMeshCollider(rbComp);
                /*});*/
            });

        /*jobsystem::wait(initialization_job);*/
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateGlobalBounds()
    {
        TRACY_PROFILE_SCOPE_NC(physics_update_global_bounds, tracy::Color::PeachPuff);

        //jobsystem::job update_global_bounds_job{};

        // Update physics World's objects position and Orientation
        static Ecs::Query rb_query = Ecs::make_query<TransformComponent, RigidbodyComponent>();
        m_world->for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
            {
                TRACY_PROFILE_SCOPE_NC(submit_update_rigidbodies, tracy::Color::PeachPuff2);


                //jobsystem::submit(update_global_bounds_job, [&]()
                    //{
                        TRACY_PROFILE_SCOPE_NC(individual_rigidbody_update, tracy::Color::PeachPuff3);
                        // only update for transformthat have changed
                            //if (tf.HasChangedThisFrame)
                            {
                                TRACY_PROFILE_SCOPE_NC(rigidbody_set_pos_orientation, tracy::Color::PeachPuff4);
                                auto pos = tf.GetGlobalPosition();
                                auto quat = tf.GetGlobalRotationQuat();
                                rb.SetPosOrientation(pos + rb.Offset, quat);
                                TRACY_PROFILE_SCOPE_END();
                            }

                            {
                                TRACY_PROFILE_SCOPE_NC(rigidbody_is_trigger_check, tracy::Color::PeachPuff4);
                                // test and set trigger boolean based on serialize value
                                /*if (rb.object.isTrigger() != rb.IsTrigger())
                                    rb.object.setTriggerShape(rb.IsTrigger());
                                rb.desired_object.is_trigger;*/
                                TRACY_PROFILE_SCOPE_END();
                            }
                            
                        TRACY_PROFILE_SCOPE_END();
                    //});
                
                TRACY_PROFILE_SCOPE_END();
            });

        //jobsystem::launch_and_wait(update_global_bounds_job);

        //Updating box collider's bounds 
        static Ecs::Query boxColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, BoxColliderComponent>();
        m_world->for_each(boxColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, BoxColliderComponent& bc)
            {
                /*jobsystem::submit(update_global_bounds_job, [&]()
                    {*/
                        auto pos = tf.GetGlobalPosition();
                        auto scale = tf.GetGlobalScale();
                        auto quat = tf.GetGlobalRotationQuat();

                        // calculate global bounds and half extents
                        bc.GlobalHalfExtents = { bc.HalfExtents * bc.Size * scale };

                        // set box size
                        rb.underlying_object.shape_type = phy::shape::box;
                        rb.underlying_object.box = { bc.GlobalHalfExtents.x, bc.GlobalHalfExtents.y, bc.GlobalHalfExtents.z };
                    //});
            });

        //jobsystem::wait(update_global_bounds_job);

        //Updating capsule collider's bounds 
        static Ecs::Query capsuleColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, CapsuleColliderComponent>();
        m_world->for_each(capsuleColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, CapsuleColliderComponent& cc)
            {
                /*jobsystem::submit(update_global_bounds_job, [&]()
                    {*/
                        auto pos = tf.GetGlobalPosition();
                        auto scale = tf.GetGlobalScale();
                        auto quat = tf.GetGlobalRotationQuat();

                        // calculate global radius and half height for capsule collider
                        cc.GlobalRadius = cc.Radius * scale.y;          // for now lets just use y-axis
                        cc.GlobalHalfHeight = cc.HalfHeight * scale.y;  // for now lets just use y axis

                        // set capsule size
                        rb.underlying_object.shape_type = phy::shape::capsule;
                        rb.underlying_object.capsule = { cc.GlobalRadius, cc.GlobalHalfHeight };
                    //});
            });

        //jobsystem::wait(update_global_bounds_job);

        //Updating sphere collider's bounds 
        static Ecs::Query sphereColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, SphereColliderComponent>();
        m_world->for_each(sphereColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, SphereColliderComponent& sc)
            {
                /*jobsystem::submit(update_global_bounds_job, [&]()
                    {*/
                        auto pos = tf.GetGlobalPosition();
                        auto scale = tf.GetGlobalScale();
                        auto quat = tf.GetGlobalRotationQuat();

                        // calculate global bounds of capsule
                        //sc.GlobalBounds.center = sc.Bounds.center + pos;
                        sc.GlobalRadius= sc.Radius * std::max(std::max(scale.x, scale.y), scale.z);

                        // set capsule size
                        rb.underlying_object.shape_type = phy::shape::sphere;
                        rb.underlying_object.sphere = { sc.GlobalRadius };
                    //});
            });


        TRACY_PROFILE_SCOPE_NC(physics_update_mesh_collider_bounds, tracy::Color::PeachPuff);
        //Updating mesh collider's bounds 
        static Ecs::Query meshColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, MeshColliderComponent, MeshRendererComponent>();
        m_world->for_each(meshColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, MeshColliderComponent& mc, MeshRendererComponent& mr)
            {
                if (mc.Vertices.empty() || mc.Reset == true)
                {
                    mc.Vertices.clear();

                    auto& vertices = mr.MeshInformation.mesh_handle.GetData<ModelFileResource*>()->vertices;
                    for (auto& vertex : vertices)
                        mc.Vertices.emplace_back(vertex.pos);

                    mc.Reset = false;
                }
            });
        TRACY_PROFILE_SCOPE_END();

        //jobsystem::wait(update_global_bounds_job);

        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateCallbacks()
    {

        TRACY_PROFILE_SCOPE_NC(physics_trigger_resoltion, tracy::Color::PeachPuff);
        auto trigger_queue = m_physicsWorld.getTriggerData();
        while (!trigger_queue.empty())
        {
            phy::TriggerManifold trigger_manifold = trigger_queue.front();

            // if either objects are already removed, we skip them
            if (m_physicsToGameObjectLookup.contains(trigger_manifold.triggerID) == false
                || m_physicsToGameObjectLookup.contains(trigger_manifold.otherID) == false)
            {
                if (DebugMessages)
                    LOG_WARN("Skipped Trigger Callback because one of these pair of (Physics)UUID died ({0}), ({1}) ", trigger_manifold.triggerID, trigger_manifold.otherID);

                trigger_queue.pop();
                continue;
            }

            //ASSERT_MSG(m_physicsToGameObjectLookup.contains(trigger_manifold.triggerID) == false, "This should never happen");
            //ASSERT_MSG(m_physicsToGameObjectLookup.contains(trigger_manifold.otherID) == false, "This should never happen");

            // retrieve their gameobject id counterpart.
            UUID trigger_go_id = m_physicsToGameObjectLookup.at(trigger_manifold.triggerID);
            UUID other_go_id = m_physicsToGameObjectLookup.at(trigger_manifold.otherID);
            
            //broadcast trigger event.
            PhysicsTriggerEvent pte;
            pte.TriggerID = trigger_go_id;
            pte.OtherID = other_go_id;
            switch (trigger_manifold.status)
            {
            case phy::trigger::none:
                pte.State = PhysicsEventState::NONE;
                break;
            case phy::trigger::onTriggerEnter:
                if(DebugMessages)
                    LOG_TRACE("Trigger Enter Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::ENTER;
                break;
            case phy::trigger::onTriggerStay:
                if (DebugMessages)
                    LOG_TRACE("Trigger Stay Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::STAY;
                break;
            case phy::trigger::onTriggerExit:
                if (DebugMessages) 
                    LOG_TRACE("Trigger Exit Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::EXIT;
                break;
            }


            TRACY_PROFILE_SCOPE_NC(broadcast_physics_trigger_event, tracy::Color::PeachPuff);
            EventManager::Broadcast(&pte);
            TRACY_PROFILE_SCOPE_END();

            trigger_queue.pop();
        }
        m_physicsWorld.clearTriggerData();
        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_NC(physics_collision_resoltion, tracy::Color::PeachPuff);
        
        //jobsystem::job collision_resolution;

        auto collision_queue = m_physicsWorld.getCollisionData();
        while (!collision_queue.empty())
        {
            phy::ContactManifold contact_manifold = collision_queue.front();

            // if either objects are already removed, we skip them
            if (m_physicsToGameObjectLookup.contains(contact_manifold.shape1_ID) == false
                || m_physicsToGameObjectLookup.contains(contact_manifold.shape2_ID) == false)
            {
                if (DebugMessages)
                    LOG_WARN("Skipped Physics Collision Callback because one of these pair of (Physics)UUID died ({0}), ({1}) ", contact_manifold.shape1_ID, contact_manifold.shape2_ID);

                collision_queue.pop();
                continue;
            }
            //ASSERT_MSG(m_physicsToGameObjectLookup.contains(contact_manifold.shape1_ID) == false, "This should never happen");
            //ASSERT_MSG(m_physicsToGameObjectLookup.contains(contact_manifold.shape2_ID) == false, "This should never happen");

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
            case phy::collision::none:
                pce.State = PhysicsEventState::NONE;
                break;
            case phy::collision::onCollisionEnter:
                if (DebugMessages)LOG_TRACE("Collision Enter Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::ENTER;
                break;
            case phy::collision::onCollisionStay:
                if(DebugMessages) LOG_TRACE("Collision Stay Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::STAY;
                break;
            case phy::collision::onCollisionExit:
                if (DebugMessages) LOG_TRACE("Collision Exit Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::EXIT;
                break;
            }

            /*jobsystem::submit(collision_resolution, [&]()
            {*/
                TRACY_PROFILE_SCOPE_NC(broadcast_physics_collision_event, tracy::Color::PeachPuff);
                EventManager::Broadcast(&pce);
                TRACY_PROFILE_SCOPE_END();
            //});

            collision_queue.pop();
        }
        m_physicsWorld.clearCollisionData();

        //jobsystem::wait(collision_resolution);

        TRACY_PROFILE_SCOPE_END();
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

            glm::vec3 rotatedX = glm::rotate(quat, glm::vec3{bc.GlobalHalfExtents.x, 0, 0});
            glm::vec3 rotatedY = glm::rotate(quat, glm::vec3{0, bc.GlobalHalfExtents.y, 0});
            glm::vec3 rotatedZ = glm::rotate(quat, glm::vec3{0, 0, bc.GlobalHalfExtents.z});

            //glm::vec3 rotatedVal = glm::conjugate(quat) * bc.GlobalHalfExtents * quat;
            ////glm::vec3 rotatedVal = glm::mat4_cast(quat) * glm::vec4{ bc.HalfExtents, 0 };
            //rotatedVal *= bc.Size * tf.GetGlobalScale();

            auto bottom_left_back   = pos - rotatedX - rotatedY - rotatedZ;
            auto bottom_right_back  = pos + rotatedX - rotatedY - rotatedZ;
            auto top_left_back      = pos - rotatedX + rotatedY - rotatedZ;
            auto top_right_back     = pos + rotatedX + rotatedY - rotatedZ;
            auto bottom_left_front  = pos - rotatedX - rotatedY + rotatedZ;
            auto bottom_right_front = pos + rotatedX - rotatedY + rotatedZ;
            auto top_left_front     = pos - rotatedX + rotatedY + rotatedZ;
            auto top_right_front    = pos + rotatedX + rotatedY + rotatedZ;

            //Debug draw the bounds
            DebugDraw::AddLine(bottom_left_back, bottom_left_front, oGFX::Colors::GREEN);
            DebugDraw::AddLine(bottom_left_front, bottom_right_front, oGFX::Colors::GREEN);
            DebugDraw::AddLine(bottom_right_front, bottom_right_back, oGFX::Colors::GREEN);
            DebugDraw::AddLine(bottom_right_back, bottom_left_back, oGFX::Colors::GREEN);

            DebugDraw::AddLine(top_left_back, top_left_front, oGFX::Colors::GREEN);
            DebugDraw::AddLine(top_left_front, top_left_front, oGFX::Colors::GREEN);
            DebugDraw::AddLine(top_right_front, top_right_back, oGFX::Colors::GREEN);
            DebugDraw::AddLine(top_right_back, top_left_back, oGFX::Colors::GREEN);

            DebugDraw::AddLine(bottom_left_back, top_left_back, oGFX::Colors::GREEN);
            DebugDraw::AddLine(bottom_left_front, top_left_front, oGFX::Colors::GREEN);
            DebugDraw::AddLine(bottom_right_front, top_right_front, oGFX::Colors::GREEN);
            DebugDraw::AddLine(bottom_right_back, top_right_back, oGFX::Colors::GREEN);

            //DebugDraw::AddAABB({ pos + bc.GlobalHalfExtents  , pos - bc.GlobalHalfExtents }, oGFX::Colors::GREEN);
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
            DebugDraw::AddAABB({ pos - GlobalHalfExtents  , pos + GlobalHalfExtents }, oGFX::Colors::GREEN);
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

    RaycastResult PhysicsSystem::Raycast(Ray ray, float distance)
    {
        auto result = m_physicsWorld.raycast({ ray.Position.x, ray.Position.y, ray.Position.z }, { ray.Direction.x, ray.Direction.y, ray.Direction.z }, distance);
        
        if(result.intersect)
            ASSERT_MSG(m_physicsToGameObjectLookup.contains(result.object_ID) == false, "Why am i hitting something that's not in the current world?");
        
        return { result.intersect, m_physicsToGameObjectLookup.at(result.object_ID), {result.position.x,result.position.y, result.position.z}, 
            { result.normal.x, result.normal.y, result.normal.z }, result.distance };
    }

    std::vector<RaycastResult> PhysicsSystem::RaycastAll(Ray ray, float distance)
    {
        std::vector<RaycastResult> result;

        auto allHits = m_physicsWorld.raycastAll({ ray.Position.x, ray.Position.y, ray.Position.z }, { ray.Direction.x, ray.Direction.y, ray.Direction.z }, distance);

        for (auto& hit : allHits)
        {
            if (hit.intersect == false)
                continue;

            ASSERT_MSG(m_physicsToGameObjectLookup.contains(hit.object_ID) == false, "Why am i hitting something that's not in the current world?");

            RaycastResult new_entry = 
            { hit.intersect
                , m_physicsToGameObjectLookup.at(hit.object_ID)
                , { hit.position.x,hit.position.y, hit.position.z }
                , { hit.normal.x, hit.normal.y, hit.normal.z }
                , hit.distance 
            };

            result.emplace_back(new_entry);
        }

        return result;
    }

    void PhysicsSystem::OnRigidbodyAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        InitializeRigidbody(rb->component);
        AddToLookUp(rb->component, m_world->get_component<GameObjectComponent>(rb->entityID));
    }

    void PhysicsSystem::OnRigidbodyRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        // Remove Data from lookup table
        ASSERT_MSG(m_physicsToGameObjectLookup.contains(rb->component.underlying_object.id) == false, "This should never happen!");

        if (m_physicsToGameObjectLookup.contains(rb->component.underlying_object.id))
        {
            m_physicsToGameObjectLookup.erase(rb->component.underlying_object.id);
        }
        //Remove all other colliders as well

        if (m_world->has_component<BoxColliderComponent>(rb->entityID))
            m_world->remove_component<BoxColliderComponent>(rb->entityID);
        if(m_world->has_component<SphereColliderComponent>(rb->entityID))
            m_world->remove_component<SphereColliderComponent>(rb->entityID);
        if(m_world->has_component<CapsuleColliderComponent>(rb->entityID))
            m_world->remove_component<CapsuleColliderComponent>(rb->entityID);
        if (m_world->has_component<MeshColliderComponent>(rb->entityID))
            m_world->remove_component<MeshColliderComponent>(rb->entityID);

        // finally we remove the physics object
        m_physicsWorld.removeInstance(rb->component.underlying_object);

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
            rb_comp.desired_object.shape_type = phy::shape::none;
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
            rb_comp.desired_object.shape_type = phy::shape::none;
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
            rb_comp.desired_object.shape_type = phy::shape::none;
            //rb_comp.object.setShape(myPhysx::shape::none);
        }
    }

    void PhysicsSystem::OnMeshColliderAdd(Ecs::ComponentEvent<MeshColliderComponent>* mc)
    {
        // if mesh collider is directly added, ensure we add rigidbody too.
        if (m_world->has_component<RigidbodyComponent>(mc->entityID) == false)
        {
            m_world->add_component<RigidbodyComponent>(mc->entityID);
        }

        auto& rb_comp = m_world->get_component<RigidbodyComponent>(mc->entityID);
        InitializeMeshCollider(rb_comp);

        //bool has_mr_comp = m_world->has_component<MeshRendererComponent>(mc->entityID);
        //if (has_mr_comp)
        //{
        //    auto& mr_comp = m_world->get_component<MeshRendererComponent>(mc->entityID);
        //    auto& vertices = mr_comp.MeshInformation.mesh_handle.GetData<ModelFileResource*>()->vertices;
        //    //mc->component.Vertices.clear();
        //    for (auto& vertex : vertices)
        //    {
        //        mc->component.Vertices.emplace_back(vertex.pos);
        //    }
        //}
    }

    void PhysicsSystem::OnMeshColliderRemove(Ecs::ComponentEvent<MeshColliderComponent>* mc)
    {
        // need this safeguard to be sure. otherwise crash.
        if (m_world->has_component<RigidbodyComponent>(mc->entityID))
        {
            auto& rb_comp = m_world->get_component<RigidbodyComponent>(mc->entityID);
            rb_comp.desired_object.shape_type = phy::shape::none;
            //rb_comp.object.setShape(myPhysx::shape::none);
        }
    }

    void PhysicsSystem::InitializeRigidbody(RigidbodyComponent& rb)
    {
        rb.underlying_object = rb.desired_object = m_physicsWorld.createInstance();
        rb.SetStatic(true); // default to static objects. Most things in the world should be static.
        //rb.EnableGravity(); // most things in the world should have gravity enabled (?)
        //default initialize material
        //rb.desired_object.material = PhysicsMaterial{};
    }

    void PhysicsSystem::InitializeBoxCollider(RigidbodyComponent& rb)
    {
        // create box
        rb.desired_object.shape_type = phy::shape::box;
    }

    void PhysicsSystem::InitializeCapsuleCollider(RigidbodyComponent& rb)
    {
        // create capsule
        rb.desired_object.shape_type = phy::shape::capsule;
    }

    void PhysicsSystem::InitializeSphereCollider(RigidbodyComponent& rb)
    {
        // create sphere
        rb.desired_object.shape_type = phy::shape::sphere;
    }

    void PhysicsSystem::InitializeMeshCollider(RigidbodyComponent& rb)
    {
        // create box
        //rb.object.setShape(myPhysx::shape::mesh);
    }

    void PhysicsSystem::DuplicateRigidbody(RigidbodyComponent& rb)
    {
        // we duplicate instead if this is an existing object
        if (m_physicsWorld.hasObject(rb.underlying_object.id))
            rb.underlying_object = rb.desired_object = m_physicsWorld.duplicateObject(rb.underlying_object.id);
    }

    void PhysicsSystem::AddToLookUp(RigidbodyComponent& rb, GameObjectComponent& goc)
    {
        if (m_physicsToGameObjectLookup.contains(rb.underlying_object.id) == false)
        {
            m_physicsToGameObjectLookup.insert({ rb.underlying_object.id, goc.Id });
        }
    }

    void PhysicsSystem::OnGameObjectEnable(GameObjectComponent::OnEnableEvent* e)
    {
        auto go = m_scene->FindWithInstanceID(e->Id);
        if (go && go->HasComponent<RigidbodyComponent>())
        {
            auto& rb = go->GetComponent<RigidbodyComponent>();
            rb.EnableCollider();
        }
    }

    void PhysicsSystem::OnGameObjectDisable(GameObjectComponent::OnDisableEvent* e)
    {
        auto go = m_scene->FindWithInstanceID(e->Id);
        if (go && go->HasComponent<RigidbodyComponent>())
        {
            auto& rb = go->GetComponent<RigidbodyComponent>();
            rb.DisableCollider();
        }
    }

    void PhysicsSystem::OnRaycastEvent(RaycastEvent* e)
    {
        e->Results = Raycast(e->ray, e->distance);
    }

    void PhysicsSystem::OnRaycastAllEvent(RaycastAllEvent* e)
    {
        e->Results = RaycastAll(e->ray, e->distance);
    }


}
