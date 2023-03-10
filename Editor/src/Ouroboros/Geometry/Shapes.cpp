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

        registration::class_<oo::AABB>("AABB")
            .property_readonly("Min", &oo::AABB::min)
            .property_readonly("Max", &oo::AABB::max);

        registration::class_<Sphere>("Sphere")
            .property_readonly("Center", &oo::Sphere::center)
            .property_readonly("Radius", &oo::Sphere::radius);
        
        registration::class_<OrientedBoundingBox>("OBB")
            .property_readonly("Center", &oo::OrientedBoundingBox::Center)
            .property_readonly("Half Extents", &oo::OrientedBoundingBox::HalfExtents)
            .property_readonly("Orientation", &oo::OrientedBoundingBox::Orientation)
            ;
    }

    AABB::AABB(glm::vec3 min, glm::vec3 max)
        : min{ min }
        , max{ max }
    {
    }

    Sphere::Sphere(glm::vec3 center, float radius)
        : center{ center }
        , radius{ radius }
    {
    }
}
