#include "pch.h"
#include "Algorithms.h"

#include <glm/gtc/matrix_inverse.hpp>
namespace oo
{
    namespace intersection
    {
        bool RayOBB(Ray ray, OrientedBoundingBox obb)
        {
            //glm::vec3 c = obb.Center; // Box center-point
            //glm::vec3 e = obb.HalfExtents; // Box halflength extents
            //glm::vec3 m = ray.Position; // Segment midpoint
            //glm::vec3 d = ray.Direction; // Segment halflength vector
            //m = m - c; // Translate box and segment to origin
            //// Try world coordinate axes as separating axes
            //float adx = std::abs(d.x);
            //if (std::abs(m.x) > e.x + adx) return false;
            //float ady = std::abs(d.y);
            //if (std::abs(m.y) > e.y + ady) return false;
            //float adz = std::abs(d.z);
            //if (std::abs(m.z) > e.z + adz) return false;
            //
            //// Add in an epsilon term to counteract arithmetic errors when segment is
            //// (near) parallel to a coordinate axis (see text for detail)
            //adx += std::numeric_limits<float>::epsilon(); 
            //ady += std::numeric_limits<float>::epsilon(); 
            //adz += std::numeric_limits<float>::epsilon();

            //// Try cross products of segment direction vector with coordinate axes
            //if (std::abs(m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady) return false;
            //if (std::abs(m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx) return false;
            //if (std::abs(m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx) return false;
            //// No separating axis found; segment must be overlapping AABB
            //return true;
            
            auto t = glm::translate(glm::mat4{ 1.f }, obb.Center);
            auto r = glm::mat4{ glm::mat4_cast(obb.Orientation) };
            auto s = glm::scale(glm::mat4{ 1.f }, obb.HalfExtents);
            auto m = t * r * s;
            auto invm = glm::affineInverse(m);

            auto raypos = invm* glm::vec4(ray.Position, 1);
            auto raydir = invm * glm::vec4(ray.Direction, 0);
            Ray newRay = { raypos, raydir };
            BoundingBox bb { {0, 0, 0}, { 1, 1, 1 } };

            return RayAABB(newRay, bb);
        }

        bool RayAABB(Ray ray, BoundingBox bb)
        {
            glm::vec3 boxMin = bb.Center - bb.HalfExtents;
            glm::vec3 boxMax = bb.Center + bb.HalfExtents;
            
            glm::vec3 tmin = (boxMin - ray.Position) / ray.Direction;
            glm::vec3 tmax = (boxMax - ray.Position) / ray.Direction;
            glm::vec3 sc = glm::min(tmin, tmax);
            glm::vec3 sf = glm::max(tmin, tmax);

            float t0 = std::max(std::max(sc.x, sc.y), sc.z);
            float t1 = std::min(std::min(sf.x, sf.y), sf.z);

            if (!(t0 <= t1 && t1 > 0.0))
                return false;
            
            return true;
            //return false;
        }

    }
}
