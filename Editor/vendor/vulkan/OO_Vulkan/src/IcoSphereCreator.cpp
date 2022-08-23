#include "IcoSphereCreator.h"


std::vector<Point3D> IcoSphereCreator::CreateIcosphere(uint8_t order)
{





 //   std::vector<Point3D> points;

 //   // set up a 20-triangle icosahedron
 //   const float f = (1 + 5 * 0.5) / 2;
 //   const uint32_t T = 4 * order;

 //   //Float32Array((10 * T + 2) * 3); //size
 //   const std::vector<Point3D> vertices = {
	// {-1.0f,    f, 0.0f }
	//,{ 1.0f,    f, 0.0f }
	//,{-1.0f,   -f, 0.0f }
	//,{ 1.0f,   -f, 0.0f }
	//,{ 0.0f,-1.0f,    f }
	//,{ 0.0f, 1.0f,    f }
	//,{ 0.0f,-1.0f,   -f }
	//,{ 0.0f, 1.0f,   -f }
	//,{    f, 0.0f,-1.0f }
	//,{    f, 0.0f, 1.0f }
	//,{   -f, 0.0f,-1.0f }
	//,{   -f, 0.0f, 1.0f } };

 //   const std::vector<uint32_t> indices{
 //       0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
 //       11, 10, 2, 5, 11, 4, 1, 5, 9, 7, 1, 8, 10, 7, 6,
 //       3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
 //       9, 8, 1, 4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7 };

 //   let v = 12;
 //   const midCache = order ? new Map() : null; // midpoint vertices cache to avoid duplicating shared vertices



 //   return points;
	return {};
}

namespace icosahedron
{

uint32_t vertex_for_edge(Lookup& lookup, VertexList& vertices, Index first, Index second)
{

	{
		Lookup::key_type key(first, second);
		if (key.first>key.second)
			std::swap(key.first, key.second);

		auto inserted=lookup.insert({key, (uint32_t)vertices.size()});
		if (inserted.second)
		{
			auto& edge0 = vertices[first];
			auto& edge1 = vertices[second];
			auto point = glm::normalize(edge0 + edge1);
			vertices.push_back(point);
		}
		return inserted.first->second;
	}
}

TriangleList subdivide(VertexList& vertices, TriangleList triangles)
{
	Lookup lookup;
	TriangleList result;

	for (auto&& each:triangles)
	{
		std::array<Index, 3> mid;
		for (int edge=0; edge<3; ++edge)
		{
			mid[edge]=vertex_for_edge(lookup, vertices,
				each.vertex[edge], each.vertex[(edge+1)%3]);
		}

		result.push_back({each.vertex[0], mid[0], mid[2]});
		result.push_back({each.vertex[1], mid[1], mid[0]});
		result.push_back({each.vertex[2], mid[2], mid[1]});
		result.push_back({mid[0], mid[1], mid[2]});
	}

	return result;
}

IndexedMesh make_icosphere(int subdivisions, bool normalized)
{
	VertexList vertices=icosahedron::vertices;
	TriangleList triangles=icosahedron::triangles;

	assert(std::abs(subdivisions) < 6);

	for (int i=0; i<subdivisions; ++i)
	{
		triangles=subdivide(vertices, triangles);
	}
	if (normalized)
	{
		for (auto&v:vertices)
		{
			// normalize to 0.5 radius sphere
			v *= 0.5f;
		}
	}

	return{vertices, triangles};
}
}// end namespace icosahedron