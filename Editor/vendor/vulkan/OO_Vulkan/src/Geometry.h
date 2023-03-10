/************************************************************************************//*!
\file           Geometry.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              For assignment geometry

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "MathCommon.h"
#include <tuple>

namespace oGFX {
using Point3D = glm::vec3;
using Mat3 = glm::mat3;
constexpr static float EPSILON = { 0.001f };

enum PlaneOrientation
{
	COPLANAR=0,
	POSITIVE=1,
	NEGATIVE=2,
};

enum class TriangleOrientation
{
	COPLANAR = 0,
	POSITIVE = 1,
	NEGATIVE = 2,
	STRADDLE = 4
};

struct Plane
{
	Plane();
	Plane(const Point3D& n, const Point3D& p);
	Plane(const Point3D& n, float d);
	glm::vec4 normal{ 0.0f };
	std::pair<glm::vec3, glm::vec3> ToPointNormal() const;
};


struct Frustum
{
	Plane top;
	Plane bottom;
	Plane right;
	Plane left;
	Plane planeFar;
	Plane planeNear;


	Point3D pt_top;
	Point3D pt_bottom;
	Point3D pt_right;
	Point3D pt_left;
	Point3D pt_planeFar;
	Point3D pt_planeNear;
};

struct Triangle
{
	Triangle();
	Triangle(const Point3D& a, const Point3D& b, const Point3D& c);
	Point3D v0{0.0f};
	Point3D v1{0.0f};
	Point3D v2{0.0f};
};

struct Sphere
{
	Sphere();
	Sphere(Point3D p, float r);
	Point3D center{0.0f};
	float radius{ 0.0f };
};

struct AABB
{
	AABB();
	AABB(const Point3D& min,const Point3D& max);
	Point3D center;
	Point3D halfExt{0.0f};

	Point3D max() const;
	Point3D min() const;
};

struct AABB2D
{
	glm::vec2 min, max;

	AABB2D(glm::vec2 min, glm::vec2 max);

};

struct Ray
{
	Ray();
	Ray(const Point3D& s,const Point3D& dir);
	Point3D start{0.0f};
	glm::vec3 direction{ 0.0f };
};

} // namespace oGFX
