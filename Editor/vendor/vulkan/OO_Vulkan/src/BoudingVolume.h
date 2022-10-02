/************************************************************************************//*!
\file           BoudingVolume.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              BV for assignment

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Geometry.h"
#include <vector>

namespace oGFX::BV
{
	static constexpr size_t RITTERS_METHOD{ 1 };
	enum EPOS : size_t
	{		
		_6 =  1,
		_14 = 2,
		_26 = 3,
		_98 = 6,
	};

	void MostSeparatedPointsOnAABB(uint32_t& min, uint32_t& max, const std::vector<Point3D>& points);
	void MostSeperatedPointsOnAxes(uint32_t& min, uint32_t& max, const std::vector<Point3D>& points, size_t range);

	void ExtremePointsAlongDirection(const glm::vec3& axis, const std::vector<Point3D>& points, uint32_t& min, uint32_t& max, float* min_val = nullptr, float* max_val = nullptr);

	void ExpandSphereAboutPoint(Sphere& s, const Point3D& point);
	void SphereFromDistantPoints(Sphere& s, const std::vector<Point3D>& points);

	Sphere HorizonDisk(const Point3D& view, const Sphere& s);

	void BoundingAABB(AABB& aabb, const std::vector<Point3D>& points);

	void LarsonSphere(Sphere& s, const std::vector<Point3D>& points, size_t range = 2);

	void RitterSphere(Sphere& s, const std::vector<Point3D>& points);

	void CovarianceMatrix(Mat3& cov, const std::vector<Point3D>& points);
	void SymSchur2(const Mat3& a, int32_t p, int32_t q, float& c, float& s);
	void Jacobi(Mat3& cov, Mat3& v);
	void EigenSphere(Sphere& s, const std::vector<Point3D>& points);

	void RittersEigenSphere(Sphere& s, const std::vector<Point3D>& points);

	PlaneOrientation ClassifyPointToPlane(const Point3D& q, const Plane& p);
	TriangleOrientation ClassifyTriangleToPlane(const Triangle& t, const Plane& p);

	bool SliceEdgeAgainstPlane(const Point3D& v0, const Point3D& v1, const Plane& p, Point3D& newPoint);
	int SliceTriangleAgainstPlane(const Triangle& t, const Plane& p,
		std::vector<Point3D>& positiveVerts, std::vector<uint32_t>& positiveIndices,
		std::vector<Point3D>& negativeVerts, std::vector<uint32_t>& negativeIndices
		);

	Plane PlaneFromTriangle(const Triangle& t);

	std::vector<glm::vec3> GetAxisFromDictionary(size_t range = 3);

};

