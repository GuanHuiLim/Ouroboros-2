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
}
