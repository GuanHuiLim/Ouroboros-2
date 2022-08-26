#pragma once

#include "Geometry.h"
#include <vector>
#include <map>
#include <array>
#include <tuple>

#ifndef ICOSPHERE
#define ICOSPHERE

namespace icosahedron
{

	using Index = uint32_t;

	struct Triangle
	{
		Index vertex[3];
	};

	using TriangleList = std::vector<Triangle>;
	using VertexList = std::vector<Point3D>;
	using Lookup=std::map<std::pair<Index, Index>, Index>;

	const float X=.525731112119133606f;
	const float Z=.850650808352039932f;
	const float N=0.f;

	inline static const VertexList vertices=
	{
		{-X,N,Z}, {X,N,Z}, {-X,N,-Z}, {X,N,-Z},
		{N,Z,X}, {N,Z,-X}, {N,-Z,X}, {N,-Z,-X},
		{Z,X,N}, {-Z,X, N}, {Z,-X,N}, {-Z,-X, N}
	};

	inline static const TriangleList triangles
	{
		{0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
		{8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
		{7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
		{6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}
	};

	uint32_t vertex_for_edge(Lookup& lookup,
		VertexList& vertices, Index first, Index second);

	TriangleList subdivide(VertexList& vertices,
		TriangleList triangles);

using IndexedMesh=std::pair<VertexList, TriangleList>;

IndexedMesh make_icosphere(int subdivisions, bool normalized = true);

}


class IcoSphereCreator
{
	
	static std::vector<Point3D> CreateIcosphere(uint8_t order);

};


#endif // !ICOSPHERE