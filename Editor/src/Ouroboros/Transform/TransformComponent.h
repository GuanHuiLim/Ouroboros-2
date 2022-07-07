#pragma once

#include <Transform.h>

namespace oo
{
    class Transform3D
    {
    public:
        using vec3 = Transform::vec3;
        using mat4 = Transform::mat4;
        using quat = Transform::quat;

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

        mat4 GetLocalMatrix()  const;
        mat4 GetGlobalMatrix() const;

        bool HasChanged()      const;
        bool IsDirty()         const;

        vec3 GetGlobalPosition()        const;
        mat4 GetGlobalRotationMatrix()  const;
        vec3 GetGlobalRotationRad()     const;  // return vec3 for rotation of each component (euler angles)
        vec3 GetGlobalRotationDeg()     const;  // return vec3 for rotation of each component (euler angles)
        vec3 GetGlobalScale()           const;

        /*-----------------------------------------------------------------------------*/
        /* Setter Functions                                                            */
        /*-----------------------------------------------------------------------------*/
        vec3& Position();
        vec3& Scale();
        // note : scale must be set using setEulerAngle (internally uses quaternions)

        // Local Setters
        void SetPosition(vec3 const& pos);
        void SetEulerAngles(vec3 const& eulerAngle);
        void SetScale(vec3 const& scale);

        // Global Setters
        void SetGlobalPosition(vec3 const& position);
        void SetGlobalScale(vec3 const& scale);
        void SetGlobalAngle(vec3 const& euler_angles);
        void SetGlobalTransform(vec3 const& position, vec3 const& euler_angles, vec3 const& scale);
        //void SetGlobalTransform(glm::mat4 const& targetGlobalTransform);

    private:
        Transform m_transform;
    };
}
