#include "BoudingVolume.h"
#include "Collision.h"
#include <algorithm>
#include <iostream>

namespace  oGFX::BV
{

	static std::vector<std::vector<glm::vec3>> g_axis{

		std::vector<glm::vec3>{ { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f, 1.0f} }, // 3 : total 3
		
		std::vector<glm::vec3>{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f,-1.0f}, { 1.0f,-1.0f, 1.0f}, { 1.0f,-1.0f,-1.0f} },  // 4 : total 7

		std::vector<glm::vec3>{ { 1.0f, 1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f}, { 1.0f, 0.0f, 1.0f}, { 1.0f, 0.0f,-1.0f}, { 0.0f, 1.0f, 1.0f}, { 0.0f, 1.0f,-1.0f} },  // 6  : total 13

		std::vector<glm::vec3>{ { 0.0f, 1.0f, 2.0f }, { 0.0f, 2.0f, 1.0f}, { 1.0f, 0.0f, 2.0f}, { 2.0f, 0.0f, 1.0f}, { 1.0f, 2.0f, 0.0f}, { 2.0f, 1.0f, 0.0f},
								{ 0.0f, 1.0f,-2.0f }, { 0.0f, 2.0f,-1.0f}, { 1.0f, 0.0f,-2.0f}, { 2.0f, 0.0f,-1.0f}, { 1.0f,-2.0f, 0.0f}, { 2.0f,-1.0f, 0.0f} }, // 12 : total 25

		std::vector<glm::vec3>{ { 1.0f, 1.0f, 2.0f }, { 2.0f, 1.0f, 1.0f}, { 1.0f, 2.0f, 1.0f}, { 1.0f,-1.0f, 2.0f}, { 1.0f, 1.0f,-2.0f}, { 1.0f,-1.0f,-2.0f},
								{ 2.0f,-1.0f, 1.0f }, { 2.0f, 1.0f,-1.0f}, { 2.0f,-1.0f,-1.0f}, { 1.0f,-2.0f, 1.0f}, { 1.0f, 2.0f,-1.0f}, { 1.0f,-2.0f,-1.0f} }, // 12 : total 37

		std::vector<glm::vec3>{ { 2.0f, 2.0f, 1.0f }, { 1.0f, 2.0f, 2.0f}, { 2.0f, 1.0f, 2.0f}, { 2.0f,-2.0f, 1.0f}, { 2.0f, 2.0f,-1.0f}, { 2.0f,-2.0f,-1.0f} ,
								{ 1.0f,-2.0f, 2.0f }, { 1.0f, 2.0f,-2.0f}, { 1.0f,-2.0f,-2.0f}, { 2.0f,-1.0f, 2.0f}, { 2.0f, 1.0f,-2.0f}, { 2.0f,-1.0f,-2.0f} }, // 12 : total 49

	};

	void MostSeparatedPointsOnAABB(uint32_t& min, uint32_t& max, const std::vector<Point3D>& points)
	{

		//find extreme points
		uint32_t minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;
		for (uint32_t i = 1; i < points.size(); ++i)
		{
			if (points[i].x < points[minx].x) minx = i;
			if (points[i].x > points[maxx].x) maxx = i;
			if (points[i].y < points[miny].y) miny = i;
			if (points[i].y > points[maxy].y) maxy = i;
			if (points[i].z < points[minz].z) minz = i;
			if (points[i].z > points[maxz].z) maxz = i;
		}

		// Compute the squared distances for the three pairs of points
		float dist2x = glm::dot((points[maxx] - points[minx]), (points[maxx] - points[minx]));
		float dist2y = glm::dot((points[maxy] - points[miny]), (points[maxy] - points[miny]));
		float dist2z = glm::dot((points[maxz] - points[minz]), (points[maxz] - points[minz]));
		// Pick the pair (min,max) of points most distant
		min = minx;
		max = maxx;
		if (dist2y > dist2x && dist2y > dist2z)
		{
			max = maxy;
			min = miny;
		}
		if (dist2z > dist2x && dist2z > dist2y)
		{
			max = maxz;
			min = minz;
		}

	}

	
	void MostSeperatedPointsOnAxes(uint32_t& min, uint32_t& max, const std::vector<Point3D>& points, size_t range)
	{
		size_t numAxis{};
		// sum up the number of min maxes we need
		for (size_t i = 0; i < range; i++)
		{
			numAxis += g_axis[i].size();
		}

		struct MinMax
		{
			float min{FLT_MAX};
			float max{FLT_MIN};
			uint32_t minIdx{};
			uint32_t maxIdx{};
		};
		std::vector<MinMax> axisExtents(numAxis);

		numAxis = 0;
		for (size_t axes = 0; axes < range; ++axes)
		{
			for (const auto& axis : g_axis[axes])
			{
				auto& ei = axisExtents[numAxis];
				ExtremePointsAlongDirection(axis, points, ei.minIdx, ei.maxIdx, &ei.min, &ei.max);
				//for (size_t p = 0; p < points.size(); ++p)
				//{
				//	float val = glm::dot(axis, points[p]);
				//	if (val > axisExtents[numAxis].max)
				//	{
				//		axisExtents[numAxis].max = val;
				//		axisExtents[numAxis].maxIdx = p;
				//		
				//	}
				//	if (val < axisExtents[numAxis].min)
				//	{
				//		axisExtents[numAxis].min = val;
				//		axisExtents[numAxis].minIdx = p;
				//	}
				//}
				++numAxis;
			}
		}

		// now find the biggest amongst all
		min = axisExtents[0].minIdx;
		max = axisExtents[0].maxIdx;
		float bigDist = glm::dot(points[max]-points[min], points[max]-points[min]);
		for (size_t i = 1; i < axisExtents.size(); ++i)
		{
			const auto& extents = axisExtents[i];
			float newDist = glm::dot(points[extents.maxIdx]-points[extents.minIdx], points[extents.maxIdx]-points[extents.minIdx]);
			if (newDist > bigDist)
			{
				min = extents.minIdx;
				max = extents.maxIdx;
				bigDist = newDist;
			}
		}

	}

	void ExtremePointsAlongDirection(const glm::vec3& axis, const std::vector<Point3D>& points, uint32_t& min, uint32_t& max, float* min_val, float* max_val)
	{
		max = 0;
		min = 0;
		float t_min{FLT_MAX};
		float t_max{FLT_MIN};
		for (size_t p = 0; p < points.size(); p++)
		{
			float val = glm::dot(axis, points[p]);
			if (val == t_max) // degenerate exactly the same dist
			{
				auto pointdiff = points[max] - points[min];
				float smallDist = glm::dot(pointdiff, pointdiff);
				auto nPtDif = points[p] - points[min];
				float dist = glm::dot(nPtDif, nPtDif);
				if (dist > smallDist)
				{
					t_max = val;
					max = static_cast<uint32_t>(p);
				}
			}
			else if (val > t_max)
			{
				t_max = val;
				max = static_cast<uint32_t>(p);
			}

			if (val == t_min)
			{
				auto pointdiff = points[min] - points[max];
				float smallDist = glm::dot(pointdiff, pointdiff);
				auto nPtDif = points[p] - points[max];
				float dist = glm::dot(nPtDif, nPtDif);
				if (dist > smallDist)
				{
					t_min = val;
					min = static_cast<uint32_t>(p);
				}
			}
			else if (val < t_min)
			{
				t_min = val;
				min = static_cast<uint32_t>(p);
			}
			
		}		

		if (max_val) *max_val = t_max;
		if (min_val) *min_val = t_min;
	}

	void ExpandSphereAboutPoint(Sphere& s, const Point3D& point)
	{
		// Compute squared distance between point and sphere center
		glm::vec3 d = point - s.center;
		float dist2 = glm::dot(d, d);
		// Only update s if point p is outside i
		if (dist2 > s.radius * s.radius)
		{
			float dist = std::sqrt(dist2);
			float newRadius = (s.radius + dist) * 0.5f;
			float k = (newRadius - s.radius) / dist;
			s.radius = newRadius;
			s.center += d * k;
		}
	}

	void SphereFromDistantPoints(Sphere& s, const std::vector<Point3D>& points, size_t range)
	{
		// Find the most separated point pair defining the encompassing AABB
		uint32_t min, max;
		MostSeperatedPointsOnAxes(min, max, points, range);
		// Set up sphere to just encompass these two points
		s.center = (points[min] + points[max]) * 0.5f;
		s.radius = glm::dot(points[max] - s.center, points[max] - s.center);
		s.radius = std::sqrt(s.radius);
	}

	Sphere HorizonDisk(const Point3D& view, const Sphere& s)
	{
		Sphere disk;
		glm::vec3 dir = s.center - view;
		float d = std::sqrt(glm::dot(dir,dir));
		assert(d != 0.0f); // divide by zero why was this hit.
		dir /= d; // length

		float l = std::sqrt(d * d - s.radius * s.radius);
		float rp = s.radius * l / d;
		float z = std::sqrt(s.radius * s.radius - rp * rp);

		disk.center = s.center - z * dir;
		disk.radius = rp;
		return disk;
	}

	void BoundingAABB(AABB& aabb, const std::vector<Point3D>& points)
	{
		glm::vec3 Min( FLT_MAX );
		glm::vec3 Max( -FLT_MAX );
		for( unsigned int i = 0; i < points.size(); ++i )
		{
			const Point3D& pt = points[i];
			Min.x = std::min( Min.x, pt.x );
			Min.y = std::min( Min.y, pt.y );
			Min.z = std::min( Min.z, pt.z );
			Max.x = std::max( Max.x, pt.x );
			Max.y = std::max( Max.y, pt.y );
			Max.z = std::max( Max.z, pt.z );
		}

		auto midPoint = (Min + Max) / 2.0f;
		aabb.center = midPoint;
		aabb.halfExt = { Max-aabb.center };
		//aabb.halfExt /= 2.0f;
	}

	void LarsonSphere(Sphere& s, const std::vector<Point3D>& points, size_t range)
	{
		// Get sphere encompassing two approximately most distant points
		SphereFromDistantPoints(s, points, range);
		// Grow sphere to include all points
		for (int i = 0; i < points.size(); i++)
		{
			ExpandSphereAboutPoint(s, points[i]);
		}
	}


	void RitterSphere(Sphere& s, const std::vector<Point3D>& points)
	{
		LarsonSphere(s, points, RITTERS_METHOD);
	}

	void CovarianceMatrix(Mat3& cov, const std::vector<Point3D>& points)
	{
		float oon = 1.0f / static_cast<float>(points.size());
		Point3D c = Point3D(0.0f, 0.0f, 0.0f);
		float e00{}, e11{}, e22{}, e01{}, e02{}, e12{};
		// Compute the center of mass (centroid) of the points
		for (size_t i = 0; i < points.size(); ++i)
		{
			c += points[i];			
		}
		// mean
		c *= oon;

		// Compute covariance elements
		for (size_t j = 0; j < points.size(); ++j)
		{
			// Translate points so center of mass is at origin
			Point3D p = points[j] - c;
			// Compute covariance of translated points
			e00 += p.x * p.x;
			e11 += p.y * p.y;
			e22 += p.z * p.z;
			e01 += p.x * p.y;
			e02 += p.x * p.z;
			e12 += p.y * p.z;
		}

		// Fill in the covariance matrix elements
		cov[0][0] = e00 * oon;
		cov[1][1] = e11 * oon;
		cov[2][2] = e22 * oon;
		cov[0][1] = cov[1][0] = e01 * oon;
		cov[0][2] = cov[2][0] = e02 * oon;
		cov[1][2] = cov[2][1] = e12 * oon;

	}

	void SymSchur2(const Mat3& a, int32_t p, int32_t q, float& c, float& s)
	{
		const static float EPSILON = 0.000001f; // high res epsilon
		if (std::abs(a[p][q]) > EPSILON)
		{
			float r = (a[q][q] - a[p][p]) / (2.0f * a[p][q]);
			float t;
			if (r >= 0.0f)
			{
				t = 1.0f / (r + std::sqrt(1.0f + r * r));
			}
			else
			{
				t = -1.0f / (-r + std::sqrt(1.0f + r * r));
			}
			c = 1.0f / std::sqrt(1.0f + t * t);
			s = t * c;
		}
		else
		{
			c = 1.0f;
			s = 0.0f;
		}
	}

	void Jacobi(Mat3& A, Mat3& v)
	{
		int i, j, n, p, q;

		float prevoff{ FLT_MAX }, c, s;
		Mat3 J, b, t;
		// Initialize v to identity matrix
		v = Mat3{ 1.0f };

		// Repeat for some maximum number of iterations
		const int MAX_ITERATIONS = 50;
		for (n = 0; n < MAX_ITERATIONS; n++)
		{
			// Find largest off-diagonal absolute element a[p][q]
			p = 0; q = 1;
			if (std::abs(A[p][q]) > std::abs(A[0][2])) p = 0, q = 2;
			if (std::abs(A[p][q]) > std::abs(A[1][2])) p = 1, q = 2;

			// Compute the Jacobi rotation matrix J(p, q, theta)
			// (This code can be optimized for the three different cases of rotation)
			SymSchur2(A, p, q, c, s);

			// create rotation matrix
			J = Mat3{ 1.0f };
			J[p][p] = c; J[p][q] = s;
			J[q][p] = -s; J[q][q] = c;
			// Cumulate rotations into what will contain the eigenvectors
			v = v * J;
			// Make ’a’ more diagonal, until just eigenvalues remain on diagonal
			A = (glm::transpose(J) * A) * J;
			// Compute "norm" of off-diagonal elements
			float off = 0.0f;
			for (i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
				{
					if (i == j) continue;
					off += A[i][j] * A[i][j];
				}
			}
			/* off = sqrt(off); not needed for norm comparison */
			// Stop when norm no longer decreasing
			if (n > 2 && off >= prevoff)
				return;
			prevoff = off;
		}
	}

	void EigenSphere(Sphere& s, const std::vector<Point3D>& points)
	{
		Mat3 covMtx{ 0.0f };
		CovarianceMatrix(covMtx, points);

		Mat3 eigenVectors{ 0.0f };
		Jacobi(covMtx, eigenVectors);

		glm::vec3 e;
		int maxc = 0;
		float maxf, maxe = std::abs(covMtx[0][0]);
		if ((maxf = std::abs(covMtx[1][1])) > maxe) maxc = 1, maxe = maxf;
		if ((maxf = std::abs(covMtx[2][2])) > maxe) maxc = 2, maxe = maxf;
		e[0] = eigenVectors[0][maxc];
		e[1] = eigenVectors[1][maxc];
		e[2] = eigenVectors[2][maxc];

		uint32_t imin, imax;
		ExtremePointsAlongDirection(e, points, imin, imax);
		Point3D minpt = points[imin];
		Point3D maxpt = points[imax];
		float dist = std::sqrt(glm::dot(maxpt - minpt, maxpt - minpt));
		s.radius = dist * 0.5f;
		s.center = (minpt + maxpt) * 0.5f;

		//Grow sphere to include all points
		for (size_t i = 0; i < points.size(); ++i)
		{
			ExpandSphereAboutPoint(s, points[i]);
		}
	}

	void RittersEigenSphere(Sphere& s, const std::vector<Point3D>& points)
	{
		EigenSphere(s, points);

		//Grow sphere to include all points
		for (size_t i = 0; i < points.size(); ++i)
		{
			ExpandSphereAboutPoint(s, points[i]);
		}

	}

	PlaneOrientation ClassifyPointToPlane(const Point3D& q, const Plane& p)
	{
		float dist = glm::dot(glm::vec3(p.normal), q) - p.normal.w;

		if (dist > EPSILON)
		{
			return PlaneOrientation::POSITIVE;
		}
		else if(dist < -EPSILON)
		{
			return PlaneOrientation::NEGATIVE;
		}

		return PlaneOrientation::COPLANAR;
	}

	TriangleOrientation ClassifyTriangleToPlane(const Triangle & t, const Plane & p)
	{
		// Loop over all polygon vertices and count how many vertices
		// lie in front of and how many lie behind of the thickened plane
		int numInFront = 0, numBehind = 0;

		Point3D v[3];
		v[0] = t.v0;
		v[1] = t.v1;
		v[2] = t.v2;

		int numVerts = 3;
		for (int i = 0; i < numVerts; i++) {
			switch (ClassifyPointToPlane(v[i], p)) {
			case PlaneOrientation::POSITIVE:
			numInFront++;
			break;
			case PlaneOrientation::NEGATIVE:
			numBehind++;
			break;
			}
		}
		// If vertices on both sides of the plane, the polygon is straddling
		if (numBehind != 0 && numInFront != 0)
			return TriangleOrientation::STRADDLE;
		// If one or more vertices in front of the plane and no vertices behind
		// the plane, the polygon lies in front of the plane
		if (numInFront != 0)
			return TriangleOrientation::POSITIVE;
		// Ditto, the polygon lies behind the plane if no vertices in front of
		// the plane, and one or more vertices behind the plane
		if (numBehind != 0)
			return TriangleOrientation::NEGATIVE;
		// All vertices lie on the plane so the polygon is coplanar with the plane
		return TriangleOrientation::COPLANAR;
	}

	bool SliceEdgeAgainstPlane(const Point3D& v0, const Point3D& v1, const Plane& p, Point3D& newPoint)
	{		
		int comp = -1;

		PlaneOrientation  t0 = ClassifyPointToPlane(v0, p);
		PlaneOrientation  t1 = ClassifyPointToPlane(v1, p);
		PlaneOrientation chk = PlaneOrientation(t0 | t1);

		if (chk == (PlaneOrientation::POSITIVE | PlaneOrientation::NEGATIVE) )
		{
			Ray r;
			r.start = v0;
			r.direction = v1 - v0;
			float t{0.0f};
			coll::RayPlane(r, p, t, newPoint);
			{
				return true;
			}
		}		
		return false;
	}

	int SliceTriangleAgainstPlane(const Triangle& t, const Plane& p,
		std::vector<Point3D>& positiveVerts, std::vector<uint32_t>& positiveIndices,
		std::vector<Point3D>& negativeVerts, std::vector<uint32_t>& negativeIndices)
	{
		int count = 0;

		Point3D v[3];
		v[0] = t.v0;
		v[1] = t.v1;
		v[2] = t.v2;


		bool positiveSide = (oGFX::BV::ClassifyPointToPlane(t.v0,p) == PlaneOrientation::POSITIVE);

		Point3D pt;
		std::vector <Point3D> frontList;
		std::vector <Point3D> backList;
		for (size_t i = 0; i < 3; i++)
		{
			int nextIndx = (i + 1) % 3;
			if (SliceEdgeAgainstPlane(v[i], v[nextIndx], p, pt))
			{
				if (positiveSide) // originating from frontside
				{
					frontList.push_back(pt);

					backList.push_back(pt);
					backList.push_back(v[nextIndx]);
				}
				else
				{
					backList.push_back(pt);

					frontList.push_back(pt);
					frontList.push_back(v[nextIndx]);
				}
				positiveSide = !positiveSide;
				
			}
			else
			{
				if (positiveSide)
				{					
					frontList.push_back(v[nextIndx]);
				}
				else
				{
					backList.push_back(v[nextIndx]);
				}
			}
		}
		// triangulate and add to list
		auto pSz = static_cast<uint32_t>(positiveVerts.size());
		for (uint32_t j = 2; j < frontList.size(); ++j)
		{
			++count;
			uint32_t i = j - 2;
			positiveVerts.push_back(frontList[i+0]);
			positiveVerts.push_back(frontList[i+1]);
			positiveVerts.push_back(frontList[i+2]);

			positiveIndices.push_back(i*3 +0 + pSz);
			positiveIndices.push_back(i*3 +1 + pSz);
			positiveIndices.push_back(i*3 +2 + pSz);
		}

		auto nSz = static_cast<uint32_t>(negativeVerts.size());
		for (uint32_t j = 2; j < backList.size(); ++j)
		{
			++count;
			uint32_t i = j - 2;
			negativeVerts.push_back(backList[i+0]);
			negativeVerts.push_back(backList[i+1]);
			negativeVerts.push_back(backList[i+2]);

			negativeIndices.push_back(i*3 +0 + nSz);
			negativeIndices.push_back(i*3 +1 + nSz);
			negativeIndices.push_back(i*3 +2 + nSz);
		}

		return count;
	}

	Plane PlaneFromTriangle(const Triangle & t)
	{
		Plane p;
		glm::vec3 normal = glm::normalize(glm::cross(t.v1 - t.v0, t.v2 - t.v0));
		float d = std::sqrtf(glm::dot(t.v0 * normal,t.v0 * normal));
		p.normal = glm::vec4{ normal,d };

		return p;
	}

	std::vector<glm::vec3> GetAxisFromDictionary(size_t range)
	{
		std::vector<glm::vec3> result;

		for (size_t i = 0; i < range; i++)
		{
			auto& axes = g_axis[i];
			result.insert(result.end(), axes.begin(), axes.end());
		}

		return result;
	}

} // end namespace oGFX::BV
