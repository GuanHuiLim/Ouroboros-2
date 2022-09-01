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

namespace oo
{
    struct MassData final
    {
        double Mass = 10.0;
        double InverseMass = 0.1;

        // for rotations
        double Inertia = 0.0;
        double InverseInertia = 0.0;

        RTTR_ENABLE();
    };

    /*-----------------------------------------------------------------------------*/
    /* Describes the supported variables that make up a material and how           */
    /* rigidbodies reacts when moving or collision.                                */
    /*-----------------------------------------------------------------------------*/
    struct PhysicsMaterial final
    {
        //Density currently purely used as mass
        double Density          = 1.0;
        double Restitution      = 0.0;
        // Dynamic Friction is typically lesser then static friction
        double DynamicFriction  = 0.2;
        double StaticFriction   = 0.4;

        RTTR_ENABLE();
    };


    /*-----------------------------------------------------------------------------*/
    /* Describes and Enables Entites with this component attached to               */
    /* abide by the laws of physics.                                               */
    /*-----------------------------------------------------------------------------*/
    class Rigidbody2D final
    {
    private:
        PhysicsMaterial m_material;
        MassData m_data;

        vec2 m_linearVelocity;
        vec2 m_force;

        // Angular components : most components are float in 2D, vec3 in 3D
        //float m_orientation;
        float m_angularVelocity;
        float m_torque;
        // Used for interpolation
        vec3 m_prevPos;

        //Accumulated impulse
        vec2 m_accumulatedImpulse;

        // Center of Mass
        vec2 m_centerOfMass;

    public:

        /*-----------------------------------------------------------------------------*/
        /* Public Adaptable Variables                                                  */
        /*-----------------------------------------------------------------------------*/
        bool Kinematic = false;
        bool Interpolate = false;
        bool UseAutoMass = true;
        bool DoNotRotate = false;
        float GravityScale = 1.0f;
        float LinearDrag = 0.1f;
        float AngularDrag = 0.1f;
        vec2 CenterOfMassOffset = vec2{ 0 };

        /*-----------------------------------------------------------------------------*/
        /* Public Interface Functions                                                  */
        /*-----------------------------------------------------------------------------*/

        /*-----------------------------------------------------------------------------*/
        /* Getter Functions                                                            */
        /*-----------------------------------------------------------------------------*/
        float GetMass()                 const { return m_data.Mass; }
        float GetInverseMass()          const { return m_data.InverseMass; }
        float GetInertia()              const { return m_data.Inertia; }
        float GetInverseInertia()       const { return m_data.InverseInertia; }
        float GetAngularVelocity()      const { return m_angularVelocity; }
        vec2 GetForce()                 const { return m_force; }
        vec2 GetVelocity()              const { return m_linearVelocity; }
        PhysicsMaterial GetMaterial()   const { return m_material; }

        /*-----------------------------------------------------------------------------*/
        /* Setter Functions                                                            */
        /*-----------------------------------------------------------------------------*/
        void SetVelocity(vec2 newVel) { m_linearVelocity = newVel; }
        void SetForce(vec2 newForce) { m_force = newForce; }
        void SetTorque(float newTorque) { m_torque = newTorque; }
        void SetMaterial(PhysicsMaterial material) { m_material = material; }

        /*-----------------------------------------------------------------------------*/
        /* Query Functions                                                             */
        /*-----------------------------------------------------------------------------*/
        bool IsKinematic()  const { return Kinematic; }
        bool IsDynamic()    const { return !Kinematic; }

        void SetInertia();
        void ResetInertia() { m_data.Inertia = m_data.Inertia = 0.f; }

        RTTR_ENABLE();
    };

    

}