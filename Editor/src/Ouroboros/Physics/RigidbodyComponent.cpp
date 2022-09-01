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
#include "pch.h"

#include "RigidbodyComponent.h"
#include <rttr/registration>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::class_<Rigidbody2D>("Rigidbody2D")
            .property("Kinematic", &Rigidbody2D::Kinematic)
            .property("UseAutoMass", &Rigidbody2D::UseAutoMass)
            .property("GravityScale", &Rigidbody2D::GravityScale)
            // .property("Mass", &Rigidbody2D::GetMass, &Rigidbody2D::SetMass)
            .property("PhysicsMaterial", &Rigidbody2D::GetMaterial, &Rigidbody2D::SetMaterial)
            .property("Linear Drag", &Rigidbody2D::LinearDrag)
            .property("Angular Drag", &Rigidbody2D::AngularDrag)
            .property("DoNotRotate", &Rigidbody2D::DoNotRotate)
            ;

        registration::class_<PhysicsMaterial>("PhysicsMaterial")
            .property("Density", &PhysicsMaterial::Density)
            .property("Restitution", &PhysicsMaterial::Restitution)
            .property("DynamicFriction", &PhysicsMaterial::DynamicFriction)
            .property("StaticFriction", &PhysicsMaterial::StaticFriction);
    }

        
//    void Rigidbody2D::SetInertia()
//    {
//        if (!HasComponent<Collider2D>())
//        {
//            ResetInertia();
//            return;
//        }
//
//        // Set Moment of Inertia base on shape of object
//        switch (GetComponent<Collider2D>().GetNarrowPhaseCollider())
//        {
//        case ColliderType::BOX:
//        {
//            ENGINE_ASSERT_MSG(HasComponent<BoxCollider2D>(), "Box Collider Must exist at this point!");
//            auto bounds = ColliderUtil::GetGlobalDimensions(GetComponent<BoxCollider2D>(), GetComponent<Transform3D>());
//            m_data.Inertia = m_data.Mass * (bounds.x * bounds.x + bounds.y * bounds.y) / 12;
//            m_data.InverseInertia = 1.0f / m_data.Inertia;
//        }
//        break;
//        case ColliderType::CIRCLE:
//        {
//            ENGINE_ASSERT_MSG(HasComponent<CircleCollider2D>(), "Circle Collider Must exist at this point!");
//            auto rad = GetComponent<CircleCollider2D>().Radius;
//            m_data.Inertia = 0.5f * m_data.Mass * rad * rad;
//            m_data.InverseInertia = 1.0f / m_data.Inertia;
//        }
//        break;
//        case ColliderType::CONVEX:  // inertia currently not supported
//        default:
//        {
//            ResetInertia();
//        }
//        break;
//        }
//    }
//
//
//    void Rigidbody2D::ApplyForce(vec2 force)
//    {
//        if (IsDynamic())
//            m_force += force;
//    }
//
//    void Rigidbody2D::ApplyForceAtPosition(vec2 force, vec2 globalPosition)
//    {
//        if (IsDynamic())
//        {
//            m_force += force;
//            m_torque += globalPosition.Cross(force);
//
//#if OO_PRODUCTION && PHYSICS_DEBUG_MSG
//            LOG_ENGINE_INFO("Adding force{0}{1}", force.x, force.y);
//            LOG_ENGINE_INFO("Adding torque{0}", (m_centerOfMass - globalPosition).Cross(force));
//#endif
//
//#if !OO_PRODUCTION && PHYSICS_DRAW_FORCES
//
//            Renderer2D::DrawImmediateArrow(
//                m_centerOfMass + globalPosition,
//                m_centerOfMass + globalPosition + force,
//                oom::vec4{ 0, 1, 1, 1 });
//#endif
//        }
//    }
//
//    void Rigidbody2D::ApplyImpulse(vec2 impulse)
//    {
//        if (IsDynamic())
//            m_linearVelocity += impulse * m_data.InverseMass;
//    }
//
//    void Rigidbody2D::ApplyImpulseAtPosition(vec2 impulse, vec2 globalPosition)
//    {
//        if (IsDynamic())
//        {
//            m_linearVelocity += impulse * m_data.InverseMass;
//            m_angularVelocity += globalPosition.Cross(impulse) * m_data.InverseInertia;
//
//#if OO_PRODUCTION && PHYSICS_DEBUG_MSG
//            LOG_ENGINE_INFO("Adding impulse{0}{1}", impulse.x, impulse.y);
//            LOG_ENGINE_INFO("Adding angular velocity{0}", (m_centerOfMass - globalPosition).Cross(impulse));
//#endif
//
//#if !OO_PRODUCTION && PHYSICS_DRAW_FORCES
//            Renderer2D::DrawImmediateArrow(
//                m_centerOfMass + globalPosition,
//                m_centerOfMass + globalPosition + impulse,
//                oom::vec4{ 0, 1, 1, 1 });
//#endif
//        }
//    }
//
//    void Rigidbody2D::ApplyVelocity(vec2 velocity)
//    {
//        if (IsKinematic())
//            m_linearVelocity += velocity;
//    }
//
//    void Rigidbody2D::SetMass(float newMass)
//    {
//        if (newMass < 0.f) throw "Mass cannot be lesser than 0!";
//
//        //ENGINE_ASSERT_MSG(newMass > 0.f, "Mass canont be lesser then 0!");
//
//        //sets both mass and inverse mass to 0
//        m_data.Mass = 0.f;
//        m_data.InverseMass = 0.f;
//
//        // Set Rotational Involvement to 0
//        m_data.Inertia = 0.f;
//        m_data.InverseInertia = 0.f;
//
//        if (newMass > 0.f)
//        {
//            m_data.Mass = newMass;
//            m_data.InverseMass = 1.0f / m_data.Mass;
//
//            SetInertia();
//        }
//
//    }

}
