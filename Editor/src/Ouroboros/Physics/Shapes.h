/************************************************************************************//*!
\file           Shapes.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 2, 2021
\brief          Shapes contains the basic data that support the fundamental piece of
                geometry used in physics engine.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Physics/PhysicsFwd.h"
#include <rttr/type>

namespace oo
{
    struct Point
    {
        vec3 pos;
    };

    struct Sphere
    {
        vec3 center;
        float radius = 0.f;
        
        Sphere() = default;
        Sphere(vec3, float);

        RTTR_ENABLE();
    };

    struct AABB
    {
        vec3 min, max;
        
        AABB() = default;
        AABB(vec3, vec3);

        RTTR_ENABLE();
    };

}