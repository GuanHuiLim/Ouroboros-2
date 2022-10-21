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
#include <Physics/Source/phy.h>

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

        PhysicsMaterial(myPhysx::Material const& mat)
            : Restitution { mat.restitution }
            , DynamicFriction { mat.dynamicFriction }
            , StaticFriction { mat.staticFriction }
        {
        }
        
        operator myPhysx::Material()
        {
            return myPhysx::Material
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
        bool IsStaticObject = true;
        myPhysx::PhysicsObject object{};

        PhysicsMaterial GetMaterial() const;
        glm::vec3 GetPositionInPhysicsWorld() const;
        glm::quat GetOrientationInPhysicsWorld() const;

        void SetStatic(bool result);

        float GetMass() const;
        float GetAngularDamping() const;
        glm::vec3 GetAngularVelocity() const;
        float GetLinearDamping() const;
        glm::vec3 GetLinearVelocity() const;

        bool IsGravityEnabled() const;
        bool IsGravityDisabled() const;

        bool IsStatic() const;
        bool IsKinematic() const;
        bool IsDynamic() const;

        void SetMaterial(PhysicsMaterial material);
        void SetPosOrientation(glm::vec3 pos, glm::quat quat);

        void SetGravity(bool enable);
        void EnableGravity();
        void DisableGravity();
        
        void SetKinematic(bool kine);
        void SetMass(float mass);
        void SetAngularDamping(float angularDamping);
        void SetAngularVelocity(glm::vec3 angularVelocity);
        void SetLinearDamping(float linearDamping);
        void SetLinearVelocity(glm::vec3 linearVelocity);

        void AddForce(vec3 force, ForceMode type = ForceMode::FORCE);
        void AddTorque(vec3 force, ForceMode type = ForceMode::FORCE);

        RTTR_ENABLE();
    };

}