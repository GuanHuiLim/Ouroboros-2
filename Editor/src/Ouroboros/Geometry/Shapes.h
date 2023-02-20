/************************************************************************************//*!
\file           Shapes.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 2, 2022
\brief          Shapes contains the basic data that support the fundamental piece of
                geometry used in physics engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace oo
{
    struct BoundingBox
    {
        glm::vec3 Center = glm::vec3{ 0 };
        glm::vec3 HalfExtents = glm::vec3{ 0 };
    };

    struct OrientedBoundingBox
    {
        glm::vec3 Center = glm::vec3{ 0 };
        glm::vec3 HalfExtents = glm::vec3{ 0 };
        glm::quat Orientation = glm::quat{ 0, 0, 0, 1 };
    };

    struct Ray
    {
        glm::vec3 Position = { 0, 0, 0 };
        glm::vec3 Direction = { 0, 0, 0 };
    };

    struct Sphere
    {
        glm::vec3 center;
        float radius = 0.f;

        Sphere() = default;
        Sphere(glm::vec3, float);

        RTTR_ENABLE();
    };

    struct AABB
    {
        glm::vec3 min, max;

        AABB() = default;
        AABB(glm::vec3, glm::vec3);

        RTTR_ENABLE();
    };
}
