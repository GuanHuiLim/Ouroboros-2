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
            .property("Static Object", &RigidbodyComponent::IsStatic, &RigidbodyComponent::SetStatic)
            .property("IsTrigger", &RigidbodyComponent::IsTrigger, &RigidbodyComponent::SetTrigger)
            .property("Disable Gravity", &RigidbodyComponent::IsGravityDisabled, &RigidbodyComponent::SetGravity)
            .property("Physics Material", &RigidbodyComponent::GetMaterial, &RigidbodyComponent::SetMaterial)
            .property("Mass", &RigidbodyComponent::GetMass, &RigidbodyComponent::SetMass)
            .property_readonly("Velocity", &RigidbodyComponent::GetLinearVelocity)
            .property("Linear Damping", &RigidbodyComponent::GetLinearDamping, &RigidbodyComponent::SetLinearDamping)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Angular Velocity", &RigidbodyComponent::GetAngularVelocity)
            .property("Angular Damping", &RigidbodyComponent::GetAngularDamping, &RigidbodyComponent::SetAngularDamping)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Offset", &RigidbodyComponent::Offset)
            .property("Lock X Axis Position", &RigidbodyComponent::LockXAxisPosition)
            .property("Lock Y Axis Position", &RigidbodyComponent::LockYAxisPosition)
            .property("Lock Z Axis Position", &RigidbodyComponent::LockZAxisPosition)
            .property("Lock X Axis Rotation", &RigidbodyComponent::LockXAxisRotation)
            .property("Lock Y Axis Rotation", &RigidbodyComponent::LockYAxisRotation)
            .property("Lock Z Axis Rotation", &RigidbodyComponent::LockZAxisRotation)
            .property_readonly("Underlying physX UUID", &RigidbodyComponent::GetUnderlyingUUID)
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
    
    vec3 oo::RigidbodyComponent::GetPositionInPhysicsWorld() const
    {
        auto res = object.getposition();
        return { res.x, res.y, res.z };
    }

    quat oo::RigidbodyComponent::GetOrientationInPhysicsWorld() const
    {
        auto res = object.getOrientation();
        return { res.w, res.x, res.y, res.z,  };
    }

    void oo::RigidbodyComponent::SetStatic(bool result)
    {
        IsStaticObject = result;
        if (IsStaticObject)
            object.setRigidType(myPhysx::rigid::rstatic);
        else
            object.setRigidType(myPhysx::rigid::rdynamic);
    }

    float oo::RigidbodyComponent::GetMass() const
    {
        return object.getMass();
    }

    float oo::RigidbodyComponent::GetAngularDamping() const
    {
        return object.getAngularDamping();
    }

    vec3 oo::RigidbodyComponent::GetAngularVelocity() const
    {
        auto vel = object.getAngularVelocity();
        return vec3{ vel.x, vel.y, vel.z };
    }

    float oo::RigidbodyComponent::GetLinearDamping() const
    {
        return object.getLinearDamping();
    }

    vec3 oo::RigidbodyComponent::GetLinearVelocity() const
    {
        auto vel = object.getLinearVelocity();
        return vec3{ vel.x, vel.y, vel.z };
    }

    bool oo::RigidbodyComponent::IsGravityEnabled() const
    {
        return !IsGravityDisabled();
    }

    bool oo::RigidbodyComponent::IsGravityDisabled() const
    {
        return object.useGravity();
    }

    bool oo::RigidbodyComponent::IsStatic() const
    {
        return IsStaticObject;
    }

    bool oo::RigidbodyComponent::IsKinematic() const
    {
        return !IsStatic() && object.isKinematic();
    }

    bool oo::RigidbodyComponent::IsDynamic() const
    {
        return !IsStatic() && !object.isKinematic();
    }

    bool oo::RigidbodyComponent::IsTrigger() const
    {
        return IsTriggerObject;//&& object.isTrigger();
    }

    bool oo::RigidbodyComponent::IsCollider() const
    {
        return !IsTrigger();
    }

    void oo::RigidbodyComponent::EnableCollider()
    {
        object.enableCollider(true);
    }

    void oo::RigidbodyComponent::DisableCollider()
    {
        object.enableCollider(false);
    }

    void oo::RigidbodyComponent::SetTrigger(bool enable)
    {
        IsTriggerObject = enable;
        //object.setTriggerShape(IsTriggerObject);
    }

    void RigidbodyComponent::SetMaterial(PhysicsMaterial material) 
    { 
        object.setMaterial(material); 
    }

    void RigidbodyComponent::SetPosOrientation(vec3 pos, quat quat) 
    { 
        object.setPosOrientation({ pos.x, pos.y, pos.z }, { quat.x, quat.y, quat.z, quat.w }); 
    }
    
    void oo::RigidbodyComponent::SetGravity(bool to_disable)
    {
        // only applies to none static objects.
        if (!IsStaticObject)
            object.disableGravity(to_disable);
    }

    /*void RigidbodyComponent::EnableGravity()
    { 
        SetGravity(false);
    }
    
    void RigidbodyComponent::DisableGravity()
    {
        SetGravity(true);
    }*/

    void oo::RigidbodyComponent::SetKinematic(bool kine) { object.enableKinematic(kine); }

    // prob functions that dont really need

    void RigidbodyComponent::SetMass(float mass)
    {
        object.setMass(static_cast<PxReal>(mass));
    }

    void RigidbodyComponent::SetAngularDamping(float angularDamping)
    {
        object.setAngularDamping(static_cast<PxReal>(angularDamping));
    }

    void RigidbodyComponent::SetAngularVelocity(vec3 angularVelocity)
    {
        object.setAngularVelocity(PxVec3{ angularVelocity.x, angularVelocity.y, angularVelocity.z });
    }

    void RigidbodyComponent::SetLinearDamping(float linearDamping)
    {
        object.setLinearDamping(static_cast<PxReal>(linearDamping));
    }

    void RigidbodyComponent::SetLinearVelocity(vec3 linearVelocity)
    {
        object.setLinearVelocity(PxVec3{ linearVelocity.x, linearVelocity.y, linearVelocity.z });
    }

    void oo::RigidbodyComponent::AddForce(vec3 force, ForceMode type)
    {
        switch (type)
        {
        case ForceMode::FORCE:
            object.addForce(PxVec3{ force.x, force.y, force.z }, myPhysx::force::force);
            break;

        case ForceMode::ACCELERATION:
            object.addForce(PxVec3{ force.x, force.y, force.z }, myPhysx::force::acceleration);
            break;
        
        case ForceMode::IMPULSE:
            object.addForce(PxVec3{ force.x, force.y, force.z }, myPhysx::force::impulse);
            break;
        
        case ForceMode::VELOCITY_CHANGE:
            object.addForce(PxVec3{ force.x, force.y, force.z }, myPhysx::force::velocityChanged);
            break;
        }
    }

    void oo::RigidbodyComponent::AddTorque(vec3 force, ForceMode type)
    {
        switch (type)
        {
        case ForceMode::FORCE:
            object.addTorque(PxVec3{ force.x, force.y, force.z }, myPhysx::force::force);
            break;

        case ForceMode::ACCELERATION:
            object.addTorque(PxVec3{ force.x, force.y, force.z }, myPhysx::force::acceleration);
            break;

        case ForceMode::IMPULSE:
            object.addTorque(PxVec3{ force.x, force.y, force.z }, myPhysx::force::impulse);
            break;

        case ForceMode::VELOCITY_CHANGE:
            object.addTorque(PxVec3{ force.x, force.y, force.z }, myPhysx::force::velocityChanged);
            break;
        }
    }

    oo::UUID oo::RigidbodyComponent::GetUnderlyingUUID() const
    {
        return oo::UUID{ std::uint64_t{object.id} };
    }

}