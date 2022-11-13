#include "pch.h"
#include "Algorithms.h"

namespace oo
{
    namespace intersection
    {
        bool RayOBB(Ray ray, OrientedBoundingBox obb)
        {
            glm::vec3 c = obb.Center; // Box center-point
            glm::vec3 e = obb.HalfExtents; // Box halflength extents
            glm::vec3 m = ray.Position; // Segment midpoint
            glm::vec3 d = ray.Direction; // Segment halflength vector
            m = m - c; // Translate box and segment to origin
            // Try world coordinate axes as separating axes
            float adx = std::abs(d.x);
            if (std::abs(m.x) > e.x + adx) return false;
            float ady = std::abs(d.y);
            if (std::abs(m.y) > e.y + ady) return false;
            float adz = std::abs(d.z);
            if (std::abs(m.z) > e.z + adz) return false;
            
            // Add in an epsilon term to counteract arithmetic errors when segment is
            // (near) parallel to a coordinate axis (see text for detail)
            adx += std::numeric_limits<float>::epsilon(); 
            ady += std::numeric_limits<float>::epsilon(); 
            adz += std::numeric_limits<float>::epsilon();

            // Try cross products of segment direction vector with coordinate axes
            if (std::abs(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady) return false;
            if (std::abs(m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx) return false;
            if (std::abs(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx) return false;
            // No separating axis found; segment must be overlapping AABB
            return true;
        }

    }
}
