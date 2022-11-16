#pragma once
#include "Shapes.h"

namespace oo
{
    namespace intersection
    {
        bool RayOBB(Ray, OrientedBoundingBox);
        bool RayAABB(Ray, BoundingBox);
    };
}