/************************************************************************************//*!
\file           TransformComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 31, 2022
\brief          Defines the data and functions required to allow for operations
                to move objects around in a define 3D space

\note           Name of component TransformComponent

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "TransformComponent.h"

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
            .property("Position", &TransformComponent::GetPosition, &TransformComponent::SetPosition)
            .property("Euler Angles", &TransformComponent::GetEulerAngles, &TransformComponent::SetRotation)
            .property_readonly("Quaternion", &TransformComponent::GetRotationQuat)
            .property("Scaling", &TransformComponent::GetScale, &TransformComponent::SetScale)
            .property_readonly("Local Matrix", &TransformComponent::GetLocalMatrix)
            .property_readonly("Global Matrix", &TransformComponent::GetGlobalMatrix)
            .property("Global Position", &TransformComponent::GetGlobalPosition, &TransformComponent::SetGlobalPosition)
            .property("Global Rotation", &TransformComponent::GetGlobalRotationDeg, &TransformComponent::SetGlobalRotation)
            .property_readonly("Global Quaternion", &TransformComponent::GetGlobalRotationQuat)
            .property("Global Scale", &TransformComponent::GetGlobalScale, &TransformComponent::SetGlobalScale);
        //added readonly for debugging purposes
    }

    TransformComponent::vec3 TransformComponent::GetPosition()                const { return m_transform.GetPosition(); }
    TransformComponent::quat TransformComponent::GetRotationQuat()            const { return m_transform.GetRotationQuat(); }
    TransformComponent::vec3 TransformComponent::GetEulerAngles()             const { return m_eulerAngles; }
    TransformComponent::vec3 TransformComponent::GetScale()                   const { return m_transform.GetScale(); }

    TransformComponent::vec3 TransformComponent::LocalRight()                 const { return m_transform.LocalRight(); }
    TransformComponent::vec3 TransformComponent::LocalUp()                    const { return m_transform.LocalUp(); }
    TransformComponent::vec3 TransformComponent::LocalForward()               const { return m_transform.LocalForward(); }
                                                
    TransformComponent::vec3 TransformComponent::GlobalRight()                const { return m_transform.GlobalRight(); }
    TransformComponent::vec3 TransformComponent::GlobalUp()                   const { return m_transform.GlobalUp(); }
    TransformComponent::vec3 TransformComponent::GlobalForward()              const { return m_transform.GlobalForward(); }
                                                
    TransformComponent::mat4 TransformComponent::GetLocalMatrix()             const { return m_transform.GetLocalMatrix(); }
    TransformComponent::mat4 TransformComponent::GetGlobalMatrix()            const { return m_transform.GetGlobalMatrix(); }

    bool TransformComponent::HasChanged()                                     const { return m_hasChanged; }
    bool TransformComponent::IsDirty()                                        const { return m_dirty; }

    TransformComponent::vec3 TransformComponent::GetGlobalPosition()          const { return m_transform.GetGlobalPosition(); }
    TransformComponent::mat4 TransformComponent::GetGlobalRotationMatrix()    const { return m_transform.GetGlobalRotationMatrix(); }
    TransformComponent::vec3 TransformComponent::GetGlobalRotationRad()       const { return m_transform.GetGlobalRotationRad(); }  
    TransformComponent::vec3 TransformComponent::GetGlobalRotationDeg()       const { return m_transform.GetGlobalRotationDeg(); }
    TransformComponent::quat oo::TransformComponent::GetGlobalRotationQuat()  const { return m_transform.GetGlobalRotationQuat(); }
    TransformComponent::vec3 TransformComponent::GetGlobalScale()             const { return m_transform.GetGlobalScale(); }

    TransformComponent::vec3& TransformComponent::Position()                  { m_dirty = true; return m_transform.Position(); }
    TransformComponent::vec3& TransformComponent::Scale()                     { m_dirty = true; return m_transform.Scale(); }
    // note : scale must be set using setEulerAngle (internally uses quaternions)

    // Local Setters
    void TransformComponent::SetPosition(vec3 pos)                          { m_dirty = true; m_transform.SetPosition(pos); }
    
    void TransformComponent::SetRotation(vec3 euler_angles_degrees)         
    { 
        m_dirty = true; 
        m_eulerAngles = euler_angles_degrees;
        m_transform.SetRotation(quaternion::from_euler(glm::radians(m_eulerAngles)));
    }

    void oo::TransformComponent::SetOrientation(quat quaternion)
    {
        m_dirty = true;
        m_transform.SetRotation(quaternion);
        m_eulerAngles = m_transform.GetEulerAngles();
    }

    void TransformComponent::SetScale(vec3 scale)                           { m_dirty = true; m_transform.SetScale(scale);}

    // Global Setters
    void TransformComponent::SetGlobalPosition(vec3 position)               { m_dirty = true; m_transform.SetGlobalPosition(position); }
    void TransformComponent::SetGlobalScale(vec3 scale)                     { m_dirty = true; m_transform.SetGlobalScale(scale); }
    void TransformComponent::SetGlobalRotation(vec3 euler_angles_degrees)   
    { 
        m_dirty = true; 
        m_transform.SetGlobalRotation(quaternion::from_euler(glm::radians(euler_angles_degrees))); 
    }

    void TransformComponent::SetGlobalOrientation(quat quaternion)
    {
        m_dirty = true;
        m_transform.SetGlobalRotation(quaternion);
    }

    void TransformComponent::SetGlobalTransform(vec3 position, vec3 euler_angles_degrees, vec3 scale) 
    { 
        m_dirty = true;
        m_transform.SetGlobalTransform(position, euler_angles_degrees, scale); 
    }
    
    void TransformComponent::ParentChanged() { m_dirty = true; }

    void TransformComponent::SetGlobalTransform(mat4 target_global_matrix)
    {
        m_hasChanged = true;
        m_transform.SetGlobalTransform(target_global_matrix);
    }

    void TransformComponent::CalculateLocalTransform()
    {
        m_dirty = false;
        m_hasChanged = true;
        m_transform.CalculateLocalTransform();
    }
}
