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
            .property_readonly("Underlying Object", &RigidbodyComponent::underlying_object)
            .property_readonly("Desired Object", &RigidbodyComponent::desired_object)
            .property("Input Layer", &RigidbodyComponent::GetInputLayer, &RigidbodyComponent::SetInputLayer)(metadata(UI_metadata::HIDDEN, true))
            .property("Output Layer", &RigidbodyComponent::GetOutputLayer, &RigidbodyComponent::SetOutputLayer)(metadata(UI_metadata::HIDDEN, true))
            .property("Static Object", &RigidbodyComponent::IsStatic, &RigidbodyComponent::SetStatic)
            .property("IsTrigger", &RigidbodyComponent::IsTrigger, &RigidbodyComponent::SetTrigger)
            .property("Enable Gravity", &RigidbodyComponent::IsGravityEnabled, &RigidbodyComponent::SetGravity)
            .property("Physics Material", &RigidbodyComponent::GetMaterial, &RigidbodyComponent::SetMaterial)
            .property("Mass", &RigidbodyComponent::GetMass, &RigidbodyComponent::SetMass)
            .property_readonly("Velocity", &RigidbodyComponent::GetLinearVel)
            .property("Linear Damping", &RigidbodyComponent::GetLinearDamping, &RigidbodyComponent::SetLinearDamping)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Angular Velocity", &RigidbodyComponent::GetAngularVel)
            .property("Angular Damping", &RigidbodyComponent::GetAngularDamping, &RigidbodyComponent::SetAngularDamping)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Offset", &RigidbodyComponent::GetOffset, &RigidbodyComponent::SetOffset)
            .property("Lock X Axis Position", &RigidbodyComponent::IsXAxisPosLocked, &RigidbodyComponent::LockXAxisPos)
            .property("Lock Y Axis Position", &RigidbodyComponent::IsYAxisPosLocked, &RigidbodyComponent::LockYAxisPos)
            .property("Lock Z Axis Position", &RigidbodyComponent::IsZAxisPosLocked, &RigidbodyComponent::LockZAxisPos)
            .property("Lock X Axis Rotation", &RigidbodyComponent::IsXAxisRotLocked, &RigidbodyComponent::LockXAxisRot)
            .property("Lock Y Axis Rotation", &RigidbodyComponent::IsYAxisRotLocked, &RigidbodyComponent::LockYAxisRot)
            .property("Lock Z Axis Rotation", &RigidbodyComponent::IsZAxisRotLocked, &RigidbodyComponent::LockZAxisRot)
            .property_readonly("Underlying physX UUID", &RigidbodyComponent::GetUnderlyingUUID)
            ;

        registration::class_<PhysicsMaterial>("Physics Material")
            .property("Restitution", &PhysicsMaterial::Restitution)
            .property("DynamicFriction", &PhysicsMaterial::DynamicFriction)
            .property("StaticFriction", &PhysicsMaterial::StaticFriction);

        registration::class_<myPhysx::PhysicsObject>("Underlying object")
            .property("Material", &myPhysx::PhysicsObject::material)
            //.property("Position", &myPhysx::PhysicsObject::position)
            //.property("Orientation", &myPhysx::PhysicsObject::orientation)
            .property("Mass", &myPhysx::PhysicsObject::mass)
            .property("Inverse Mass", &myPhysx::PhysicsObject::invmass)
            .property("Linear Damping", &myPhysx::PhysicsObject::linearDamping)
            .property("Angular Damping", &myPhysx::PhysicsObject::angularDamping)
            //.property("Linear Velocity", &myPhysx::PhysicsObject::linearVel)
            //.property("Angular Velocity", &myPhysx::PhysicsObject::angularVel)
            .property("Shape Type", &myPhysx::PhysicsObject::shape_type)
            .property("Rigid Type", &myPhysx::PhysicsObject::rigid_type)
            .property("Lock Position Axis", &myPhysx::PhysicsObject::lockPositionAxis)
            .property("Lock Rotation Axis", &myPhysx::PhysicsObject::lockRotationAxis)
            .property("Is Trigger", &myPhysx::PhysicsObject::is_trigger)
            .property("Gravity Enabled", &myPhysx::PhysicsObject::gravity_enabled)
            .property("Is Kinematic", &myPhysx::PhysicsObject::is_kinematic)
            .property("Is Collider", &myPhysx::PhysicsObject::is_collider)
            .property("Input Layer", &myPhysx::PhysicsObject::filterIn)
            .property("Output Layer", &myPhysx::PhysicsObject::filterOut)
            //.property("", &myPhysx::PhysicsObject::meshScale)
            ;
        
        registration::class_<myPhysx::Material>("Physics Material")
            .property("Static friction", &myPhysx::Material::staticFriction)
            .property("Dynamic friction", &myPhysx::Material::dynamicFriction)
            .property("Restitution", &myPhysx::Material::restitution)
            ;
        
        //registration::class_<myPhysx::shape>("Physics Material")
        //    .property("Static friction", &myPhysx::Material::staticFriction)
        //    .property("Dynamic friction", &myPhysx::Material::dynamicFriction)
        //    .property("Restitution", &myPhysx::Material::restitution)
        //    ;

        registration::enumeration<myPhysx::shape>("rigidbody shape")
            (
                value("None", myPhysx::shape::none),

                value("Box", myPhysx::shape::box),
                value("Sphere", myPhysx::shape::sphere),
                value("Capsule", myPhysx::shape::capsule),

                value("Plane", myPhysx::shape::plane),
                value("Convex", myPhysx::shape::convex)
            );

        registration::enumeration<myPhysx::rigid>("rigidbody type")
            (
                value("None", myPhysx::rigid::none),

                value("Static", myPhysx::rigid::rstatic),
                value("Dynamic", myPhysx::rigid::rdynamic)
            );

        registration::class_<myPhysx::LockingAxis>("Locking Axis")
            .property("X-axis", &myPhysx::LockingAxis::x_axis)
            .property("Y-axis", &myPhysx::LockingAxis::y_axis)
            .property("Z-axis", &myPhysx::LockingAxis::z_axis);
    }

    PhysicsMaterial RigidbodyComponent::GetMaterial() const 
    { 
        return underlying_object.material; 
    }
    
    vec3 oo::RigidbodyComponent::GetPositionInPhysicsWorld() const
    {
        auto res = underlying_object.position;
        return { res.x, res.y, res.z };
    }

    quat oo::RigidbodyComponent::GetOrientationInPhysicsWorld() const
    {
        return underlying_object.orientation;
    }

    //std::vector<physx::PxVec3> oo::RigidbodyComponent::StoreMesh(std::vector<oo::vec3> result) {

    //    /*std::vector<physx::PxVec3> vertices{ result.begin(), result.end() };
    //    
    //    underlying_object.storeMeshVertices(vertices);

    //    vertices = underlying_object.getAllMeshVertices();

    //    return vertices;*/
    //    
    //    // todo : this is obviously incorrect.
    //    IsDirty = true;
    //    std::vector<physx::PxVec3> vertices{ result.begin(), result.end() };
    //    underlying_object.meshVertices = vertices;
    //    vertices = underlying_object.meshVertices;
    //    return vertices;
    //}
    
    void oo::RigidbodyComponent::SetStatic(bool setToStatic)
    {
        if (setToStatic)
            desired_object.rigid_type = myPhysx::rigid::rstatic;
        else
            desired_object.rigid_type = myPhysx::rigid::rdynamic;
        
        IsDirty = true;
    }

    float oo::RigidbodyComponent::GetMass() const
    {
        if (IsStatic())
            return 0;

        return desired_object.mass;
        //return underlying_object.mass;
    }

    float oo::RigidbodyComponent::GetAngularDamping() const
    {
        if (IsStatic())
            return 0;

        return desired_object.angularDamping;
        //return underlying_object.angularDamping;
    }

    vec3 oo::RigidbodyComponent::GetAngularVelocity() const
    {
        if (IsStatic())
            return {0};

        return desired_object.angularVel;
        //return underlying_object.angularVel;
    }

    float oo::RigidbodyComponent::GetLinearDamping() const
    {
        if (IsStatic())
            return 0;

        return desired_object.linearDamping;
        //return underlying_object.linearDamping;
    }

    vec3 oo::RigidbodyComponent::GetLinearVelocity() const
    {
        if (IsStatic())
            return {0};

        return desired_object.linearVel;
        //return underlying_object.linearVel;
    }

    bool oo::RigidbodyComponent::IsGravityEnabled() const
    {
        return desired_object.gravity_enabled;
        //return underlying_object.gravity_enabled;
    }

    bool oo::RigidbodyComponent::IsGravityDisabled() const
    {
        return !IsGravityEnabled();
    }

    bool oo::RigidbodyComponent::IsStatic() const
    {
        return desired_object.rigid_type == myPhysx::rigid::rstatic;
        //return underlying_object.rigid_type == myPhysx::rigid::rstatic;
    }

    bool oo::RigidbodyComponent::IsKinematic() const
    {
        return !IsStatic() && desired_object.is_kinematic;
        //return !IsStatic() && underlying_object.is_kinematic;
    }

    bool oo::RigidbodyComponent::IsDynamic() const
    {
        return !IsStatic() && !desired_object.is_kinematic;
        //return !IsStatic() && !underlying_object.is_kinematic;
    }

    bool oo::RigidbodyComponent::IsTrigger() const
    {
        return desired_object.is_trigger;
        //return underlying_object.is_trigger;
    }

    bool oo::RigidbodyComponent::IsCollider() const
    {
        return !IsTrigger();
    }

    bool oo::RigidbodyComponent::VerticesChanged() const
    {
        return desired_object.changeVertices;
    }

    void oo::RigidbodyComponent::EnableCollider()
    {
        desired_object.is_collider = true;
        IsDirty = true;
    }

    void oo::RigidbodyComponent::DisableCollider()
    {
        desired_object.is_collider = false;
        IsDirty = true;
    }

    void oo::RigidbodyComponent::LockXAxisPos(bool enable)
    {
        desired_object.lockPositionAxis.x_axis = enable;
        IsDirty = true;
    }

    bool oo::RigidbodyComponent::IsXAxisPosLocked()
    {
        return desired_object.lockPositionAxis.x_axis;
        //return underlying_object.lockPositionAxis.x_axis;
    }

    void oo::RigidbodyComponent::LockYAxisPos(bool enable)
    {
        desired_object.lockPositionAxis.y_axis = enable;
        IsDirty = true;
    }

    bool oo::RigidbodyComponent::IsYAxisPosLocked()
    {
        return desired_object.lockPositionAxis.y_axis;
        //return underlying_object.lockPositionAxis.y_axis;
    }

    void oo::RigidbodyComponent::LockZAxisPos(bool enable)
    {
        desired_object.lockPositionAxis.z_axis = enable;
        IsDirty = true;
    }

    bool oo::RigidbodyComponent::IsZAxisPosLocked()
    {
        return desired_object.lockPositionAxis.z_axis;
        //return underlying_object.lockPositionAxis.z_axis;
    }

    void oo::RigidbodyComponent::LockXAxisRot(bool enable)
    {
        desired_object.lockRotationAxis.x_axis = enable;
        IsDirty = true;
    }

    bool oo::RigidbodyComponent::IsXAxisRotLocked()
    {
        return desired_object.lockRotationAxis.x_axis;
        //return underlying_object.lockRotationAxis.x_axis;
    }

    void oo::RigidbodyComponent::LockYAxisRot(bool enable)
    {
        desired_object.lockRotationAxis.y_axis = enable;
        IsDirty = true;
    }

    bool oo::RigidbodyComponent::IsYAxisRotLocked()
    {
        return desired_object.lockRotationAxis.y_axis;
        //return underlying_object.lockRotationAxis.y_axis;
    }
    
    void oo::RigidbodyComponent::LockZAxisRot(bool enable)
    {
        desired_object.lockRotationAxis.z_axis = enable;
        IsDirty = true;
    }

    bool oo::RigidbodyComponent::IsZAxisRotLocked()
    {
        return desired_object.lockRotationAxis.z_axis;
        //return underlying_object.lockRotationAxis.z_axis;
    }

    void oo::RigidbodyComponent::SetTrigger(bool enable)
    {
        desired_object.is_trigger = enable;
        IsDirty = true;
    }

    void RigidbodyComponent::SetMaterial(PhysicsMaterial material) 
    { 
        desired_object.material = material;
        IsDirty = true;
    }

    void RigidbodyComponent::SetPosOrientation(vec3 pos, quat quat) 
    { 
        //underlying_object.setPosOrientation(pos, quat); 
        
        //glm::quat quat_diff = orientation - glm::quat{ underlying_object.orientation.x, underlying_object.orientation.y, underlying_object.orientation.z, underlying_object.orientation.w };
        //if (glm::dot(quat_diff, quat_diff) > glm::epsilon<float>())
        {
            desired_object.position = { pos.x, pos.y, pos.z };
            desired_object.orientation = { quat.x, quat.y, quat.z, quat.w };
            IsDirty = true;
        }
    }
    
    void oo::RigidbodyComponent::SetGravity(bool enable)
    {
        // only applies to none static objects.
        desired_object.gravity_enabled = enable;
        IsDirty = true;
    }

    void oo::RigidbodyComponent::SetKinematic(bool kine) 
    { 
        desired_object.is_kinematic = kine;
        IsDirty = true;
    }

    void RigidbodyComponent::SetMass(float mass)
    {
        desired_object.mass = static_cast<PxReal>(mass);
        IsDirty = true;
    }

    void RigidbodyComponent::SetAngularDamping(float angularDamping)
    {
        desired_object.angularDamping = static_cast<PxReal>(angularDamping);
        IsDirty = true;
    }

    void RigidbodyComponent::SetAngularVelocity(vec3 angularVelocity)
    {
        desired_object.angularVel = PxVec3{ angularVelocity.x, angularVelocity.y, angularVelocity.z };
        IsDirty = true;
    }

    void RigidbodyComponent::SetLinearDamping(float linearDamping)
    {
        desired_object.linearDamping = static_cast<PxReal>(linearDamping);
        IsDirty = true;
    }

    void RigidbodyComponent::SetLinearVelocity(vec3 linearVelocity)
    {
        desired_object.linearVel = PxVec3{ linearVelocity.x, linearVelocity.y, linearVelocity.z };
        IsDirty = true;
    }

    void oo::RigidbodyComponent::AddForce(oo::vec3 force, ForceMode type)
    {
        myPhysx::PhysicsCommand cmd;
        cmd.Id = underlying_object.id;
        cmd.Force = PxVec3{ force.x, force.y, force.z };
        cmd.AddForce = true;
        switch (type)
        {
        case ForceMode::FORCE:
            cmd.Type = myPhysx::force::force;
            break;

        case ForceMode::ACCELERATION:
            cmd.Type = myPhysx::force::acceleration;
            break;

        case ForceMode::IMPULSE:
            cmd.Type = myPhysx::force::impulse;
            break;

        case ForceMode::VELOCITY_CHANGE:
            cmd.Type = myPhysx::force::velocityChanged;
            break;
        }

        external_commands.emplace_back(cmd);
    }

    void oo::RigidbodyComponent::AddTorque(vec3 force, ForceMode type)
    {
        myPhysx::PhysicsCommand cmd;
        cmd.Id = underlying_object.id;
        cmd.Force = PxVec3{ force.x, force.y, force.z };
        cmd.AddTorque = true;
        switch (type)
        {
        case ForceMode::FORCE:
            cmd.Type = myPhysx::force::force;
            break;

        case ForceMode::ACCELERATION:
            cmd.Type = myPhysx::force::acceleration;
            break;

        case ForceMode::IMPULSE:
            cmd.Type = myPhysx::force::impulse;
            break;

        case ForceMode::VELOCITY_CHANGE:
            cmd.Type = myPhysx::force::velocityChanged;
            break;
        }

        external_commands.emplace_back(cmd);
    }

    void oo::RigidbodyComponent::UploadVertices(std::vector<PxVec3> newVertices)
    {
        desired_object.uploadVertices = newVertices;
        IsDirty = true;
    }

    void oo::RigidbodyComponent::SetMeshScale(PxVec3 newScale)
    {
        desired_object.meshScale = newScale;
        IsDirty = true;
    }

    void oo::RigidbodyComponent::ForceDirty()
    {
        IsDirty = true;
    }

    oo::UUID oo::RigidbodyComponent::GetUnderlyingUUID() const
    {
        return oo::UUID{ std::uint64_t{underlying_object.id} };
    }

    glm::vec3 oo::RigidbodyComponent::GetLinearVel() const
    {
        return GetLinearVelocity();
    }

    glm::vec3 oo::RigidbodyComponent::GetAngularVel() const
    {
        return GetAngularVelocity();
    }

    void oo::RigidbodyComponent::SetOffset(glm::vec3 offset)
    {
        Offset = {offset};
        IsDirty = true;
    }

    glm::vec3 oo::RigidbodyComponent::GetOffset() const
    {
        return Offset;
    }

}