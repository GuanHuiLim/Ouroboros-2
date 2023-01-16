/************************************************************************************//*!
\file           RigidbodyComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 01, 2022
\brief          Rigidbody Component describes the gameobject that has it to
                have physics related dynamics (gravity, restitution, forces etc)
                and could result itself or others to have physics based
                responses (collision response)

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "PhysicsFwd.h"

#include <glm/glm.hpp>
#include <rttr/type>
//#include <Physics/Source/phy.h>
#include <Physics/Source/physics.h>

#include "Utility/UUID.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* Describes the supported variables that make up a material and how           */
    /* rigidbodies reacts when moving or collision.                                */
    /*-----------------------------------------------------------------------------*/
    struct PhysicsMaterial final
    {
        //Density currently purely used as mass
        //double Density          = 1.0;
        double Restitution      = 0.0;
        // Dynamic Friction is typically lesser then static friction
        double DynamicFriction  = 0.2;
        double StaticFriction   = 0.4;

        PhysicsMaterial() = default;

        PhysicsMaterial(phy::Material const& mat)
            : Restitution { mat.restitution }
            , DynamicFriction { mat.dynamicFriction }
            , StaticFriction { mat.staticFriction }
        {
        }
        
        operator phy::Material()
        {
            return phy::Material
            { 
                .staticFriction = static_cast<float>(StaticFriction), 
                .dynamicFriction = static_cast<float>(DynamicFriction), 
                .restitution = static_cast<float>(Restitution)
            };
        }

        RTTR_ENABLE();
    };
    
    enum class ForceMode
    {
        FORCE,
        ACCELERATION,
        IMPULSE,
        VELOCITY_CHANGE,
    };


    /*-----------------------------------------------------------------------------*/
    /* Describes and Enables Entites with this component attached to               */
    /* abide by the laws of physics.                                               */
    /*-----------------------------------------------------------------------------*/
    class RigidbodyComponent final
    {
    public:
        phy::PhysicsObject underlying_object{};
        phy::PhysicsObject desired_object{};

        std::vector<phy::PhysicsCommand> external_commands;

    /*private:
        
        bool IsStaticObject = true;

    public:
        bool IsTriggerObject = false;
        
        bool LockXAxisPosition = false;
        bool LockYAxisPosition = false;
        bool LockZAxisPosition = false;
        bool LockXAxisRotation = false;
        bool LockYAxisRotation = false;
        bool LockZAxisRotation = false;*/
        
        vec3 Offset = { 0.0, 0.0, 0.0 };

        // property wrapper functions
        oo::UUID GetUnderlyingUUID() const;
        
        PhysicsMaterial GetMaterial() const;
        void SetMaterial(PhysicsMaterial material);

        vec3 GetPositionInPhysicsWorld() const;
        quat GetOrientationInPhysicsWorld() const;
        void SetPosOrientation(vec3 pos, quat quat);

        float GetMass() const;
        void SetMass(float mass);

        float GetLinearDamping() const;
        void SetLinearDamping(float linearDamping);
        float GetAngularDamping() const;
        void SetAngularDamping(float angularDamping);

        vec3 GetLinearVelocity() const;
        void SetLinearVelocity(vec3 linearVelocity);
        vec3 GetAngularVelocity() const;
        void SetAngularVelocity(vec3 angularVelocity);

        bool IsStatic() const;
        void SetStatic(bool result);

        bool IsGravityEnabled() const;
        bool IsGravityDisabled() const;
        void SetGravity(bool enable);

        bool IsKinematic() const;
        void SetKinematic(bool kine);
        bool IsDynamic() const;

        void SetTrigger(bool enable);
        bool IsTrigger() const;
        bool IsCollider() const;
        
        void EnableCollider();
        void DisableCollider();

        // command wrapper functions
        void AddForce(vec3 force, ForceMode type = ForceMode::FORCE);
        void AddTorque(vec3 force, ForceMode type = ForceMode::FORCE);

        RTTR_ENABLE();
    };

}