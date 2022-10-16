/************************************************************************************//*!
\file           TransformComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 31, 2022
\brief          Defines the data and functions required to allow for operations
                to move objects around in a define 3D space

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "TransformComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration>
namespace oo
{
    /********************************************************************************//*!
     @brief     Used to register for RTTR Specifically to display the relevant
                information that will be displayed to the editor
    *//*********************************************************************************/
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<TransformComponent>("Transform Component")(metadata(UI_metadata::NOT_REMOVABLE,true))
            .property("Position", &TransformComponent::GetPosition, &TransformComponent::SetPosition)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Euler Angles", &TransformComponent::GetEulerAngles, &TransformComponent::SetRotation)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Quaternion", &TransformComponent::GetRotationQuat, &TransformComponent::SetOrientation)(metadata(UI_metadata::HIDDEN, true))
            .property("Scaling", &TransformComponent::GetScale, &TransformComponent::SetScale)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Local Matrix", &TransformComponent::GetLocalMatrix)
            .property_readonly("Global Matrix", &TransformComponent::GetGlobalMatrix)
            .property_readonly("Global Position", &TransformComponent::GetGlobalPosition)
            .property_readonly("Global Rotation", &TransformComponent::GetGlobalRotationDeg)
            .property_readonly("Global Quaternion", &TransformComponent::GetGlobalRotationQuat)
            .property_readonly("Global Scale", &TransformComponent::GetGlobalScale);
        //added readonly for debugging purposes
    }

    TransformComponent::vec3 TransformComponent::GetPosition()                const { return LocalTransform.Position; }
    TransformComponent::quat TransformComponent::GetRotationQuat()            const { return LocalTransform.Orientation; }
    TransformComponent::vec3 TransformComponent::GetEulerAngles()             const { return LocalEulerAngles; }
    TransformComponent::vec3 TransformComponent::GetScale()                   const { return LocalTransform.Scale; }

    TransformComponent::vec3 TransformComponent::LocalRight()                 const { return LocalTransform.Right(); }
    TransformComponent::vec3 TransformComponent::LocalUp()                    const { return LocalTransform.Up(); }
    TransformComponent::vec3 TransformComponent::LocalForward()               const { return LocalTransform.Forward(); }
                                                
    TransformComponent::vec3 TransformComponent::GlobalRight()                const { return GlobalTransform.Right(); }
    TransformComponent::vec3 TransformComponent::GlobalUp()                   const { return GlobalTransform.Up(); }
    TransformComponent::vec3 TransformComponent::GlobalForward()              const { return GlobalTransform.Forward(); }
                                                
    TransformComponent::mat4 TransformComponent::GetLocalMatrix()             const { return LocalTransform.GetMatrix(); }
    TransformComponent::mat4 TransformComponent::GetGlobalMatrix()            const { return GlobalTransform.GetMatrix(); }

    TransformComponent::vec3 TransformComponent::GetGlobalPosition()          const { return GlobalTransform.Position; }
    TransformComponent::mat4 TransformComponent::GetGlobalRotationMatrix()    const { return GlobalTransform.GetRotationMatrix(); }
    TransformComponent::vec3 TransformComponent::GetGlobalRotationRad()       const { return GlobalTransform.GetEulerAnglesRad(); }
    TransformComponent::vec3 TransformComponent::GetGlobalRotationDeg()       const { return GlobalTransform.GetEulerAnglesDeg(); }
    TransformComponent::quat oo::TransformComponent::GetGlobalRotationQuat()  const { return GlobalTransform.Orientation; }
    TransformComponent::vec3 TransformComponent::GetGlobalScale()             const { return GlobalTransform.Scale; }

    TransformComponent::vec3& TransformComponent::Position()                  
    {
        LocalMatrixDirty = true; 
        return LocalTransform.Position;
    }

    TransformComponent::vec3& TransformComponent::Scale()                     
    {
        LocalMatrixDirty = true; 
        return LocalTransform.Scale; 
    }
    // note : scale must be set using setEulerAngle (internally uses quaternions)

    // Local Setters
    void TransformComponent::SetPosition(vec3 pos)                          
    {
        LocalMatrixDirty = true; 
        LocalTransform.Position = pos;
    }
    
    void TransformComponent::SetRotation(vec3 euler_angles_degrees)         
    { 
        LocalMatrixDirty = true; 
        LocalEulerAngles = euler_angles_degrees;
        LocalTransform.SetRotation(quaternion::from_euler(glm::radians(LocalEulerAngles)));
    }

    void oo::TransformComponent::SetOrientation(quat quaternion)
    {
        LocalMatrixDirty = true;
        LocalTransform.SetRotation(quaternion);
        LocalEulerAngles = LocalTransform.GetEulerAnglesDeg();
    }

    void TransformComponent::SetScale(vec3 scale)                           
    { 
        LocalMatrixDirty = true; 
        LocalTransform.Scale = scale;
    }

    void TransformComponent::SetLocalTransform(mat4 target_local_matrix)
    {
        LocalMatrixDirty = false;
        HasChangedThisFrame = true;
        LocalTransform.SetTransform(target_local_matrix);
    }

    // Global Setters
    void TransformComponent::SetGlobalPosition(vec3 position)               
    {
        GlobalMatrixDirty = true;
        GlobalTransform.Position = position;
    }

    void TransformComponent::SetGlobalScale(vec3 scale)                     
    {
        GlobalMatrixDirty = true;
        GlobalTransform.Scale = scale;
    }

    void TransformComponent::SetGlobalRotation(vec3 euler_angles_degrees)   
    { 
        //LocalMatrixDirty = true; 
        GlobalMatrixDirty = true;
        GlobalTransform.SetRotation(quaternion::from_euler(glm::radians(euler_angles_degrees))); 
    }

    void TransformComponent::SetGlobalOrientation(quat quaternion)
    {
        //LocalMatrixDirty = true;
        GlobalMatrixDirty = true;
        GlobalTransform.SetRotation(quaternion);
    }

    void TransformComponent::SetGlobalTransform(vec3 position, vec3 euler_angles_degrees, vec3 scale) 
    {
        SetGlobalPosition(position);
        SetGlobalRotation(euler_angles_degrees);
        SetGlobalScale(scale);
    }

    void oo::TransformComponent::SetGlobalTransform(vec3 position, quat quaternion, vec3 scale)
    {
        SetGlobalPosition(position);
        SetGlobalOrientation(quaternion);
        SetGlobalScale(scale);
    }

    void TransformComponent::SetGlobalTransform(mat4 target_global_matrix)
    {
        GlobalMatrixDirty = true;
        GlobalTransform.SetTransform(target_global_matrix);
    }

    void oo::TransformComponent::LookAt(vec3 target)
    {
        LocalMatrixDirty = true;
        LocalTransform.LookAt(target);
    }

    void TransformComponent::CalculateLocalTransform()
    {
        LocalMatrixDirty = false;
        HasChangedThisFrame = true;
        LocalTransform.CalculateTransform();
    }

    void oo::TransformComponent::CalculateGlobalTransform()
    {
        GlobalMatrixDirty = false;
        HasChangedThisFrame = true;
        GlobalTransform.CalculateTransform();
    }
}
