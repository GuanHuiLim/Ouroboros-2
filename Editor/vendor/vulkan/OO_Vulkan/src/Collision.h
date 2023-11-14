/************************************************************************************//*!
\file           Collision.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Collision for assignment

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Geometry.h"

namespace oGFX::coll
{
	constexpr static float BARY_EPSILON = { 0.01f };

	enum Collision {
		OUTSIDE,
		INTERSECTS,
		CONTAINS
	};

bool PointSphere(const Point3D& p, const Sphere& s);
bool PointSphere(const Sphere& s, const Point3D& p);

bool PointAabb(const Point3D& p, const AABB& aabb);
bool PointAabb(const AABB& aabb, const Point3D& p);

bool PointPlane(const Point3D& q, const Plane& p);
bool PointPlane(const Plane& p, const Point3D& q);
bool PointPlane(const Point3D& q, const Plane& p, float epsilon);

float SqDistPointAabb(const Point3D& p, const AABB& a);
float DistPointPlane(const Point3D& q, const Plane& p);
float DistanceToPoint(const Plane& p, const Point3D& point);

float ScalarTriple(const Point3D& u, const Point3D& v, const Point3D& w);

bool BaryCheckUVW(float u, float v, float w);
bool BaryCheckUVW(const Point3D& p1, const Point3D& p2, const Point3D& p3, float u, float v, float w);
bool BaryCentricTriangle(const Point3D& p, const Point3D& p1, const Point3D& p2, const Point3D& p3, float u, float v, float w);
bool BaryCentricTriangle(const Point3D& p, const Triangle& tri,float& u, float& v, float& w);

bool SphereSphere(const Sphere& a, const Sphere& b);

bool AabbAabb(const AABB& a, const AABB& b);
bool AabbContains(const AABB& source, const AABB& contain);

bool SphereAabb(const Sphere& s, const AABB& a);
bool SphereAabb(const AABB& a, const Sphere& s);

bool RayPlane(const Ray& r, const Plane& p, float& t, Point3D& pt);
bool RayPlane(const Ray& r, const Plane& p, float& t);
bool RayPlane(const Ray& r, const Plane& p);
bool RayPlane(const Plane& p, const Ray& r);

bool RayAabb(const Ray& r, const AABB& a);
bool RayAabb(const AABB& a, const Ray& r);
bool RayAabb(const Ray& r, const AABB& a, float& tmin);
bool RayAabb(const Ray& r, const AABB& a, float& tmin, float& tmax, Point3D& p);

bool RaySphere(const Ray& r, const Sphere& s);
bool RaySphere(const Ray& r, const Sphere& s, float& t);

bool RayTriangle(const Ray& r, const Triangle& tri);
bool RayTriangle(const Ray& r, const Triangle& tri, float& t);

bool PointInTriangle(const Point3D& p, const Point3D& v1, const Point3D& v2, const Point3D& v3);
bool PointInTriangle(const Point3D& p, const Triangle& t);
bool PointInTriangle(const Triangle& t, const Point3D& p);

bool PlaneSphere(const Plane& p, const Sphere& s);
bool PlaneSphere(const Plane& p, const Sphere& s, float& t);
bool SphereOnOrForwardPlane(const Plane& p, const Sphere& s);
bool PointOnOrForwardPlane(const Plane& p, const Point3D& q);
bool PointOnOrForwardPlane(const Plane& p, const Point3D& q, float* t);

bool PlaneAabb(const Plane& p, const AABB& a);
bool PlaneAabb(const Plane& p, const AABB& a, float& t);

bool SphereInFrustum(const Frustum& f, const Sphere& s);
Collision AABBInFrustum(const Frustum& f, const AABB& a, bool draw = false);

}// end namespace oGFX::coll
