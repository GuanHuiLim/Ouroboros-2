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

#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include "Ouroboros/Physics/PhysicsFwd.h"
#include "PhysicsEvents.h"
#include "Ouroboros/Core/Timer.h"
#include <Physics/Source/phy.h>
#include <bitset>
#include "Ouroboros/ECS/GameObjectComponent.h"
#include "Ouroboros/Scene/Scene.h"

#include "Ouroboros/Geometry/Shapes.h"
#include "Ouroboros/Physics/Raycast.h"

namespace oo
{
    class Scene;

    class PhysicsSystem final : public Ecs::System
    {
    public:
        using Timestep = double;

        PhysicsSystem() = default;
        virtual ~PhysicsSystem();
        virtual void Run(Ecs::ECSWorld*) override {};

        void Init(Scene* m_scene);
        void PostLoadSceneInit();
        void RuntimeUpdate(Timestep deltaTime);
        void EditorUpdate(Timestep deltaTime);
        void RenderDebugColliders();

        // Global gravity
        vec3 Gravity = { 0, -9.81f, 0 };
        
        inline static bool ColliderDebugDraw = true;
        inline static bool DebugMessages = false;

        // Layers  related functions
        inline static std::vector<std::string> LayerNames = { "Default", "Environment","Player","Enemy","Layer Five","Layer Six","Layer Seven","Layer Eight" };
        
        static LayerType GenerateCollisionMask(std::vector<std::string> names);

        // Manupilating Fixed DT
        static void SetFixedDeltaTimescale(Timestep NewMultiplier);
        static Timestep GetFixedDeltaTimescale();
        static Timestep GetFixedDeltaTime();
        
        RaycastResult Raycast(Ray ray, float distance = std::numeric_limits<float>::max(), LayerType collisionFilter = std::numeric_limits<LayerType>::max());
        std::vector<RaycastResult> RaycastAll(Ray ray , float distance = std::numeric_limits<float>::max(), LayerType collisionFilter = std::numeric_limits<LayerType>::max());
    
    private:
        //inline static std::uint64_t MaxIterations = 2;
        inline static Timestep FixedDeltaTimeBase = 1.0/60.0;               // physics updates at 60 fps
        inline static Timestep FixedDeltaTimescale = 1.0;                  // additional level of control for scripts
        inline static Timestep FixedDeltaTime = FixedDeltaTimeBase * FixedDeltaTimescale;  // physics updates at 60 fps
        inline static Timestep MaxFrameRateMultiplier = 2;
        inline static Timestep MaxFrameTime = FixedDeltaTime * MaxFrameRateMultiplier;
        //inline static Timestep AccumulatorLimit = FixedDeltaTime * MaxIterations;   // To prevent spiral of death

        void UpdateDynamics(Timestep deltaTime);
        void UpdatePhysicsResolution(Timestep deltaTime);
        
        void UpdateJustCreated();
        void UpdateDuplicatedObjects();
        void UpdatedExisting();

        void UpdateGlobalBounds();
        void UpdateCallbacks();
        void PostUpdate();

        Scene* m_scene = nullptr;

        // need a way to track physics uuids to gameobject uuids
        std::map<phy_uuid::UUID, UUID> m_physicsToGameObjectLookup = {};

        //underlying physics world
        myPhysx::PhysxWorld m_physicsWorld{ { Gravity.x, Gravity.y, Gravity.z} };

        //time accumulator
        double m_accumulator = 0.0;

        void OnRigidbodyAdd(Ecs::ComponentEvent<RigidbodyComponent>* rb);
        void OnRigidbodyRemove(Ecs::ComponentEvent<RigidbodyComponent>* rb);

        void OnBoxColliderAdd(Ecs::ComponentEvent<BoxColliderComponent>* bc);
        void OnBoxColliderRemove(Ecs::ComponentEvent<BoxColliderComponent>* bc);

        void OnCapsuleColliderAdd(Ecs::ComponentEvent<CapsuleColliderComponent>* cc);
        void OnCapsuleColliderRemove(Ecs::ComponentEvent<CapsuleColliderComponent>* cc);

        void OnSphereColliderAdd(Ecs::ComponentEvent<SphereColliderComponent>* sc);
        void OnSphereColliderRemove(Ecs::ComponentEvent<SphereColliderComponent>* sc);

        void OnMeshColliderAdd(Ecs::ComponentEvent<ConvexColliderComponent>* mc);
        void OnMeshColliderRemove(Ecs::ComponentEvent<ConvexColliderComponent>* mc);

        void InitializeRigidbody(RigidbodyComponent& rb);
        void InitializeBoxCollider(RigidbodyComponent& rb);
        void InitializeCapsuleCollider(RigidbodyComponent& rb);
        void InitializeSphereCollider(RigidbodyComponent& rb);
        void InitializeMeshCollider(RigidbodyComponent& rb);

        void DuplicateRigidbody(RigidbodyComponent& rb);


        void AddToLookUp(RigidbodyComponent& rb, GameObjectComponent& goc);

        void OnGameObjectEnable(GameObjectComponent::OnEnableEvent* e);
        void OnGameObjectDisable(GameObjectComponent::OnDisableEvent* e);

        void OnRaycastEvent(RaycastEvent* e);
        void OnRaycastAllEvent(RaycastAllEvent* e);
    };


}
