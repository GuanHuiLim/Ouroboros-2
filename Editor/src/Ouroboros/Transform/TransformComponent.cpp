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
        registration::class_<TransformComponent>("Transform Component")
            .property("Position", &TransformComponent::GetPosition, &TransformComponent::SetPosition)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Euler Angles", &TransformComponent::GetEulerAngles, &TransformComponent::SetRotation)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Quaternion", &TransformComponent::GetRotationQuat)
            .property("Scaling", &TransformComponent::GetScale, &TransformComponent::SetScale)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Local Matrix", &TransformComponent::GetLocalMatrix)
            .property_readonly("Global Matrix", &TransformComponent::GetGlobalMatrix)
            .property_readonly("Global Position", &TransformComponent::GetGlobalPosition)
            .property_readonly("Global Rotation", &TransformComponent::GetGlobalRotationDeg)
            .property_readonly("Global Quaternion", &TransformComponent::GetGlobalRotationQuat)
            .property_readonly("Global Scale", &TransformComponent::GetGlobalScale);
        //added readonly for debugging purposes
    }

    TransformComponent::vec3 TransformComponent::GetPosition()                const { return m_localTransform.Position; }
    TransformComponent::quat TransformComponent::GetRotationQuat()            const { return m_localTransform.Orientation; }
    TransformComponent::vec3 TransformComponent::GetEulerAngles()             const { return m_eulerAngles; }
    TransformComponent::vec3 TransformComponent::GetScale()                   const { return m_localTransform.Scale; }

    TransformComponent::vec3 TransformComponent::LocalRight()                 const { return m_localTransform.Right(); }
    TransformComponent::vec3 TransformComponent::LocalUp()                    const { return m_localTransform.Up(); }
    TransformComponent::vec3 TransformComponent::LocalForward()               const { return m_localTransform.Forward(); }
                                                
    TransformComponent::vec3 TransformComponent::GlobalRight()                const { return m_globalTransform.Right(); }
    TransformComponent::vec3 TransformComponent::GlobalUp()                   const { return m_globalTransform.Up(); }
    TransformComponent::vec3 TransformComponent::GlobalForward()              const { return m_globalTransform.Forward(); }
                                                
    TransformComponent::mat4 TransformComponent::GetLocalMatrix()             const { return m_localTransform.GetMatrix(); }
    TransformComponent::mat4 TransformComponent::GetGlobalMatrix()            const { return m_globalTransform.GetMatrix(); }

    bool TransformComponent::HasChanged()                                     const { return m_hasChanged; }
    bool TransformComponent::IsDirty()                                        const { return m_dirty; }

    TransformComponent::vec3 TransformComponent::GetGlobalPosition()          const { return m_globalTransform.Position; }
    TransformComponent::mat4 TransformComponent::GetGlobalRotationMatrix()    const { return m_globalTransform.GetRotationMatrix(); }
    TransformComponent::vec3 TransformComponent::GetGlobalRotationRad()       const { return m_globalTransform.GetEulerAnglesRad(); }
    TransformComponent::vec3 TransformComponent::GetGlobalRotationDeg()       const { return m_globalTransform.GetEulerAnglesDeg(); }
    TransformComponent::quat oo::TransformComponent::GetGlobalRotationQuat()  const { return m_globalTransform.Orientation; }
    TransformComponent::vec3 TransformComponent::GetGlobalScale()             const { return m_globalTransform.Scale; }

    TransformComponent::vec3& TransformComponent::Position()                  
    {
        m_dirty = true; 
        return m_localTransform.Position;
    }

    TransformComponent::vec3& TransformComponent::Scale()                     
    {
        m_dirty = true; 
        return m_localTransform.Scale; 
    }
    // note : scale must be set using setEulerAngle (internally uses quaternions)

    // Local Setters
    void TransformComponent::SetPosition(vec3 pos)                          
    {
        m_dirty = true; 
        m_localTransform.Position = pos;
    }
    
    void TransformComponent::SetRotation(vec3 euler_angles_degrees)         
    { 
        m_dirty = true; 
        m_eulerAngles = euler_angles_degrees;
        m_localTransform.SetRotation(quaternion::from_euler(glm::radians(m_eulerAngles)));
    }

    void oo::TransformComponent::SetOrientation(quat quaternion)
    {
        m_dirty = true;
        m_localTransform.SetRotation(quaternion);
        m_eulerAngles = m_localTransform.GetEulerAnglesDeg();
    }

    void TransformComponent::SetScale(vec3 scale)                           
    { 
        m_dirty = true; 
        m_localTransform.Scale = scale;
    }

    void TransformComponent::SetLocalTransform(mat4 target_local_matrix)
    {
        m_hasChanged = true;
        m_localTransform.SetTransform(target_local_matrix);
    }

    // Global Setters
    void TransformComponent::SetGlobalPosition(vec3 position)               
    {
        //m_dirty = true; 
        m_globalDirty = true;
        m_globalTransform.Position = position;
    }

    void TransformComponent::SetGlobalScale(vec3 scale)                     
    {
        //m_dirty = true; 
        m_globalDirty = true;
        m_globalTransform.Scale = scale;
    }

    void TransformComponent::SetGlobalRotation(vec3 euler_angles_degrees)   
    { 
        //m_dirty = true; 
        m_globalDirty = true;
        m_globalTransform.SetRotation(quaternion::from_euler(glm::radians(euler_angles_degrees))); 
    }

    void TransformComponent::SetGlobalOrientation(quat quaternion)
    {
        //m_dirty = true;
        m_globalDirty = true;
        m_globalTransform.SetRotation(quaternion);
    }

    void TransformComponent::SetGlobalTransform(vec3 position, vec3 euler_angles_degrees, vec3 scale) 
    {
        SetGlobalPosition(position);
        SetGlobalRotation(euler_angles_degrees);
        SetGlobalScale(scale);

        //m_dirty = true;
        //m_globalTransform.SetLocalTransform(position, euler_angles_degrees, scale); 
    }
    
    void TransformComponent::ParentChanged() { /*m_dirty = true;*/ m_globalDirty = true; }

    void oo::TransformComponent::SetHasChanged(bool value)
    {
        m_hasChanged = value;
    }

    void TransformComponent::SetGlobalTransform(mat4 target_global_matrix)
    {
        //m_dirty = true;
        m_globalDirty = true;
        m_globalTransform.SetTransform(target_global_matrix);
        //m_hasChanged = true;
        //m_localTransform.SetGlobalTransform(target_global_matrix);
    }

    void oo::TransformComponent::LookAt(vec3 target)
    {
        m_dirty = true;
        m_localTransform.LookAt(target);
    }

    void TransformComponent::CalculateLocalTransform()
    {
        m_dirty = false;
        m_hasChanged = true;
        m_localTransform.CalculateTransform();
    }

    void oo::TransformComponent::CalculateGlobalTransform()
    {
        m_globalDirty = false;
        m_hasChanged = true;
        m_globalTransform.CalculateTransform();
    }
}
