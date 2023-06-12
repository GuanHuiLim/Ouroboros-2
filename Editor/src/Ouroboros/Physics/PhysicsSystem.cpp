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

#include "Utility/Hash.h"

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

        m_world->SubscribeOnAddComponent<PhysicsSystem, ConvexColliderComponent>(
            this, &PhysicsSystem::OnMeshColliderAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, ConvexColliderComponent>(
            this, &PhysicsSystem::OnMeshColliderRemove);

        myPhysx::physx_system::setCurrentWorld(&m_physicsWorld);
    }

    void PhysicsSystem::PostLoadSceneInit()
    {
        TRACY_PROFILE_SCOPE_NC(post_load_scene_init, tracy::Color::PeachPuff2);

        // Update physics World's objects position and Orientation
        static Ecs::Query rb_query = Ecs::make_raw_query<TransformComponent, RigidbodyComponent>();
        m_world->parallel_for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
            {
                {
                    TRACY_PROFILE_SCOPE_NC(rigidbody_set_pos_orientation, tracy::Color::PeachPuff4);
                    oo::vec3 pos = tf.GetGlobalPosition();
                    oo::quat quat = tf.GetGlobalRotationQuat();
                    rb.SetPosOrientation(pos + rb.Offset, quat);
                    TRACY_PROFILE_SCOPE_END();
                }

                {
                    TRACY_PROFILE_SCOPE_NC(rigidbody_is_trigger_check, tracy::Color::PeachPuff4);
                    //// test and set trigger boolean based on serialize value
                    //if (rb.IsTrigger() != rb.IsTrigger())
                    //    rb.object.setTriggerShape(rb.IsTrigger());
                    TRACY_PROFILE_SCOPE_END();
                }
            });

        // we initialize stuff that we can initialize!
        // for objects just created!
        static Ecs::Query meshColliderJustCreatedQuery = Ecs::make_raw_query<TransformComponent, RigidbodyComponent, ConvexColliderComponent, MeshRendererComponent, JustCreatedComponent>();
        m_world->for_each(meshColliderJustCreatedQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, ConvexColliderComponent& mc, MeshRendererComponent& mr, JustCreatedComponent& jc)
            {
                // we can confirm because anything that has convex-collider wants to use its vertices!
                if (rb.underlying_object.meshVertices.empty() || mc.Reset == true)
                {
                    //mc.Vertices.clear();
                    //mc.WorldSpaceVertices.clear();

                    if (mr.MeshInformation.mesh_handle.GetID() != oo::Asset::ID_NULL)
                    {
                        auto& vertices = mr.MeshInformation.mesh_handle.GetData<ModelFileResource*>()->vertices;

                        auto new_vertices = std::vector<physx::PxVec3>();
                        new_vertices.reserve(vertices.size());
                        std::for_each(vertices.begin(), vertices.end(), [&](auto&& vertex) {
                            new_vertices.emplace_back(physx::PxVec3{ vertex.pos.x, vertex.pos.y, vertex.pos.z });
                            });

                        // we submit our desired vertices to let the physics engine decide
                        rb.UploadVertices(new_vertices);

                        //auto generated_vertices = rb.StoreMesh(new_vertices);

                        //std::vector<oo::vec3> temp(generated_vertices.begin(), generated_vertices.end());
                        //std::vector<glm::vec3> actual(temp.begin(), temp.end());
                        //mc.WorldSpaceVertices = mc.Vertices = actual;
                    }

                    mc.Reset = false;
                }

                // set scale to current scale
                auto scale = tf.GetGlobalScale();
                rb.desired_object.meshScale = { scale.x, scale.y, scale.z };

                // this function needs to be deferred to later only once it is retrieved.
                //// update the world vertex position based on matrix of current object(this is now just for visualization purposes only!)
                //auto globalMat = tf.GetGlobalMatrix();
                //for (auto& worldSpaceVert : mc.WorldSpaceVertices)
                //{
                //    worldSpaceVert = globalMat * glm::vec4{ worldSpaceVert, 1 };
                //}
                
                // we update using the same local vertices but update its scale
                /*std::vector<oo::vec3> temp{ mc.Vertices.begin(), mc.Vertices.end() };
                std::vector<PxVec3> res{ temp.begin(), temp.end() };
                oo::vec3 scale = tf.GetGlobalScale();*/
                // TODO : find out what replaces this
                // rb.object.setConvexProperty(res, scale);

            });

        // we run update dynamics once
        TRACY_PROFILE_SCOPE_NC(start_of_frame_retrieve_physics_properties, tracy::Color::VioletRed1);
        UpdateDynamics(FixedDeltaTime);
        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_END();
    }
    
    void PhysicsSystem::RuntimeUpdate(Timestep deltaTime)
    {
        TRACY_PROFILE_SCOPE_NC(physics_update, tracy::Color::PeachPuff);

        //avoids spiral of death.
        Timestep frametime = deltaTime > MaxFrameTime ? MaxFrameTime : deltaTime;
        m_accumulator += frametime;

        while (m_accumulator >= FixedDeltaTime)
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
        
        // Submit update to physics world to reflect changes.[properties only!]
        SubmitUpdatesToPhysicsWorld();

        TRACY_PROFILE_SCOPE_NC(physics_update_internal, tracy::Color::VioletRed1);
        m_physicsWorld.updateInternal();
        TRACY_PROFILE_SCOPE_END();

        // Finally we retrieve the newly updated information from physics world 
        // and update our affected data.
        RetrieveUpdatedObjects();

        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateDynamics(Timestep deltaTime)
    {
        // Update Duplicated Objects
        UpdateDuplicatedObjects();

        // Update global bounds of all objects
        UpdateGlobalBounds();
        
        // Submit update to physics world to reflect changes.[properties only!]
        SubmitUpdatesToPhysicsWorld();

        // additionally we also submit script commands!
        SubmitScriptCommands();

        // than we tell physics engine to update!
        TRACY_PROFILE_SCOPE_NC(physX_backend_simulate, tracy::Color::Brown)
            // update the physics world using fixed dt.
            m_physicsWorld.updateScene(static_cast<float>(FixedDeltaTime));
        TRACY_PROFILE_SCOPE_END();

        // Finally we retrieve the newly updated information from physics world 
        // and update our affected data.
        RetrieveUpdatedObjects();

        /* Update transform to reflect rigidbody changes*/
        static Ecs::Query rb_query = Ecs::make_query<GameObjectComponent, TransformComponent, RigidbodyComponent>();
        // set position and orientation of our scene's dynamic rigidbodies with updated physics results
        m_world->parallel_for_each(rb_query, [&](GameObjectComponent& goc, TransformComponent& tf, RigidbodyComponent& rb)
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

            glm::quat orientation = rb.GetOrientationInPhysicsWorld();
            auto delta_orientation = orientation - tf.GetGlobalRotationQuat().value;

            //LOG_TRACE("orientation {0},{1},{2},{3}", orientation.x, orientation.y, orientation.z, orientation.w);
            //LOG_TRACE("global orientation {0},{1},{2},{3}", tf.GetGlobalRotationQuat().value.x, tf.GetGlobalRotationQuat().value.y, tf.GetGlobalRotationQuat().value.z, tf.GetGlobalRotationQuat().value.w);
            //LOG_TRACE("delta orientation {0},{1},{2},{3}", delta_orientation.x, delta_orientation.y, delta_orientation.z, delta_orientation.w);
            //LOG_TRACE("local orientation {0},{1},{2},{3}", tf.GetRotationQuat().value.x, tf.GetRotationQuat().value.y, tf.GetRotationQuat().value.z, tf.GetRotationQuat().value.w);

            tf.SetGlobalOrientation({ tf.GetGlobalRotationQuat().value + delta_orientation });
        });
        
        TRACY_PROFILE_SCOPE_NC(physics_update_transform_tree, tracy::Color::Brown)
        // lastly update transform system for transforms changes to be reflected
        m_world->Get_System<TransformSystem>()->UpdateEntireTree();
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdatePhysicsResolution(Timestep deltaTime)
    {
        UpdateCallbacks();
    }

    void PhysicsSystem::UpdateJustCreated()
    {
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

        static Ecs::Query duplicated_rb_with_box_query = Ecs::make_raw_query<RigidbodyComponent, BoxColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->parallel_for_each(duplicated_rb_with_box_query, [&](RigidbodyComponent& rbComp, BoxColliderComponent& bcComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeBoxCollider(rbComp);
            });

        static Ecs::Query duplicated_rb_with_capsule_query = Ecs::make_raw_query<RigidbodyComponent, CapsuleColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->parallel_for_each(duplicated_rb_with_capsule_query, [&](RigidbodyComponent& rbComp, CapsuleColliderComponent& ccComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeCapsuleCollider(rbComp);
            });

        static Ecs::Query duplicated_rb_with_sphere_query = Ecs::make_raw_query<RigidbodyComponent, SphereColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->parallel_for_each(duplicated_rb_with_sphere_query, [&](RigidbodyComponent& rbComp, SphereColliderComponent& scComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeSphereCollider(rbComp);
            });

        static Ecs::Query duplicated_rb_with_mesh_query = Ecs::make_raw_query<RigidbodyComponent, ConvexColliderComponent, TransformComponent, DuplicatedComponent>();
        m_world->parallel_for_each(duplicated_rb_with_mesh_query, [&](RigidbodyComponent& rbComp, ConvexColliderComponent& mcComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeMeshCollider(rbComp);
            });

        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdatedExisting()
    {
        UpdateGlobalBounds();
    }

    void PhysicsSystem::UpdateGlobalBounds()
    {
        TRACY_PROFILE_SCOPE_NC(physics_update_global_bounds, tracy::Color::PeachPuff);

        //std::mutex m;
        // Update physics World's objects position and Orientation
        static Ecs::Query rb_query = Ecs::make_query<TransformComponent, RigidbodyComponent>();
        m_world->parallel_for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
            {
                TRACY_PROFILE_SCOPE_NC(submit_update_rigidbodies, tracy::Color::PeachPuff2);
                if(tf.HasChangedThisFrame || rb.IsTrigger())
                {
                    TRACY_PROFILE_SCOPE_NC(rigidbody_set_pos_orientation, tracy::Color::PeachPuff4);
                    oo::vec3 pos = tf.GetGlobalPosition();
                    oo::quat quat = tf.GetGlobalRotationQuat();
                    //std::lock_guard lock(m);
                    rb.SetPosOrientation(pos + rb.Offset, quat);
                    TRACY_PROFILE_SCOPE_END();
                }

                {
                    TRACY_PROFILE_SCOPE_NC(rigidbody_is_trigger_check, tracy::Color::PeachPuff4);
                    // test and set trigger boolean based on serialize value
                    /*if (rb.isTrigger() != rb.IsTrigger())
                        rb.SetTriggerShape(rb.IsTrigger());
                    rb.desired_object.is_trigger;*/
                    TRACY_PROFILE_SCOPE_END();
                }

                TRACY_PROFILE_SCOPE_END();
            });

        //Updating box collider's bounds 
        static Ecs::Query boxColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, BoxColliderComponent>();
        m_world->parallel_for_each(boxColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, BoxColliderComponent& bc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate global bounds and half extents
                bc.GlobalHalfExtents = { bc.HalfExtents * bc.Size * scale };

                // set box size
                rb.desired_object.box = { bc.GlobalHalfExtents.x, bc.GlobalHalfExtents.y, bc.GlobalHalfExtents.z };

                // update filtering
                {
                    TRACY_PROFILE_SCOPE_NC(box_update_filters, tracy::Color::PeachPuff4);
                    rb.desired_object.filterIn = static_cast<std::uint32_t>(rb.InputLayer.to_ulong());
                    rb.desired_object.filterOut = static_cast<std::uint32_t>(rb.OutputLayer.to_ulong());
                    TRACY_PROFILE_SCOPE_END();
                }
            });

        //Updating capsule collider's bounds 
        static Ecs::Query capsuleColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, CapsuleColliderComponent>();
        m_world->parallel_for_each(capsuleColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, CapsuleColliderComponent& cc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate global radius and half height for capsule collider
                cc.GlobalRadius = cc.Radius * scale.y;          // for now lets just use y-axis
                cc.GlobalHalfHeight = cc.HalfHeight * scale.y;  // for now lets just use y axis

                // set capsule size
                rb.desired_object.capsule = { cc.GlobalRadius, cc.GlobalHalfHeight };

                // update filtering
                {
                    TRACY_PROFILE_SCOPE_NC(box_update_filters, tracy::Color::PeachPuff4);
                    rb.desired_object.filterIn = static_cast<std::uint32_t>(rb.InputLayer.to_ulong());
                    rb.desired_object.filterOut = static_cast<std::uint32_t>(rb.OutputLayer.to_ulong());
                    TRACY_PROFILE_SCOPE_END();
                }
            });

        //Updating sphere collider's bounds 
        static Ecs::Query sphereColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, SphereColliderComponent>();
        m_world->parallel_for_each(sphereColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, SphereColliderComponent& sc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate global bounds of capsule
                //sc.GlobalBounds.center = sc.Bounds.center + pos;
                sc.GlobalRadius= sc.Radius * std::max(std::max(scale.x, scale.y), scale.z);

                // set capsule size
                rb.desired_object.sphere = { sc.GlobalRadius };

                // update filtering
                {
                    TRACY_PROFILE_SCOPE_NC(box_update_filters, tracy::Color::PeachPuff4);
                    rb.desired_object.filterIn = static_cast<std::uint32_t>(rb.InputLayer.to_ulong());
                    rb.desired_object.filterOut = static_cast<std::uint32_t>(rb.OutputLayer.to_ulong());
                    TRACY_PROFILE_SCOPE_END();
                }
            });

        {
            TRACY_PROFILE_SCOPE_NC(physics_update_mesh_collider_bounds, tracy::Color::PeachPuff);
            //Updating mesh collider's bounds 
            static Ecs::Query meshColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, ConvexColliderComponent, MeshRendererComponent>();
            m_world->for_each(meshColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, ConvexColliderComponent& mc, MeshRendererComponent& mr)
                {
                    //bool justEdited = false;

                    if (mc.Reset == true)
                    {
                        //mc.Vertices.clear();
                        mc.WorldSpaceVertices.clear();
                        

                        if (mr.MeshInformation.mesh_handle.GetID() != oo::Asset::ID_NULL)
                        {
                            auto const vertices = mr.MeshInformation.mesh_handle.GetData<ModelFileResource*>()->vertices;

                            auto new_vertices = std::vector<physx::PxVec3>();
                            new_vertices.reserve(vertices.size());
                            std::for_each(vertices.begin(), vertices.end(), [&](auto&& vertex) {
                                new_vertices.emplace_back(physx::PxVec3{ vertex.pos.x, vertex.pos.y, vertex.pos.z });
                                });

                            //auto generated_vertices = rb.StoreMesh(new_vertices);

                            // we submit our desired vertices to let the physics engine decide
                            rb.UploadVertices(new_vertices);

                            /*std::vector<oo::vec3> temp{ generated_vertices.begin(), generated_vertices.end() };
                            std::vector<glm::vec3> final_result{temp.begin(), temp.end() };
                            mc.WorldSpaceVertices = mc.Vertices = final_result;*/
                        }

                        mc.Reset = false;
                        //justEdited = true;
                    }
                    
                    if (tf.HasChangedThisFrame)
                    {
                        // set scale to current scale
                        auto scale = tf.GetGlobalScale();
                        rb.desired_object.meshScale = { scale.x, scale.y, scale.z };
                    }

                    if (rb.underlying_object.changeVertices)
                    {
                        auto gen_vertices = rb.underlying_object.meshVertices;
                        std::vector<oo::vec3> temp{ gen_vertices.begin(), gen_vertices.end() };
                        std::vector<glm::vec3> final_result{ temp.begin(), temp.end() };
                        mc.WorldSpaceVertices = final_result;
                        auto vertices = final_result.begin();
                        auto globalMat = tf.GetGlobalMatrix();
                        std::for_each(std::execution::par_unseq, mc.WorldSpaceVertices.begin(), mc.WorldSpaceVertices.end(), [&](auto&& v)
                            {
                                v = globalMat * glm::vec4{ static_cast<glm::vec3>(*vertices++), 1 };
                            });
                    }

                    //if (tf.HasChangedThisFrame || (justEdited && !mc.Vertices.empty()))
                    //{
                    //    // update the world vertex position based on matrix of current object
                    //    auto globalMat = tf.GetGlobalMatrix();
                    //    auto vertices = mc.Vertices.begin();
                    //    std::for_each(mc.WorldSpaceVertices.begin(), mc.WorldSpaceVertices.end(), [&](auto&& v)
                    //        {
                    //            v = globalMat * glm::vec4{ static_cast<glm::vec3>(*vertices++), 1 };
                    //        });
                    //    std::vector<oo::vec3>temp{ mc.Vertices.begin(), mc.Vertices.end() };
                    //    std::vector<PxVec3> res{ temp.begin(), temp.end() }; 
                    //    oo::vec3 scale = tf.GetGlobalScale();
                    //    //rb.object.setConvexProperty(res, scale);
                    //}

                    // update filtering
                    {
                        TRACY_PROFILE_SCOPE_NC(box_update_filters, tracy::Color::PeachPuff4);
                        rb.desired_object.filterIn = static_cast<std::uint32_t>(rb.InputLayer.to_ulong());
                        rb.desired_object.filterOut = static_cast<std::uint32_t>(rb.OutputLayer.to_ulong());
                        TRACY_PROFILE_SCOPE_END();
                    }
                });


            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::SubmitUpdatesToPhysicsWorld()
    {
        TRACY_PROFILE_SCOPE_NC(physics_submit_updates_to_physX_world, tracy::Color::VioletRed1);

        {
            TRACY_PROFILE_SCOPE_NC(compiling_objects_that_requires_update, tracy::Color::VioletRed2);

            needsUpdating.clear();
            needsUpdating.reserve(1024);
            static Ecs::Query rb_query = Ecs::make_query<TransformComponent, RigidbodyComponent>();
            m_world->for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
                {
                    if (rb.IsDirty)
                        needsUpdating.emplace_back(rb.desired_object);

                    rb.IsDirty = false;
                });
            TRACY_PROFILE_SCOPE_END();
        }

        {
            TRACY_PROFILE_SCOPE_NC(physX_internal_updating, tracy::Color::VioletRed2);
            // submit to update
            m_physicsWorld.submitUpdatedObjects(std::move(needsUpdating));
            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void oo::PhysicsSystem::SubmitScriptCommands()
    {
        TRACY_PROFILE_SCOPE_NC(submit_script_commands, tracy::Color::Brown)
        {
            std::vector<myPhysx::PhysicsCommand> all_commands;
            static Ecs::Query rb_query = Ecs::make_query<TransformComponent, RigidbodyComponent>();
            m_world->for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
                {
                    std::move(rb.external_commands.begin(), rb.external_commands.end(), all_commands.end());
                });
            m_physicsWorld.submitPhysicsCommand(std::move(all_commands));
        }
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::RetrieveUpdatedObjects()
    {
        TRACY_PROFILE_SCOPE_NC(physics_retrieve_updated_objects, tracy::Color::Brown)
            auto const& updatedPhysicsObjects = m_physicsWorld.retrieveCurrentObjects();
        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_NC(physics_update_physics_properties, tracy::Color::Brown)
            static Ecs::Query rb_query = Ecs::make_query<TransformComponent, RigidbodyComponent>();
        m_world->parallel_for_each(rb_query, [&](TransformComponent& tf, RigidbodyComponent& rb)
            {
                rb.desired_object = rb.underlying_object = updatedPhysicsObjects.at(rb.underlying_object.id);
            });
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateCallbacks()
    {
        TRACY_PROFILE_SCOPE_NC(physics_trigger_resoltion, tracy::Color::PeachPuff);
        auto trigger_queue = m_physicsWorld.getTriggerData();
        PhysicsTriggersEvent ptse;
        ptse.TriggerEvents.reserve(trigger_queue->size());
        while (!trigger_queue->empty())
        {
            myPhysx::TriggerManifold trigger_manifold = trigger_queue->front();

            // if either objects are already removed, we skip them
            if (m_physicsToGameObjectLookup.contains(trigger_manifold.triggerID) == false
                || m_physicsToGameObjectLookup.contains(trigger_manifold.otherID) == false)
            {
                if (DebugMessages)
                    LOG_WARN("Skipped Trigger Callback because one of these pair of (Physics)UUID died ({0}), ({1}) ", trigger_manifold.triggerID, trigger_manifold.otherID);

                trigger_queue->pop();
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
            case myPhysx::trigger::none:
                pte.State = PhysicsEventState::NONE;
                break;
            case myPhysx::trigger::onTriggerEnter:
                if(DebugMessages)
                    LOG_TRACE("Trigger Enter Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::ENTER;
                break;
            case myPhysx::trigger::onTriggerStay:
                if (DebugMessages)
                    LOG_TRACE("Trigger Stay Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::STAY;
                break;
            case myPhysx::trigger::onTriggerExit:
                if (DebugMessages) 
                    LOG_TRACE("Trigger Exit Event! Trigger Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pte.TriggerID)->Name(), m_scene->FindWithInstanceID(pte.OtherID)->Name());
                pte.State = PhysicsEventState::EXIT;
                break;
            }

            ptse.TriggerEvents.emplace_back(pte);

            /*TRACY_PROFILE_SCOPE_NC(broadcast_physics_trigger_event, tracy::Color::PeachPuff);
            EventManager::Broadcast(&pte);
            TRACY_PROFILE_SCOPE_END();*/

            trigger_queue->pop();
        }
        m_physicsWorld.clearTriggerData();


        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_NC(physics_collision_resoltion, tracy::Color::PeachPuff);
        
        //jobsystem::job collision_resolution;

        auto collision_queue = m_physicsWorld.getCollisionData();
        PhysicsCollisionsEvent pcse;
        pcse.CollisionEvents.reserve(collision_queue->size());
        while (!collision_queue->empty())
        {
            myPhysx::ContactManifold contact_manifold = collision_queue->front();

            // if either objects are already removed, we skip them
            if (m_physicsToGameObjectLookup.contains(contact_manifold.shape1_ID) == false
                || m_physicsToGameObjectLookup.contains(contact_manifold.shape2_ID) == false)
            {
                if (DebugMessages)
                    LOG_WARN("Skipped Physics Collision Callback because one of these pair of (Physics)UUID died ({0}), ({1}) ", contact_manifold.shape1_ID, contact_manifold.shape2_ID);

                collision_queue->pop();
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
            case myPhysx::collision::none:
                pce.State = PhysicsEventState::NONE;
                break;
            case myPhysx::collision::onCollisionEnter:
                if (DebugMessages)LOG_TRACE("Collision Enter Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::ENTER;
                break;
            case myPhysx::collision::onCollisionStay:
                if(DebugMessages) LOG_TRACE("Collision Stay Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::STAY;
                break;
            case myPhysx::collision::onCollisionExit:
                if (DebugMessages) LOG_TRACE("Collision Exit Event! Collider Name \"{0}\", Other Name \"{1}\"", m_scene->FindWithInstanceID(pce.Collider1)->Name(), m_scene->FindWithInstanceID(pce.Collider2)->Name());
                pce.State = PhysicsEventState::EXIT;
                break;
            }
            
            pcse.CollisionEvents.emplace_back(pce);

            /*jobsystem::submit(collision_resolution, [&]()
            {*/
                /*TRACY_PROFILE_SCOPE_NC(broadcast_physics_collision_event, tracy::Color::PeachPuff);
                EventManager::Broadcast(&pce);
                TRACY_PROFILE_SCOPE_END();*/
            //});

            collision_queue->pop();
        }
        m_physicsWorld.clearCollisionData();


        // resolve all events at the end
        {
            TRACY_PROFILE_SCOPE_NC(broadcast_physics_triggers_bulk_event, tracy::Color::PeachPuff);
            EventManager::Broadcast(&ptse);
            TRACY_PROFILE_SCOPE_END();
        }
        {
            TRACY_PROFILE_SCOPE_NC(broadcast_physics_collisions_bulk_event, tracy::Color::PeachPuff);
            EventManager::Broadcast(&pcse);
            TRACY_PROFILE_SCOPE_END();
        }
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
            glm::vec3 pos = rb.GetPositionInPhysicsWorld();
            glm::quat quat = rb.GetOrientationInPhysicsWorld();

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
            oGFX::DebugDraw::AddLine(bottom_left_back, bottom_left_front, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(bottom_left_front, bottom_right_front, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(bottom_right_front, bottom_right_back, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(bottom_right_back, bottom_left_back, oGFX::Colors::GREEN);

            oGFX::DebugDraw::AddLine(top_left_back, top_left_front, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(top_left_front, top_left_front, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(top_right_front, top_right_back, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(top_right_back, top_left_back, oGFX::Colors::GREEN);

            oGFX::DebugDraw::AddLine(bottom_left_back, top_left_back, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(bottom_left_front, top_left_front, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(bottom_right_front, top_right_front, oGFX::Colors::GREEN);
            oGFX::DebugDraw::AddLine(bottom_right_back, top_right_back, oGFX::Colors::GREEN);

            //oGFX::DebugDraw::AddAABB({ pos + bc.GlobalHalfExtents  , pos - bc.GlobalHalfExtents }, oGFX::Colors::GREEN);
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
            oGFX::DebugDraw::AddAABB({ pos - GlobalHalfExtents  , pos + GlobalHalfExtents }, oGFX::Colors::GREEN);
            // draw top sphere
            oGFX::DebugDraw::AddSphere({ pos + vec3{ 0, GlobalHalfExtents.y, 0}, cc.GlobalRadius }, oGFX::Colors::GREEN);
            // draw bottom sphere
            oGFX::DebugDraw::AddSphere({ pos - vec3{ 0, GlobalHalfExtents.y, 0}, cc.GlobalRadius }, oGFX::Colors::GREEN);
        });

        //Updating capsule collider's bounds and debug drawing
        static Ecs::Query sphereColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, SphereColliderComponent>();
        m_world->for_each(sphereColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, SphereColliderComponent& sc)
            {
                auto pos = rb.GetPositionInPhysicsWorld();
                auto scale = tf.GetGlobalScale();
                auto quat = rb.GetOrientationInPhysicsWorld();

                // debug Draw the sphere collider
                oGFX::DebugDraw::AddSphere({ pos, sc.GlobalRadius }, oGFX::Colors::GREEN);
            });

        static Ecs::Query meshColliderQuery = Ecs::make_query<TransformComponent, RigidbodyComponent, ConvexColliderComponent>();
        m_world->for_each(meshColliderQuery, [&](TransformComponent& tf, RigidbodyComponent& rb, ConvexColliderComponent& mc)
            {
                /*
                * TODO : FIGURE OUT WHY DAHELL THERES A DIFFERENCE, TOP DOESNT CRASH, BOTTOM DOES!
                for (auto& i : mc.WorldSpaceVertices)
                {
                }

                for (auto iter = mc.WorldSpaceVertices.cbegin(); iter != mc.WorldSpaceVertices.cend(); ++iter)
                {
                }*/

                if (mc.WorldSpaceVertices.size() > 1)
                {
                    auto first = mc.WorldSpaceVertices.back(); 
                    auto next = mc.WorldSpaceVertices.front();
                    oGFX::DebugDraw::AddLine(first, next, oGFX::Colors::GREEN);

                    for (auto curr : mc.WorldSpaceVertices)
                    {
                        first = next;
                        next = curr;
                        oGFX::DebugDraw::AddLine(first, next, oGFX::Colors::GREEN);
                    }

                    //auto first = mc.WorldSpaceVertices.back();
                    //auto next = mc.WorldSpaceVertices.front();
                    
                    //for (auto iter = std::next(mc.WorldSpaceVertices.begin()); iter != mc.WorldSpaceVertices.end(); ++iter)
                    //{
                    //    //DebugDraw::AddLine(first, next, oGFX::Colors::GREEN);
                    //    
                    //    //auto vert = *iter;
                    //    //first = next;
                    //    //next = vert;
                    //}
                    //DebugDraw::AddLine(first, next, oGFX::Colors::GREEN);
                }
            });

        TRACY_PROFILE_SCOPE_END();
    }

    LayerType PhysicsSystem::GenerateCollisionMask(std::vector<std::string> names)
    {
        /*static std::vector<std::pair<util::StringHash, unsigned int>> hashedTable; 
        if (hashedTable.size() == 0)
        {
            unsigned int count = 0;
            for (auto& layer : LayerNames)
            {
                hashedTable.emplace_back(std::pair{ util::StringHash::GenerateFNV1aHash(layer), count++ });
            }
        }*/

        LayerType result = 0;
        unsigned int count = 0;
        std::for_each(LayerNames.cbegin(), LayerNames.cend(),
            [&](auto&& elem) 
            {
                //auto key = util::StringHash::GenerateFNV1aHash(elem);
                //auto value = std::find_if(hashedTable.cbegin(), hashedTable.cend(), [&](auto&& elem) { return elem.second == key; });
                //if (value != hashedTable.cend())
                auto value = std::find(names.cbegin(), names.cend(), elem);
                if(value != names.cend())
                    result |= (1 << count);

                count++;
            });

        if(result == 0)
            LOG_WARN("mask generated won't work against anything");

        return result;
    }

    void PhysicsSystem::SetFixedDeltaTimescale(Timestep NewFixedTime)
    { 
        FixedDeltaTimescale = NewFixedTime;
        FixedDeltaTime = FixedDeltaTimeBase * FixedDeltaTimescale;
        MaxFrameTime = MaxFrameRateMultiplier * FixedDeltaTime;

        //FixedDeltaTime = NewFixedTime; 
        //auto newLimit = FixedDeltaTime * MaxIterations;
        //AccumulatorLimit =  AccumulatorLimit < newLimit ? newLimit : AccumulatorLimit;
    }

    PhysicsSystem::Timestep PhysicsSystem::GetFixedDeltaTimescale()
    {
        return FixedDeltaTimescale;
    }

    PhysicsSystem::Timestep PhysicsSystem::GetFixedDeltaTime()
    {
        return FixedDeltaTime; 
    }

    RaycastResult PhysicsSystem::Raycast(Ray ray, float distance, LayerType collisionFilter)
    {
        TRACY_PROFILE_SCOPE_NC(physics_raycast, tracy::Color::PeachPuff4);

        auto result = m_physicsWorld.raycast({ ray.Position.x, ray.Position.y, ray.Position.z }, { ray.Direction.x, ray.Direction.y, ray.Direction.z }, distance
            , static_cast<std::uint32_t>(collisionFilter));
        
        ASSERT_MSG(result.intersect && m_physicsToGameObjectLookup.contains(result.object_ID) == false, "Why am i hitting something that's not in the current world?");
        
        TRACY_PROFILE_SCOPE_END();

        return { result.intersect, m_physicsToGameObjectLookup.at(result.object_ID), {result.position.x,result.position.y, result.position.z}, 
            { result.normal.x, result.normal.y, result.normal.z }, result.distance };
    }

    std::vector<RaycastResult> PhysicsSystem::RaycastAll(Ray ray, float distance, LayerType collisionFilter)
    {
        TRACY_PROFILE_SCOPE_NC(physics_raycast_all, tracy::Color::PeachPuff4);

        std::vector<RaycastResult> result;
        
        // normalize our ray just to make sure
        ray.Direction = glm::normalize(ray.Direction);

        auto allHits = m_physicsWorld.raycastAll({ ray.Position.x, ray.Position.y, ray.Position.z }, { ray.Direction.x, ray.Direction.y, ray.Direction.z }, distance
            , static_cast<std::uint32_t>(collisionFilter));

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

        TRACY_PROFILE_SCOPE_END();

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
        if (m_world->has_component<ConvexColliderComponent>(rb->entityID))
            m_world->remove_component<ConvexColliderComponent>(rb->entityID);

        // IMPT!
        auto& obj = m_world->get_component<RigidbodyComponent>(rb->entityID);

        // finally we remove the physics object
        m_physicsWorld.removeInstance(obj.underlying_object);

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
            //rb_comp.object.removeShape();
            rb_comp.desired_object.shape_type = myPhysx::shape::none;

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
            //rb_comp.object.removeShape();
            rb_comp.desired_object.shape_type = myPhysx::shape::none;
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
            //rb_comp.object.removeShape();
            rb_comp.desired_object.shape_type = myPhysx::shape::none;
            //rb_comp.object.setShape(myPhysx::shape::none);
        }
    }

    void PhysicsSystem::OnMeshColliderAdd(Ecs::ComponentEvent<ConvexColliderComponent>* mc)
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

    void PhysicsSystem::OnMeshColliderRemove(Ecs::ComponentEvent<ConvexColliderComponent>* mc)
    {
        // need this safeguard to be sure. otherwise crash.
        if (m_world->has_component<RigidbodyComponent>(mc->entityID))
        {
            auto& rb_comp = m_world->get_component<RigidbodyComponent>(mc->entityID);
            //rb_comp.object.removeShape();
            rb_comp.desired_object.shape_type = myPhysx::shape::none;
            //rb_comp.object.setShape(myPhysx::shape::none);
        }
    }

    void PhysicsSystem::InitializeRigidbody(RigidbodyComponent& rb)
    {
        //rb.object = m_physicsWorld.createInstance();
        myPhysx::PhysicsObject instance = m_physicsWorld.createInstance();
        rb.desired_object = instance;
        rb.underlying_object = instance;

        rb.SetStatic(true); // default to static objects. Most things in the world should be static.
        //rb.EnableGravity(); // most things in the world should have gravity enabled (?)
        //default initialize material
        //rb.object.setMaterial(PhysicsMaterial{});
    }

    void PhysicsSystem::InitializeBoxCollider(RigidbodyComponent& rb)
    {
        // create box
        //rb.object.setShape(myPhysx::shape::box);

        rb.desired_object.shape_type = myPhysx::shape::box;
    }

    void PhysicsSystem::InitializeCapsuleCollider(RigidbodyComponent& rb)
    {
        // create capsule
        //rb.object.setShape(myPhysx::shape::capsule);
        rb.desired_object.shape_type = myPhysx::shape::capsule;
    }

    void PhysicsSystem::InitializeSphereCollider(RigidbodyComponent& rb)
    {
        // create sphere
        //rb.object.setShape(myPhysx::shape::sphere);
        rb.desired_object.shape_type = myPhysx::shape::sphere;
    }

    void PhysicsSystem::InitializeMeshCollider(RigidbodyComponent& rb)
    {
        // create mesh collider
        //rb.object.setShape(myPhysx::shape::convex);
        rb.desired_object.shape_type = myPhysx::shape::convex;
    }

    void PhysicsSystem::DuplicateRigidbody(RigidbodyComponent& rb)
    {
        // we duplicate instead if this is an existing object
        /*if (m_physicsWorld.hasObject(rb.object.id))
            rb.object = m_physicsWorld.duplicateObject(rb.object.id);*/
        if (m_physicsWorld.hasObject(rb.underlying_object.id))
            rb.underlying_object = rb.desired_object = m_physicsWorld.duplicateObject(rb.underlying_object.id);
    }

    void PhysicsSystem::AddToLookUp(RigidbodyComponent& rb, GameObjectComponent& goc)
    {
        /*if (m_physicsToGameObjectLookup.contains(rb.object.id) == false)
        {
            m_physicsToGameObjectLookup.insert({ rb.object.id, goc.Id });
        }*/
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
