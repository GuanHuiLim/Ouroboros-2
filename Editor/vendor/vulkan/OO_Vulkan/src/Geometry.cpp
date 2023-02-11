/************************************************************************************//*!
\file           Geometry.cpp
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
#include "Geometry.h"

namespace oGFX {
AABB::AABB() :
	center{},
	halfExt{ 0.5f,0.5f,0.5f }
{
}

AABB::AABB(const Point3D& min, const Point3D& max)
{
	auto mid = (min + max) / 2.0f;
	center = mid;
	halfExt = { max - mid };
}

Point3D AABB::max()const
{
	return this->center + this->halfExt;
}

Point3D AABB::min()const
{
	return this->center - this->halfExt;
}

Sphere::Sphere() : center{}, radius{ 1.0f }
{
}

Sphere::Sphere(Point3D p, float r) :
	center{ p },
	radius{ r }
{
}


Ray::Ray() :
	start{},
	direction{ 1.0 }
{
}

Ray::Ray(const Point3D& s, const Point3D& dir) :
	start{ s },
	direction{ dir }
{
}

Plane::Plane() 
{
	static const glm::vec3 norm = []() {
		glm::vec3 n{1.0f};
		n = glm::normalize(n);
		return n;
	}();
	normal = { norm,0.0f };
}

Plane::Plane(const Point3D& n, const Point3D& p)
{
	auto nv = glm::normalize(n);
	normal = { nv, (glm::dot(p,nv)) };
}

Plane::Plane(const Point3D& n, float d)
	:
	normal{ glm::normalize(n),d }
{
}

std::pair<glm::vec3, glm::vec3> Plane::ToPointNormal() const
{
	float d = normal.w / normal.z;
	assert(d != 0.0f);
	return std::pair<glm::vec3, glm::vec3>( glm::vec3{ 0.0,0.0, d},glm::vec3{ normal });
}

Triangle::Triangle():
v0{glm::vec3{0.5f,0.5f,0.5f}},
v1{glm::vec3{0.0f,0.5f,0.5f}},
v2{glm::vec3{0.5f,0.0f,0.5f}}
{
}

Triangle::Triangle(const Point3D& a, const Point3D& b, const Point3D& c)
	:v0{a},
	v1{b},
	v2{c}
{
}
