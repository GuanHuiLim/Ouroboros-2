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

#include "Ouroboros/Physics/PhysicsComponent.h"
#include "Ouroboros/Physics/RigidbodyComponent.h"
#include "Ouroboros/Physics/ColliderComponents.h"

#include "Ouroboros/ECS/DeferredComponent.h"

#include "Ouroboros/Transform/TransformComponent.h"

#include "OO_Vulkan/src/DebugDraw.h"
namespace oo
{
    PhysicsSystem::PhysicsSystem()
        : m_accumulator{0}
        , Gravity { 0, -0.0981f, 0 }
        , m_physicsWorld{ PxVec3{Gravity.x, Gravity.y, Gravity.z} }
    {
        
    }

    void PhysicsSystem::Init()
    {
        m_world->SubscribeOnAddComponent<PhysicsSystem, RigidbodyComponent>(
            this, &PhysicsSystem::OnRigidbodyAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, RigidbodyComponent>(
            this, &PhysicsSystem::OnRigidbodyRemove);
        
        m_world->SubscribeOnAddComponent<PhysicsSystem, BoxColliderComponent>(
            this, &PhysicsSystem::OnBoxColliderAdd);

        m_world->SubscribeOnRemoveComponent<PhysicsSystem, BoxColliderComponent>(
            this, &PhysicsSystem::OnBoxColliderRemove);
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

        static Ecs::Query rb_query = []()
        {
            Ecs::Query query;
            query.with<TransformComponent, PhysicsComponent, RigidbodyComponent>().exclude<DeferredComponent>().build();
            return query;
        }();

        m_world->for_each(rb_query, [&](TransformComponent& tf, PhysicsComponent& phy, RigidbodyComponent& rb)
        {
            auto pos = tf.GetGlobalPosition();
            //phy.object.setposition({ pos.x, pos.y, pos.z });
            
            auto quat = tf.GetGlobalRotationQuat();
            //phy.object.setOrientation({quat.value.x, quat.value.y, quat.value.z, quat.value.w});
            
            phy.object.setPosOrientation( { pos.x, pos.y, pos.z }, { quat.value.w, quat.value.x, quat.value.y, quat.value.z  } );
        });

        // TODO: This should be temporary soln
        //Updating Drawing of static Box Collider's Bounds
        static Ecs::Query boxColliderDrawQuery = []()
        {
            Ecs::Query query;
            query.with<TransformComponent, PhysicsComponent, BoxColliderComponent>().exclude<DeferredComponent>().build();
            return query;
        }();

        m_world->for_each(boxColliderDrawQuery, [&](TransformComponent& tf, PhysicsComponent& phy, BoxColliderComponent& bc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate local scale
                bc.Bounds.min = (bc.Size * -0.5f) * scale;
                bc.Bounds.max = (bc.Size * 0.5f) * scale;

                // calculate global bounds and half extents
                bc.GlobalBounds = { bc.Bounds.min , bc.Bounds.max };
                auto halfExtents = (bc.GlobalBounds.max - bc.GlobalBounds.min) * 0.5f;
                auto globalPos = pos + bc.Offset;

                //TODO: Debug draw the bounds
                DebugDraw::AddAABB({ globalPos + bc.Bounds.min , globalPos + bc.Bounds.max }, oGFX::Colors::GREEN);
            });

        //Updating Static Box Collider's Bounds
        static Ecs::Query boxColliderQuery = []()
        {
            Ecs::Query query;
            query.with<TransformComponent, PhysicsComponent, BoxColliderComponent>().exclude<DeferredComponent, RigidbodyComponent>().build();
            return query;
        }();

        m_world->for_each(boxColliderQuery, [&](TransformComponent& tf, PhysicsComponent& phy, BoxColliderComponent& bc)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate local scale
                bc.Bounds.min = (bc.Size * -0.5f) * scale;
                bc.Bounds.max = (bc.Size * 0.5f) * scale;

                // calculate global bounds and half extents
                bc.GlobalBounds = { bc.Bounds.min , bc.Bounds.max };
                auto halfExtents = (bc.GlobalBounds.max - bc.GlobalBounds.min) * 0.5f;
                auto globalPos = pos + bc.Offset;
                // set box size
                phy.object.setBoxProperty(halfExtents.x, halfExtents.y, halfExtents.z);
                
                // set box size
                //phy.object.setposition({ globalPos.x, globalPos.y, globalPos.z });
                phy.object.setPosOrientation({ globalPos.x, globalPos.y, globalPos.z }, { quat.value.w, quat.value.x, quat.value.y, quat.value.z });
                
            });

        // Update global bounds of all objects
        //UpdateGlobalBounds();
        TRACY_PROFILE_SCOPE_END();
    }

    void PhysicsSystem::UpdateDynamics(Timestep deltaTime)
    {
        //TODO: Should remove eventually
        EditorUpdate(deltaTime);
        
        // update the physics world using fixed dt.
        m_physicsWorld.updateScene(FixedDeltaTime);

        static Ecs::Query rb_query = []()
        {
            Ecs::Query query;
            query.with<TransformComponent, PhysicsComponent, RigidbodyComponent>().exclude<DeferredComponent>().build();
            return query;
        }();
        
        // set position and orientation
        m_world->for_each(rb_query, [&](TransformComponent& tf, PhysicsComponent& phy, RigidbodyComponent& rb)
        {
            auto pos = phy.object.getposition();
            glm::vec3 new_pos{ pos.x, pos.y, pos.z };
            tf.SetGlobalPosition(new_pos);

            auto orientation = phy.object.getOrientation();
            tf.SetGlobalOrientation({ orientation.x, orientation.y, orientation.z, orientation.w });
        });

        static Ecs::Query dynamicBoxColliderQuery = []()
        {
            Ecs::Query query;
            query.with<TransformComponent, PhysicsComponent, BoxColliderComponent, RigidbodyComponent>().exclude<DeferredComponent>().build();
            return query;
        }();

        //Updating Dynamic Box Collider Bounds
        m_world->for_each(dynamicBoxColliderQuery, [&](TransformComponent& tf, PhysicsComponent& phy, BoxColliderComponent& bc, RigidbodyComponent& rb)
            {
                auto pos = tf.GetGlobalPosition();
                auto scale = tf.GetGlobalScale();
                auto quat = tf.GetGlobalRotationQuat();

                // calculate local scale
                bc.Bounds.min = (bc.Size * -0.5f) * scale;
                bc.Bounds.max = (bc.Size * 0.5f) * scale;

                // calculate global bounds and half extents
                bc.GlobalBounds = { bc.Bounds.min , bc.Bounds.max };
                auto halfExtents = (bc.GlobalBounds.max - bc.GlobalBounds.min) * 0.5f;
                auto globalPos = pos + bc.Offset;
                // set box size
                phy.object.setBoxProperty(halfExtents.x, halfExtents.y, halfExtents.z);

                //phy.object.setposition({ globalPos.x, globalPos.y, globalPos.z });
                phy.object.setPosOrientation({ globalPos.x, globalPos.y, globalPos.z }, { quat.value.w, quat.value.x, quat.value.y, quat.value.z });


                //TODO : Toggle to enable/disable debug drawing of bounds.
                //TODO : Debug draw the bounds

                DebugDraw::AddAABB({ globalPos + bc.Bounds.min , globalPos + bc.Bounds.max }, oGFX::Colors::GREEN);
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

    void PhysicsSystem::OnRigidbodyAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        // set rigid to be default dynamic if its the first component added.
        if (m_world->has_component<PhysicsComponent>(rb->entityID) == false) 
        {
            m_world->add_component<PhysicsComponent>(rb->entityID);

            auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
            phy_comp.object = m_physicsWorld.createInstance();
        }

        auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
        // if box collider exist on item already.
        phy_comp.object.setRigidType(rigid::rdynamic);

    }

    void PhysicsSystem::OnRigidbodyRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb)
    {
        m_physicsWorld.removeInstance(m_world->get_component<PhysicsComponent>(rb->entityID).object);
        
        // if this is the last component, we remove the physics component as well.
        if (m_world->has_component<BoxColliderComponent>(rb->entityID) == false
            && m_world->has_component<SphereColliderComponent>(rb->entityID) == false)
        {
            m_world->remove_component<PhysicsComponent>(rb->entityID);
        }
    }

    void PhysicsSystem::OnBoxColliderAdd(Ecs::ComponentEvent<BoxColliderComponent>* rb)
    {
        // if this is the first component to be added
        if (m_world->has_component<PhysicsComponent>(rb->entityID) == false) 
        {
            m_world->add_component<PhysicsComponent>(rb->entityID);

            auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
            phy_comp.object = m_physicsWorld.createInstance();

            phy_comp.object.setRigidType(rigid::rstatic);   // static if this is first added.

            Material mat{}; //default initialize
            mat.restitution = 0.f;
            mat.staticFriction = 1.f;
            mat.dynamicFriction = 1.f;
            phy_comp.object.setMaterial(mat);

            // create box temporarily
            phy_comp.object.setShape(shape::box);
        }
        // if we do have box collider Component
        else if (m_world->has_component<SphereColliderComponent>(rb->entityID) == true)
        {
            // just need to switch shape.
            auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
            phy_comp.object.setShape(shape::box);
        }
        else
        {
            // if you have a rigidbody already or finished adding physics comp
            auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
            
            Material mat{}; //default initialize

            mat.restitution = 0.f;
            mat.staticFriction = 1.f;
            mat.dynamicFriction = 1.f;

            phy_comp.object.setMaterial(mat);

            // create box temporarily
            phy_comp.object.setShape(shape::box);
            //phy_comp.object.setBoxProperty();
        }
    }

    void PhysicsSystem::OnBoxColliderRemove(Ecs::ComponentEvent<BoxColliderComponent>* rb)
    {
        if (m_world->has_component<RigidbodyComponent>(rb->entityID) == false
            && m_world->has_component<SphereColliderComponent>(rb->entityID) == false)
        {
            m_world->remove_component<PhysicsComponent>(rb->entityID);
        }
    }

    void PhysicsSystem::OnSphereColliderAdd(Ecs::ComponentEvent<SphereColliderComponent>* rb)
    {
        // if this is the first component to be added

        if (m_world->has_component<PhysicsComponent>(rb->entityID) == false)
        {
            auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
            m_world->add_component<PhysicsComponent>(rb->entityID);

            phy_comp.object = m_physicsWorld.createInstance();

            //default initialize material
            Material mat{};
            mat.restitution = 0.f;
            mat.staticFriction = 1.f;
            mat.dynamicFriction = 1.f;
            phy_comp.object.setMaterial(mat);

            phy_comp.object.setRigidType(rigid::rstatic);   // static if this is first added.
        }
        // if we do have box collider Component
        else if (m_world->has_component<BoxColliderComponent>(rb->entityID) == true)
        {
            // just need to switch shape.
            auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
            phy_comp.object.setShape(shape::sphere);
        }
        // we have a rigid body component only
        else
        {
            auto& phy_comp = m_world->get_component<PhysicsComponent>(rb->entityID);
            
            // we need to set a default material for 
            Material mat{};
            mat.restitution = 0.f;
            mat.staticFriction = 1.f;
            mat.dynamicFriction = 1.f;
            phy_comp.object.setMaterial(mat);

            // create box temporarily
            phy_comp.object.setShape(shape::sphere);
            //phy_comp.object.setBoxProperty();
        }
    }

    void PhysicsSystem::OnSphereColliderRemove(Ecs::ComponentEvent<SphereColliderComponent>* rb)
    {
        // no rigid nor box
        if (m_world->has_component<RigidbodyComponent>(rb->entityID) == false
            && m_world->has_component<BoxColliderComponent>(rb->entityID) == false)
        {
            m_world->remove_component<PhysicsComponent>(rb->entityID);
        }
    }

}
