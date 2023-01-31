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

namespace oo
{
    // Data Types
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;
    using mat2 = glm::mat2;
    using mat3 = glm::mat3;
    using quat = glm::quat;

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
    struct MeshColliderComponent;
    //struct ConvexCollider;
}
