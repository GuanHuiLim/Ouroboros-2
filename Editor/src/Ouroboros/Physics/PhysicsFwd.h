/************************************************************************************//*!
\file           PhysicsFwd.h
\project        Ouroboros 2
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 2, 2022
\brief          Forward Declaration of all the various types that are used in the
                Physics Engine

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <glm/glm.hpp>
#include "Physics/Source/phy.h"
#include "Ouroboros/Transform/TransformComponent.h"

namespace oo
{
    // Data Types
    
    // we wrap our own class so implicit conversation is possible
    struct vec3
    {
        float x, y, z;
        
        vec3() = default;
        vec3& operator=(vec3 const&) = default;
        glm::vec3& operator=(glm::vec3 const& other) const { return *this = static_cast<oo::vec3>(other); }

        vec3(float v) : x{ v }, y{ v }, z{ v }{};
        vec3(float x, float y, float z) : x{ x }, y{ y }, z{ z } {};
        //vec3(glm::vec3 in) : x{ in.x }, y{ in.y }, z{ in.z } {};
        vec3(physx::PxVec3 in) : x{ in.x }, y{ in.y }, z{ in.z } {};
        vec3(glm::vec4 in) :x{ in.x }, y{ in.y }, z{ in.z } {};
        vec3(oo::TransformComponent::vec3 in) : x{ in.x }, y{ in.y }, z{ in.z } {};

        operator glm::vec3() const { return glm::vec3{ x, y, z }; }
        operator physx::PxVec3() const { return physx::PxVec3{ x,y,z }; }

        glm::vec3 operator-(oo::vec3 other) const { return static_cast<glm::vec3>(*this) - static_cast<glm::vec3>(other); }

        glm::vec3 operator-(glm::vec3 other) const { return static_cast<glm::vec3>(*this) - other; }
        glm::vec3 operator+(glm::vec3 other) const { return static_cast<glm::vec3>(*this) + other; }
        
        friend glm::vec3 operator-(glm::vec3 other, oo::vec3 target) { return other - static_cast<glm::vec3>(target); }
    };

    struct quat
    {
        float x, y, z, w;

        quat() = default;
        quat(TransformComponent::quat v) : x{ v.value.x }, y{ v.value.y }, z{ v.value.z }, w{ v.value.w }{};
        quat(glm::quat v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w }{};
        quat(physx::PxQuat v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w }{};

        operator glm::quat() const { return glm::quat{ w, x, y, z }; }
        operator physx::PxQuat() const { return physx::PxQuat{ x, y, z, w }; }
    };

    //using vec3 = glm::vec3;
    //using mat3 = glm::mat3;
    //using quat = glm::quat;

    class RigidbodyComponent;
    //class ColliderCore;

    // Broadphase
    //struct SortSweepCompare;

    // Colliders
    //struct BoundingVolume;    // broadphase AABB-Collider
    struct ColliderBase;
    struct BoxColliderComponent;
    struct SphereColliderComponent;
    struct CapsuleColliderComponent;
    struct ConvexColliderComponent;
    //struct ConvexCollider;

    // Data Structures
    /*struct Contact2D;
    struct Manifold2D;
    struct PhysicsMaterial;
    struct MassData;*/

    //// Solvers
    //struct Solver;
    //struct ImpulseSolver;
    //struct SmoothPositionSolver;
}
