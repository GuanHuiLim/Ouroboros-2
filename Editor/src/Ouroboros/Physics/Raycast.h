#pragma once
#include <glm/glm.hpp>
#include "Utility/UUID.h"
#include "Ouroboros/EventSystem/Event.h"

namespace oo
{
    struct RaycastResult
    {
        bool Intersect = false;
        oo::UUID UUID = oo::UUID::Invalid;
        glm::vec3 Position = glm::vec3{ 0 };
        glm::vec3 Normal = glm::vec3{ 0 };
        float Distance = 0;
    };

    struct RaycastAllEvent : public oo::Event
    {
        Ray ray;
        float distance = std::numeric_limits<float>::max();

        std::vector<RaycastResult> Results;
    };
    
    struct RaycastEvent : public oo::Event
    {
        Ray ray;
        float distance = std::numeric_limits<float>::max();

        RaycastResult Results;
    };
}