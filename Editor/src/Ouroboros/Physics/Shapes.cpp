/************************************************************************************//*!
\file           Shapes.cpp
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
#include "pch.h"

#include "Shapes.h"

#include <rttr/registration>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::class_<AABB>("AABB")
            .property_readonly("Min", &AABB::min)
            .property_readonly("Max", &AABB::max);

        registration::class_<Sphere>("Sphere")
            .property_readonly("Center", &Sphere::center)
            .property_readonly("Radius", &Sphere::radius);
    }
    
    AABB::AABB(vec3 min, vec3 max)
        : min{ min }
        , max{ max }
    {
    }

    Sphere::Sphere(vec3 center, float radius)
        : center{ center }
        , radius{ radius }
    {
    }
}
