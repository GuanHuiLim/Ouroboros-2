/************************************************************************************//*!
\file           Collision.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Collision for assignmetn

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "Collision.h"
#include <algorithm>

namespace coll
{

bool PointSphere(const Point3D& p, const Sphere& s)
{
	glm::vec3 d = (glm::vec3)p - (glm::vec3)s.center;
	float r_sq = (s.radius * s.radius);
	return (glm::dot(d, d) <= r_sq);
}

bool PointSphere(const Sphere& s, const Point3D& p)
{
	return PointSphere(p,s);
}

bool BaryCentricTriangle(const Point3D& p, const Triangle& tri, float& u, float& v, float& w)
{
	auto l0 = tri.v1 - tri.v0;
	auto l1 = tri.v2 - tri.v1;
	auto l2 = tri.v0 - tri.v2;
	auto signedMag = glm::length(glm::cross(-l1,l0));

	if (signedMag == 0) return false;

	auto s0 = p - tri.v0;
	auto s1 = p - tri.v1;
	auto s2 = p - tri.v2;

	auto al0 = glm::length(glm::cross(l0, s0));
	auto al1 = glm::length(glm::cross(l1, s1));
	auto al2 = glm::length(glm::cross(l2, s2));

	auto r0 = al0 / signedMag;
	auto r1 = al1 / signedMag;
	auto r2 = al2 / signedMag;

	auto result = BaryCheckUVW(r0, r1, r2);

	if (result)
	{
		u = r0;
		v = r1;
		w = r2;
	}

	return true;
}

bool SphereSphere(const Sphere& a, const Sphere& b)
{
	return PointSphere(b.center,
		Sphere{a.center, a.radius + b.radius});
}

bool AabbAabb(const AABB& a, const AABB& b)

{
	// for each axis { X, Y, Z }
	const auto aMax = a.max();
	const auto aMin = a.min();

	const auto bMax = b.max();
	const auto bMin = b.min();

	// todo optimize for XYZ based on game
	for(unsigned int i = 0; i < 3; ++i)
	{
		// if no overlap for the axis, no overlap overall
		if(aMax[i] < bMin[i] || bMax[i] < aMin[i])
			return false;
	}
	return true;
}

bool SphereAabb(const Sphere& s, const AABB& a)
{
	float sqrDist = SqDistPointAabb(s.center, a);

	return sqrDist <= s.radius*s.radius;
}

bool SphereAabb(const AABB& a, const Sphere& s)
{
	return SphereAabb(s,a);
}

bool PointAabb(const Point3D& p, const AABB& aabb)
{
	const auto min = aabb.min();
	const auto max = aabb.max();

	for (uint32_t i = 0; i < 3; i++)
	{
		if (p[i] < min[i] || p[i] > max[i])
			return false;
	}
	return true;
}

bool PointAabb(const AABB& aabb, const Point3D& p)
{
	return PointAabb(p,aabb);
}

bool PointPlane(const Point3D& q, const Plane& p)
{
	
	return PointPlane(q,p,EPSILON);
}

bool PointPlane(const Plane& p, const Point3D& q)
{
	return PointPlane(q,p);
}

bool PointPlane(const Point3D& q, const Plane& p, float epsilon)
{
	float d = glm::dot(q, glm::vec3(p.normal));
	return std::abs(d- p.normal.w) < epsilon;
}

float SqDistPointAabb(const Point3D& p, const AABB& a)
{
	float sqrDist = 0.0f;

	glm::vec3 min = a.min();
	glm::vec3 max = a.max();

	// for each axis
	for (glm::vec3::length_type i = 0; i < 3; i++)
	{
		float v = p[i];
		if (v < min[i]) sqrDist += (min[i] - v) * (min[i] - v);
		if (v > max[i]) sqrDist += (v- max[i]) * (v - max[i]);
	}

	return sqrDist;
}

float DistPointPlane(const Point3D& q, const Plane& p)
{
	auto n = glm::vec3(p.normal);// can skip dot if normalized
	return (glm::dot(n,q)-p.normal.w)/ glm::dot(n,n);
}

float ScalarTriple(const Point3D& u, const Point3D& v, const Point3D& w)
{
	return glm::dot(glm::cross(u, v), w);
}

bool BaryCheckUVW(float u, float v, float w)
{
	if (std::abs(u + v + w) - 1.0f > BARY_EPSILON)
	{
		return false;
	}

	if (u > 1.0f + BARY_EPSILON || u < -BARY_EPSILON) return false;
	if (v > 1.0f + BARY_EPSILON || v < -BARY_EPSILON) return false;
	if (w > 1.0f + BARY_EPSILON || w < -BARY_EPSILON) return false;

	return true;
}

bool BaryCheckUVW(const Point3D& p1, const Point3D& p2, const Point3D& p3, float u, float v, float w)
{
	return BaryCheckUVW(u,v,w);
}

bool BaryCentricTriangle(const Point3D& p, const Point3D& p1, const Point3D& p2, const Point3D& p3, float u, float v, float w)
{
	return BaryCentricTriangle(p,Triangle(p1,p2,p3),u,v,w);
}


bool RayPlane(const Ray& r, const Plane& p, float& t, Point3D& pt)
{
	glm::vec3 pNorm = p.normal;
	float d = p.normal.w;

	float divs = glm::dot(pNorm, r.direction);
	if (std::abs(divs) > EPSILON)
	{
		t = (d-glm::dot(pNorm, r.start)) / divs;
		if (t >= EPSILON)
		{
			pt = r.start + r.direction * t;
			return true;
		}
	}
	return false;
}

bool RayPlane(const Ray& r, const Plane& p, float& t)
{
	Point3D pt;
	return RayPlane(r, p, t, pt);
}

bool RayPlane(const Ray& r, const Plane& p)
{
	float t;
	Point3D pt;
	return RayPlane(r, p, t, pt);
}

bool RayPlane(const Plane& p, const Ray& r)
{
	return RayPlane(r,p);
}

bool RayAabb(const Ray& r, const AABB& a)
{
	float tmin;
	float tmax;
	Point3D p;
	return RayAabb(r, a, tmin, tmax, p);	
}

bool RayAabb(const Ray& r, const AABB& a, float& tmin)
{
	float tmax;
	Point3D p;
	return RayAabb(r, a, tmin, tmax, p);
}

bool RayAabb(const Ray& r, const AABB& a, float& tmin, float& tmax, Point3D& p)
{
	tmin = 0.0f; // set to -FLT_MAX to get first hit on line
	tmax = FLT_MAX; // set to max distance ray can travel (for segment)
						  // For all three slabs
	
	auto aMin = a.min();
	auto aMax = a.max();

	for (int i = 0; i < 3; i++)
	{
		if (std::abs(r.direction[i]) < EPSILON)
		{
			// Ray is parallel to slab. No hit if origin not within slab
			if (r.start[i] < aMin[i] || r.start[i] > aMax[i]) return 0;
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab
			float ood = 1.0f / r.direction[i];
			float t1 = (aMin[i] - r.start[i]) * ood;
			float t2 = (aMax[i] - r.start[i]) * ood;
			// Make t1 be intersection with near plane, t2 with far plane
			if (t1 > t2) std::swap(t1, t2);
			// Compute the intersection of slab intersection intervals
			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) return 0;
		}
	}
	// Ray intersects all 3 slabs. Return point (q) and intersection t value (tmin)
	p= r.start + r.direction * tmin;
	return true;
}

bool RaySphere(const Ray& r, const Sphere& s)
{
	float t{ FLT_MAX };
	return RaySphere(r, s, t);
}

bool RaySphere(const Ray& r, const Sphere& s, float& t)
{
	glm::vec3 m = r.start - s.center;
	float b = glm::dot(m, r.direction);
	float c = glm::dot (m, m) - s.radius * s.radius;
	// Exit if r’s origin outside s (c > 0) and r pointing away from s (b > 0)
	if (c > 0.0f && b > 0.0f) return 0;
	float discr = b*b - c;
	// A negative discriminant corresponds to ray missing sphere
	if (discr < 0.0f) return 0;
	// Ray now found to intersect sphere, compute smallest t value of intersection
	t = -b - std::sqrtf(discr);
	// If t is negative, ray started inside sphere so clamp t to zero
	if (t < 0.0f) t = 0.0f;
	
	//q = p + t * d;
	
	return 1;
}

bool RayTriangle(const Ray& r, const Triangle& tri)
{
	float t{ FLT_MAX };
	return RayTriangle(r, tri, t);
}

bool RayTriangle(const Ray& r, const Triangle& tri, float& t)
{
	Plane plane = { glm::cross(tri.v1, tri.v2), tri.v1 };	
	if (RayPlane(r, plane, t))
	{
		return PointInTriangle(r.start + r.direction * t, tri);
	}
	return false;
}

bool PointInTriangle(const Point3D& p, const Point3D& v1, const Point3D& v2, const Point3D& v3)
{
	// Translate point to origin
	Point3D a{ v1 - p };
	Point3D b{ v2 - p };
	Point3D c{ v3 - p };

	// Legrange's identity
	float ab = glm::dot(a, b);
	float ac = glm::dot(a, c);
	float bc = glm::dot(b, c);
	float cc = glm::dot(c, c);

	// normals pab and pbc point in the same direction
	if (bc * ac - cc * ab < 0.0f) return 0;

	float bb = glm::dot(b, b);
	// normals for pab and pca point in the same direction
	if (ab * bc - ac * bb < 0.0f) return 0;

	// p must be in the triangle
	return 1;

}

bool PointInTriangle(const Point3D& p, const Triangle& t)
{
	return PointInTriangle(p,t.v0,t.v1,t.v2);
}

bool PointInTriangle(const Triangle& t, const Point3D& p)
{
	return PointInTriangle(p,t.v0,t.v1,t.v2);
}

bool PlaneSphere(const Plane& p, const Sphere& s)
{
	float t;
	return PlaneSphere(p, s, t);
}

bool PlaneSphere(const Plane& p, const Sphere& s, float& t)
{
	float dist = glm::dot(s.center, glm::vec3(p.normal))-p.normal.w;
	auto result = std::abs(dist) <= s.radius;
	if (result != true)
	{
		t = dist;
	}
	return result;
}

bool PlaneAabb(const Plane& p, const AABB& a)
{
	float t;
	return PlaneAabb(p, a, t);
}

bool PlaneAabb(const Plane& p, const AABB& a, float& t)
{
	Point3D c = a.center;
	const glm::vec3& e = a.halfExt;

	float r = e[0] * std::abs(p.normal[0]) 
		+ e[1] * std::abs(p.normal[1]) 
		+ e[2] * std::abs(p.normal[2]);

	float s = glm::dot(glm::vec3{ p.normal }, a.center);

	auto result = std::abs(s) <= r;
	if (result != true)
	{
		glm::vec3 planePt(glm::vec3{p.normal} * p.normal.w);
		t = -glm::dot(planePt-c, glm::vec3{ p.normal });
	}
	return result;
}

}// end namespace coll
