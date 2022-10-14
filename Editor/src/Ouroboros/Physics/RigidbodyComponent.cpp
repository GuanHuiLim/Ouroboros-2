/************************************************************************************//*!
\file           RigidbodyComponent.cpp
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
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::class_<RigidbodyComponent>("Rigidbody")
            //.property("type", &RigidbodyComponent::collider_type)
            .property("Static Object", &RigidbodyComponent::GetStatic, &RigidbodyComponent::SetStatic)
            //.property_readonly("underlying shape", &RigidbodyComponent::collider_shape)
            //.property_readonly("dirty", &RigidbodyComponent::Dirty)
            /*.property("Kinematic", &RigidbodyComponent::Kinematic)
            .property("UseAutoMass", &RigidbodyComponent::UseAutoMass)
            .property("GravityScale", &RigidbodyComponent::GravityScale)
            .property("Mass", &RigidbodyComponent::GetMass, &RigidbodyComponent::SetMass)
            .property("PhysicsMaterial", &RigidbodyComponent::GetMaterial, &RigidbodyComponent::SetMaterial)
            .property("Linear Drag", &RigidbodyComponent::LinearDrag)
            .property("Angular Drag", &RigidbodyComponent::AngularDrag)
            .property("DoNotRotate", &RigidbodyComponent::DoNotRotate)*/
            ;

        registration::class_<PhysicsMaterial>("Physics Material")
            //.property("Density", &PhysicsMaterial::Density)
            .property("Restitution", &PhysicsMaterial::Restitution)
            .property("DynamicFriction", &PhysicsMaterial::DynamicFriction)
            .property("StaticFriction", &PhysicsMaterial::StaticFriction);
        
    }

    PhysicsMaterial RigidbodyComponent::GetMaterial() const 
    { 
        return object.getMaterial(); 
    }
    
    glm::vec3 oo::RigidbodyComponent::GetPositionInPhysicsWorld() const
    {
        auto res = object.getposition();
        return { res.x,res.y, res.z };
    }

    glm::quat oo::RigidbodyComponent::GetOrientationInPhysicsWorld() const
    {
        auto res = object.getOrientation();
        return { res.x, res.y, res.z, res.w };
    }

    bool oo::RigidbodyComponent::GetStatic() const
    {
        return StaticObject; 
    }

    void oo::RigidbodyComponent::SetStatic(bool result)
    {
        StaticObject = result;
        if (StaticObject)
            object.setRigidType(rigid::rstatic);
        else
            object.setRigidType(rigid::rdynamic);
    }

    void RigidbodyComponent::SetMaterial(PhysicsMaterial material) { object.setMaterial(material); }
    void RigidbodyComponent::SetPosOrientation(PxVec3 pos, PxQuat quat) { object.setPosOrientation(pos, quat); }
    void RigidbodyComponent::EnableGravity(bool enable) { object.setGravity(enable); }

    // prob functions that dont really need

    void RigidbodyComponent::SetMass(float mass)
    {
        object.setMass(static_cast<PxReal>(mass));
    }
    void RigidbodyComponent::SetAngularDamping(float angularDamping)
    {
        object.setAngularDamping(static_cast<PxReal>(angularDamping));
    }
    void RigidbodyComponent::SetAngularVelocity(glm::vec3 angularVelocity)
    {
        object.setAngularVelocity(PxVec3{ angularVelocity.x, angularVelocity.y, angularVelocity.z });
    }
    void RigidbodyComponent::SetLinearDamping(float linearDamping)
    {
        object.setLinearDamping(static_cast<PxReal>(linearDamping));
    }
    void RigidbodyComponent::SetVelocity(glm::vec3 linearVelocity)
    {
        object.setLinearVelocity(PxVec3{ linearVelocity.x, linearVelocity.y, linearVelocity.z });
    }
}