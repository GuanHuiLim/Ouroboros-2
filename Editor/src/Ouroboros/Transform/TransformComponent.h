/************************************************************************************//*!
\file           TransformComponent.h
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
#pragma once

#include <Quaternion/include/Transform.h>

#include <rttr/type>

namespace oo
{
    class TransformComponent final
    {
    public:
        using vec3 = Transform3D::vec3;
        using mat4 = Transform3D::mat4;
        using quat = Transform3D::quat;

        /*-----------------------------------------------------------------------------*/
        /* Getter Functions                                                            */
        /*-----------------------------------------------------------------------------*/
        vec3 GetPosition()     const;
        quat GetRotationQuat() const;
        vec3 GetEulerAngles()  const;
        vec3 GetScale()        const;

        vec3 LocalRight()      const;
        vec3 LocalUp()         const;
        vec3 LocalForward()    const;

        vec3 GlobalRight()     const;
        vec3 GlobalUp()        const;
        vec3 GlobalForward()   const;

        //mat4 GetLocalMatrix()  const;
        //mat4 GetGlobalMatrix() const;

        vec3 GetGlobalPosition()        const;
        mat4 GetGlobalRotationMatrix()  const;
        vec3 GetGlobalRotationRad()     const;  // return vec3 for rotation of each component (euler angles)
        vec3 GetGlobalRotationDeg()     const;  // return vec3 for rotation of each component (euler angles)
        quat GetGlobalRotationQuat()    const;
        vec3 GetGlobalScale()           const;

        /*-----------------------------------------------------------------------------*/
        /* Setter Functions                                                            */
        /*-----------------------------------------------------------------------------*/
        vec3& Position();
        vec3& Scale();
        // note : rotation must be set using setEulerAngle (internally uses quaternions)

        // Local Setters
        void SetPosition(vec3 pos);
        void SetRotation(vec3 euler_angles_degrees);
        void SetOrientation(quat quaternion);
        void SetScale(vec3 scale);
        void SetLocalTransform(mat4 target_local_matrix);

        // Global Setters
        void SetGlobalPosition(vec3 position);
        void SetGlobalScale(vec3 scale);
        void SetGlobalRotation(vec3 euler_angles_degrees);
        void SetGlobalOrientation(quat quaternion);
        void SetGlobalTransform(vec3 position, vec3 euler_angles_degrees, vec3 scale);
        void SetGlobalTransform(vec3 position, quat quaternion, vec3 scale);
        void SetGlobalTransform(mat4 target_global_matrix);

        // Extra Functions
        void LookAt(vec3 target);

        void CalculateLocalTransform();
        void CalculateGlobalTransform();

        RTTR_ENABLE();
    public:

        Transform3D LocalTransform;
        Transform3D GlobalTransform;

        bool LocalMatrixDirty = false;
        bool GlobalMatrixDirty = false;
        bool HasChangedThisFrame = false;

        glm::vec3 LocalEulerAngles;    // fake data.
    };

    static constexpr std::size_t transform_component_size = sizeof(TransformComponent);
}
