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

        PhysicsMaterial(::Material const& mat)
            : Restitution { mat.restitution }
            , DynamicFriction { mat.dynamicFriction }
            , StaticFriction { mat.staticFriction }
        {
        }
        
        operator ::Material()
        {
            return ::Material
            { 
                .staticFriction = static_cast<float>(StaticFriction), 
                .dynamicFriction = static_cast<float>(DynamicFriction), 
                .restitution = static_cast<float>(Restitution)
            };
        }

        RTTR_ENABLE();
    };

    /*-----------------------------------------------------------------------------*/
    /* Describes and Enables Entites with this component attached to               */
    /* abide by the laws of physics.                                               */
    /*-----------------------------------------------------------------------------*/
    class RigidbodyComponent final
    {
    public:
        PhysicsObject object{};

        PhysicsMaterial GetMaterial() const;
        glm::vec3 GetPositionInPhysicsWorld() const;
        glm::quat GetOrientationInPhysicsWorld() const;

        bool StaticObject = true;
        bool GetStatic() const;
        void SetStatic(bool result);

        //rigid GetRigidType() const { object.getRigidType(type); }
        //void SetRigidType(rigid type) { object.setRigidType(type); }
        
        void SetMaterial(PhysicsMaterial material);
        void SetPosOrientation(PxVec3 pos, PxQuat quat);

        void EnableGravity(bool enable);
        //void SetKinematic(bool kine) { object.setKinematic(kine); }

        // set default value for each type of shape & can change shape too
        //void SetShape(shape shape) { object.setShape(shape); }

        //// change each individual property based on its shape
        //void SetBoxProperty(float halfextent_width, float halfextent_height, float halfextent_depth)
        //{
        //    object.setBoxProperty(halfextent_width, halfextent_height, halfextent_depth);
        //}

        //void SetSphereProperty(float radius)
        //{
        //    object.setSphereProperty(radius);
        //}

        ////void setPlaneProperty(float radius);
        //void SetCapsuleProperty(float radius, float halfHeight)
        //{
        //    object.setCapsuleProperty(radius, halfHeight);
        //}

        // prob functions that dont really need
        void SetMass(float mass);

        void SetAngularDamping(float angularDamping);

        void SetAngularVelocity(glm::vec3 angularVelocity);

        void SetLinearDamping(float linearDamping);

        void SetVelocity(glm::vec3 linearVelocity);

        RTTR_ENABLE();
    };

}