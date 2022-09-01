/************************************************************************************//*!
\file           TransformComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 31, 2022
\brief          Defines the data and functions required to allow for operations
                to move objects around in a define 3D space

\note           Name of component Transform3D

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
        registration::class_<Transform3D>("Transform3D")
            .property("Position", &Transform3D::GetPosition, &Transform3D::SetPosition)
            .property("Euler Angles", &Transform3D::GetEulerAngles, &Transform3D::SetEulerAngles)
            .property_readonly("Quaternion", &Transform3D::GetRotationQuat)
            .property("Scaling", &Transform3D::GetScale, &Transform3D::SetScale)
            .property_readonly("Local Matrix", &Transform3D::GetLocalMatrix)
            .property_readonly("Global Matrix", &Transform3D::GetGlobalMatrix)
            .property_readonly("Global Rotation", &Transform3D::GetGlobalRotationDeg)
            .property_readonly("Global Scale", &Transform3D::GetGlobalScale); //added readonly for debugging purposes
    }

    Transform3D::vec3 Transform3D::GetPosition()                const { return m_transform.GetPosition(); }
    Transform3D::quat Transform3D::GetRotationQuat()            const { return m_transform.GetRotationQuat(); }
    Transform3D::vec3 Transform3D::GetEulerAngles()             const { return m_transform.GetEulerAngles(); }
    Transform3D::vec3 Transform3D::GetScale()                   const { return m_transform.GetScale(); }

    Transform3D::vec3 Transform3D::LocalRight()                 const { return m_transform.LocalRight(); }
    Transform3D::vec3 Transform3D::LocalUp()                    const { return m_transform.LocalUp(); }
    Transform3D::vec3 Transform3D::LocalForward()               const { return m_transform.LocalForward(); }
                                                
    Transform3D::vec3 Transform3D::GlobalRight()                const { return m_transform.GlobalRight(); }
    Transform3D::vec3 Transform3D::GlobalUp()                   const { return m_transform.GlobalUp(); }
    Transform3D::vec3 Transform3D::GlobalForward()              const { return m_transform.GlobalForward(); }
                                                
    Transform3D::mat4 Transform3D::GetLocalMatrix()             const { return m_transform.GetLocalMatrix(); }
    Transform3D::mat4 Transform3D::GetGlobalMatrix()            const { return m_transform.GetGlobalMatrix(); }

    bool Transform3D::HasChanged()                              const { return m_transform.HasChanged(); }
    bool Transform3D::IsDirty()                                 const { return m_transform.IsDirty(); }

    Transform3D::vec3 Transform3D::GetGlobalPosition()          const { return m_transform.GetGlobalPosition(); }
    Transform3D::mat4 Transform3D::GetGlobalRotationMatrix()    const { return m_transform.GetGlobalRotationMatrix(); }
    Transform3D::vec3 Transform3D::GetGlobalRotationRad()       const { return m_transform.GetGlobalRotationRad(); }  
    Transform3D::vec3 Transform3D::GetGlobalRotationDeg()       const { return m_transform.GetGlobalRotationDeg(); }  
    Transform3D::vec3 Transform3D::GetGlobalScale()             const { return m_transform.GetGlobalScale(); }

    Transform3D::vec3& Transform3D::Position()                  { return m_transform.Position(); }
    Transform3D::vec3& Transform3D::Scale()                     { return m_transform.Scale(); }
    // note : scale must be set using setEulerAngle (internally uses quaternions)

    // Local Setters
    void Transform3D::SetPosition(vec3 pos)              { m_transform.SetPosition(pos); }
    void Transform3D::SetEulerAngles(vec3 eulerAngle)    { m_transform.SetEulerAngles(eulerAngle); }
    void Transform3D::SetScale(vec3 scale)               { m_transform.SetScale(scale);}

    // Global Setters
    void Transform3D::SetGlobalPosition(vec3 position)   { m_transform.SetGlobalPosition(position); }
    void Transform3D::SetGlobalScale(vec3 scale)         { m_transform.SetGlobalScale(scale); }
    void Transform3D::SetGlobalAngle(vec3 euler_angles)  { m_transform.SetGlobalAngle(euler_angles); }
    void Transform3D::SetGlobalTransform(vec3 position, vec3 euler_angles, vec3 scale) { m_transform.SetGlobalTransform(position, euler_angles, scale); }
    
    void Transform3D::ParentChanged()
    {
        m_transform.m_dirty = true;
    }

    //void SetGlobalTransform(glm::mat4 const& targetGlobalTransform) { }

}
