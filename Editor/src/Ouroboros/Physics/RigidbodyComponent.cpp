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

        registration::enumeration<ColliderShape>("Collider Shape")
        (
            value("None", ColliderShape::NONE),
            value("Box", ColliderShape::BOX),
            value("Capsule", ColliderShape::CAPSULE),
            value("Sphere", ColliderShape::SPHERE),
            value("Mesh", ColliderShape::MESH)
        );

        registration::class_<RigidbodyComponent>("Rigidbody")
            //.property("type", &RigidbodyComponent::collider_type)
            .property_readonly("Underlying physX UUID", &RigidbodyComponent::GetUnderlyingUUID)
            .property("Offset", &RigidbodyComponent::Offset)
            .property("Physics Material", &RigidbodyComponent::GetMaterial, &RigidbodyComponent::SetMaterial)
            .property("Mass", &RigidbodyComponent::GetMass, &RigidbodyComponent::SetMass)
            .property("Linear Damping", &RigidbodyComponent::GetLinearDamping, &RigidbodyComponent::SetLinearDamping)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Angular Damping", &RigidbodyComponent::GetAngularDamping, &RigidbodyComponent::SetAngularDamping)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Velocity", &RigidbodyComponent::GetLinearVelocity)
            .property_readonly("Angular Velocity", &RigidbodyComponent::GetAngularVelocity)
            .property_readonly("Shape",&RigidbodyComponent::GetUnderlyingShape)
            .property("IsTrigger", &RigidbodyComponent::IsTrigger, &RigidbodyComponent::SetTrigger)
            .property("Enable Gravity", &RigidbodyComponent::IsGravityEnabled, &RigidbodyComponent::SetGravity)
            .property("Static Object", &RigidbodyComponent::IsStatic, &RigidbodyComponent::SetStatic)
            .property("Lock X Axis Position", &RigidbodyComponent::IsPositionLockedX, &RigidbodyComponent::LockPositionX)
            .property("Lock Y Axis Position", &RigidbodyComponent::IsPositionLockedY, &RigidbodyComponent::LockPositionY)
            .property("Lock Z Axis Position", &RigidbodyComponent::IsPositionLockedZ, &RigidbodyComponent::LockPositionZ)
            .property("Lock X Axis Rotation", &RigidbodyComponent::IsRotationLockedX, &RigidbodyComponent::LockRotationX)
            .property("Lock Y Axis Rotation", &RigidbodyComponent::IsRotationLockedY, &RigidbodyComponent::LockRotationY)
            .property("Lock Z Axis Rotation", &RigidbodyComponent::IsRotationLockedZ, &RigidbodyComponent::LockRotationZ)
            
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
        return underlying_object.material; 
    }
    
    vec3 oo::RigidbodyComponent::GetPositionInPhysicsWorld() const
    {
        auto& res = underlying_object.position;
        return { res.x, res.y, res.z };
    }

    quat oo::RigidbodyComponent::GetOrientationInPhysicsWorld() const
    {
        auto& res = underlying_object.orientation;
        return { res.w, res.x, res.y, res.z,  };
    }

    void oo::RigidbodyComponent::SetStatic(bool result)
    {
        result ? desired_object.rigid_type = phy::rigid::rstatic : desired_object.rigid_type = phy::rigid::rdynamic;
        HasChanged = true;
    }

    float oo::RigidbodyComponent::GetMass() const
    {
        return underlying_object.mass;
    }

    float oo::RigidbodyComponent::GetAngularDamping() const
    {
        return underlying_object.angularDamping;
    }

    vec3 oo::RigidbodyComponent::GetAngularVelocity() const
    {
        auto& vel = underlying_object.angularVel;
        return vec3{ vel.x, vel.y, vel.z };
    }

    float oo::RigidbodyComponent::GetLinearDamping() const
    {
        return underlying_object.linearDamping;
    }

    vec3 oo::RigidbodyComponent::GetLinearVelocity() const
    {
        auto& vel = underlying_object.linearVel;
        return vec3{ vel.x, vel.y, vel.z };
    }

    bool oo::RigidbodyComponent::IsGravityEnabled() const
    {
        return underlying_object.gravity_enabled;
    }

    bool oo::RigidbodyComponent::IsGravityDisabled() const
    {
        return !IsGravityEnabled();
    }

    bool oo::RigidbodyComponent::IsStatic() const
    {
        return underlying_object.rigid_type == phy::rigid::rstatic;
    }

    bool oo::RigidbodyComponent::IsKinematic() const
    {
        return !IsStatic() && underlying_object.is_kinematic;
    }

    bool oo::RigidbodyComponent::IsDynamic() const
    {
        return !IsStatic() && !underlying_object.is_kinematic;
    }

    bool oo::RigidbodyComponent::IsTrigger() const
    {
        return underlying_object.is_trigger;
    }

    bool oo::RigidbodyComponent::IsCollider() const
    {
        return !IsTrigger();
    }

    void oo::RigidbodyComponent::EnableCollider()
    {
        desired_object.is_collider_enabled = true;
        HasChanged = true;
    }

    void oo::RigidbodyComponent::DisableCollider()
    {
        desired_object.is_collider_enabled = false;
    }

    void oo::RigidbodyComponent::LockPositionX(bool enable)
    {
        desired_object.lockPositionAxis.x_axis = enable;
        HasChanged = true;
    }

    bool oo::RigidbodyComponent::IsPositionLockedX()
    {
        return underlying_object.lockPositionAxis.x_axis;
    }

    void oo::RigidbodyComponent::LockPositionY(bool enable)
    {
        desired_object.lockPositionAxis.y_axis = enable;
        HasChanged = true;
    }

    bool oo::RigidbodyComponent::IsPositionLockedY()
    {
        return underlying_object.lockPositionAxis.y_axis;
    }

    void oo::RigidbodyComponent::LockPositionZ(bool enable)
    {
        desired_object.lockPositionAxis.z_axis = enable;
        HasChanged = true;
    }

    bool oo::RigidbodyComponent::IsPositionLockedZ()
    {
        return underlying_object.lockPositionAxis.z_axis;
    }

    void oo::RigidbodyComponent::LockRotationX(bool enable)
    {
        desired_object.lockRotationAxis.x_axis = enable;
        HasChanged = true;
    }

    bool oo::RigidbodyComponent::IsRotationLockedX()
    {
        return underlying_object.lockRotationAxis.x_axis;
    }

    void oo::RigidbodyComponent::LockRotationZ(bool enable)
    {
        desired_object.lockRotationAxis.z_axis = enable;
        HasChanged = true;
    }

    bool oo::RigidbodyComponent::IsRotationLockedZ()
    {
        return underlying_object.lockRotationAxis.z_axis;
    }

    void oo::RigidbodyComponent::LockRotationY(bool enable)
    {
        desired_object.lockRotationAxis.y_axis = enable;
        HasChanged = true;
    }

    bool oo::RigidbodyComponent::IsRotationLockedY()
    {
        return underlying_object.lockRotationAxis.y_axis;
    }

    void oo::RigidbodyComponent::SetTrigger(bool enable)
    {
        desired_object.is_trigger = enable;
        HasChanged = true;
    }

    void RigidbodyComponent::SetMaterial(PhysicsMaterial material) 
    { 
        desired_object.material = material;
        HasChanged = true;
    }

    void RigidbodyComponent::SetPosOrientation(vec3 pos, quat quat)
    {
        /*glm::vec3 cur_pos = { underlying_object.position.x, underlying_object.position.y, underlying_object.position.z };
        glm::quat cur_quat = { underlying_object.orientation.x, underlying_object.orientation.y, underlying_object.orientation.z, underlying_object.orientation.w };
        if (glm::dot(cur_pos, pos) > glm::epsilon<float>() * 5.0f || glm::dot(cur_quat, quat) > glm::epsilon<float>() * 5.0f)
        {*/
            desired_object.position = { pos.x, pos.y, pos.z };
            desired_object.orientation = { quat.x, quat.y, quat.z, quat.w };
            HasChanged = true;
        //}
    }
    
    void oo::RigidbodyComponent::SetGravity(bool enable)
    {
        desired_object.gravity_enabled = enable;
        HasChanged = true;
    }

    void oo::RigidbodyComponent::SetKinematic(bool kine) 
    { 
        desired_object.is_kinematic = kine; 
        HasChanged = true;
    }

    // prob functions that dont really need

    void RigidbodyComponent::SetMass(float mass)
    {
        desired_object.mass = static_cast<PxReal>(mass);
        HasChanged = true;
    }

    void RigidbodyComponent::SetAngularDamping(float angularDamping)
    {
        desired_object.angularDamping = static_cast<PxReal>(angularDamping);
        HasChanged = true;
    }

    void RigidbodyComponent::SetAngularVelocity(vec3 angularVelocity)
    {
        desired_object.angularVel = PxVec3{ angularVelocity.x, angularVelocity.y, angularVelocity.z };
        HasChanged = true;
    }

    ColliderShape oo::RigidbodyComponent::GetUnderlyingShape() const
    {
        switch (underlying_object.shape_type)
        {
        case phy::shape::box:
            ColliderShape::BOX;
            break;
        case phy::shape::capsule:
            ColliderShape::CAPSULE;
            break;
        case phy::shape::sphere:
            ColliderShape::SPHERE;
            break;

        case phy::shape::none:
        case phy::shape::plane:
        default:
            return ColliderShape::NONE;
        }
    }

    void RigidbodyComponent::SetLinearDamping(float linearDamping)
    {
        desired_object.linearDamping = static_cast<PxReal>(linearDamping);
        HasChanged = true;
    }

    void RigidbodyComponent::SetLinearVelocity(vec3 linearVelocity)
    {
        desired_object.linearVel = PxVec3{ linearVelocity.x, linearVelocity.y, linearVelocity.z };
        HasChanged = true;
    }

    void oo::RigidbodyComponent::AddForce(vec3 force, ForceMode type)
    {
        phy::PhysicsCommand cmd;
        cmd.Id = underlying_object.id;
        cmd.Force = PxVec3{ force.x, force.y, force.z };
        cmd.AddForce = true;
        switch (type)
        {
        case ForceMode::FORCE:
            cmd.Type = phy::force::force;
            break;

        case ForceMode::ACCELERATION:
            cmd.Type = phy::force::acceleration;
            break;
        
        case ForceMode::IMPULSE:
            cmd.Type = phy::force::impulse;
            break;
        
        case ForceMode::VELOCITY_CHANGE:
            cmd.Type = phy::force::velocityChanged;
            break;
        }

        external_commands.emplace_back(cmd);
    }

    void oo::RigidbodyComponent::AddTorque(vec3 force, ForceMode type)
    {
        phy::PhysicsCommand cmd;
        cmd.Id = underlying_object.id;
        cmd.Force = PxVec3{ force.x, force.y, force.z };
        cmd.AddTorque = true;
        switch (type)
        {
        case ForceMode::FORCE:
            cmd.Type = phy::force::force;
            break;

        case ForceMode::ACCELERATION:
            cmd.Type = phy::force::acceleration;
            break;

        case ForceMode::IMPULSE:
            cmd.Type = phy::force::impulse;
            break;

        case ForceMode::VELOCITY_CHANGE:
            cmd.Type = phy::force::velocityChanged;
            break;
        }

        external_commands.emplace_back(cmd);
    }

    oo::UUID oo::RigidbodyComponent::GetUnderlyingUUID() const
    {
        return oo::UUID{ std::uint64_t{underlying_object.id} };
    }


}