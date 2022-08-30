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
        registration::class_<TransformComponent>("TransformComponent")
            .property("Position", &TransformComponent::GetPosition, &TransformComponent::SetPosition)
            .property("Euler Angles", &TransformComponent::GetEulerAngles, &TransformComponent::SetEulerAngles)
            .property_readonly("Quaternion", &TransformComponent::GetRotationQuat)
            .property("Scaling", &TransformComponent::GetScale, &TransformComponent::SetScale)
            .property_readonly("Local Matrix", &TransformComponent::GetLocalMatrix)
            .property_readonly("Global Matrix", &TransformComponent::GetGlobalMatrix)
            .property_readonly("Global Rotation", &TransformComponent::GetGlobalRotationDeg)
            .property_readonly("Global Scale", &TransformComponent::GetGlobalScale); //added readonly for debugging purposes
    }

    TransformComponent::vec3 TransformComponent::GetPosition()                const { return m_transform.GetPosition(); }
    TransformComponent::quat TransformComponent::GetRotationQuat()            const { return m_transform.GetRotationQuat(); }
    TransformComponent::vec3 TransformComponent::GetEulerAngles()             const { return m_transform.GetEulerAngles(); }
    TransformComponent::vec3 TransformComponent::GetScale()                   const { return m_transform.GetScale(); }

    TransformComponent::vec3 TransformComponent::LocalRight()                 const { return m_transform.LocalRight(); }
    TransformComponent::vec3 TransformComponent::LocalUp()                    const { return m_transform.LocalUp(); }
    TransformComponent::vec3 TransformComponent::LocalForward()               const { return m_transform.LocalForward(); }
                                                
    TransformComponent::vec3 TransformComponent::GlobalRight()                const { return m_transform.GlobalRight(); }
    TransformComponent::vec3 TransformComponent::GlobalUp()                   const { return m_transform.GlobalUp(); }
    TransformComponent::vec3 TransformComponent::GlobalForward()              const { return m_transform.GlobalForward(); }
                                                
    TransformComponent::mat4 TransformComponent::GetLocalMatrix()             const { return m_transform.GetLocalMatrix(); }
    TransformComponent::mat4 TransformComponent::GetGlobalMatrix()            const { return m_transform.GetGlobalMatrix(); }

    bool TransformComponent::HasChanged()                              const { return m_transform.HasChanged(); }
    bool TransformComponent::IsDirty()                                 const { return m_transform.IsDirty(); }

    TransformComponent::vec3 TransformComponent::GetGlobalPosition()          const { return m_transform.GetGlobalPosition(); }
    TransformComponent::mat4 TransformComponent::GetGlobalRotationMatrix()    const { return m_transform.GetGlobalRotationMatrix(); }
    TransformComponent::vec3 TransformComponent::GetGlobalRotationRad()       const { return m_transform.GetGlobalRotationRad(); }  
    TransformComponent::vec3 TransformComponent::GetGlobalRotationDeg()       const { return m_transform.GetGlobalRotationDeg(); }  
    TransformComponent::vec3 TransformComponent::GetGlobalScale()             const { return m_transform.GetGlobalScale(); }

    TransformComponent::vec3& TransformComponent::Position()                  { return m_transform.Position(); }
    TransformComponent::vec3& TransformComponent::Scale()                     { return m_transform.Scale(); }
    // note : scale must be set using setEulerAngle (internally uses quaternions)

    // Local Setters
    void TransformComponent::SetPosition(vec3 pos)              { m_transform.SetPosition(pos); }
    void TransformComponent::SetEulerAngles(vec3 eulerAngle)    { m_transform.SetEulerAngles(eulerAngle); }
    void TransformComponent::SetScale(vec3 scale)               { m_transform.SetScale(scale);}

    // Global Setters
    void TransformComponent::SetGlobalPosition(vec3 position)   { m_transform.SetGlobalPosition(position); }
    void TransformComponent::SetGlobalScale(vec3 scale)         { m_transform.SetGlobalScale(scale); }
    void TransformComponent::SetGlobalAngle(vec3 euler_angles)  { m_transform.SetGlobalAngle(euler_angles); }
    void TransformComponent::SetGlobalTransform(vec3 position, vec3 euler_angles, vec3 scale) { m_transform.SetGlobalTransform(position, euler_angles, scale); }
    
    void TransformComponent::ParentChanged()
    {
        m_transform.m_dirty = true;
    }

    //void SetGlobalTransform(glm::mat4 const& targetGlobalTransform) { }

}
