#include "OctTree.h"
#include "BoudingVolume.h"
#include <algorithm>
#include <iostream>

OctTree::OctTree(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices, int maxTriangles)
{
	m_vertices = vertices;
	m_indices = indices;
	m_maxNodesTriangles = maxTriangles;

}


std::tuple< std::vector<Point3D>, std::vector<uint32_t>,std::vector<uint32_t> > OctTree::GetTriangleList()
{
	std::vector<Point3D> vertices;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> depth;

	GatherTriangles(m_root.get(), vertices, indices,depth);

	return std::tuple< std::vector<Point3D>, std::vector<uint32_t>,std::vector<uint32_t> >(vertices,indices,depth);
}

std::tuple<std::vector<AABB>, std::vector<uint32_t>> OctTree::GetActiveBoxList()
{
	std::vector<AABB> boxes; std::vector<uint32_t> depth;

	boxes.push_back(m_root->box);
	depth.push_back(0);
	GatherBox(m_root.get(), boxes, depth);

	return std::tuple< std::vector<AABB>, std::vector<uint32_t> >(boxes,depth);
}

void OctTree::Rebuild()
{
	for (size_t i = 0; i < s_num_children; i++) m_boxesInsertCnt[i] = 0;
	
	m_root.reset(nullptr);
	m_trianglesSaved = 0;
	m_trianglesRemaining = m_indices.size() / 3;

	m_root = std::make_unique<OctNode>();

	oGFX::BV::BoundingAABB(m_root->box, m_vertices);
	AABB& box = m_root->box;
	float max = std::max({ box.halfExt.x,box.halfExt.y,box.halfExt.z });
	box.halfExt = Point3D(max, max, max);

	SplitNode(m_root.get(), m_root->box, m_vertices, m_indices);

	//for (size_t i = 0; i < s_num_children; i++)
	//	std::cout << "Inserted into box [" << i << "] - " << m_boxesInsertCnt[i] << " times\n";
}

void OctTree::SetTriangles(int maxTrianges)
{
	m_maxNodesTriangles = maxTrianges;
	m_maxNodesTriangles = std::max(static_cast<uint32_t>(m_maxNodesTriangles) , s_stop_triangles);
}

int OctTree::GetTriangles()
{
	return m_maxNodesTriangles;
}

float OctTree::progress()
{
	return static_cast<float>(m_trianglesSaved)/m_trianglesRemaining;
}

uint32_t OctTree::size() const
{
	return m_nodes;
}

void OctTree::SplitNode(OctNode* node, const AABB& box, const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices)
{
	const uint32_t currDepth = node->depth + 1;
	const uint32_t numTriangles = static_cast<uint32_t>(indices.size()) / 3;

	if (currDepth > m_maxDepth
		|| (numTriangles <= m_maxNodesTriangles))
	{
		node->type = OctNode::LEAF;
		node->nodeID = ++m_nodes;
		if (currDepth > s_stop_depth)
		{
			//std::cout << "Depth limit reached!" << std::endl;
		}
		m_trianglesSaved += numTriangles;
		node->vertices =(vertices);
		node->indices = (indices);
	}
	else
	{
		node->type = OctNode::INTERNAL;
		const Plane xPlane({ 1.0f,0.0f,0.0f }, box.center.x);
		const Plane yPlane({ 0.0f,1.0f,0.0f }, box.center.y);
		const Plane zPlane({ 0.0f,0.0f,1.0f }, box.center.z);

		// xsplit
		std::vector<Point3D> positiveVerts;
		std::vector<uint32_t> positiveIndices;
		std::vector<Point3D> negativeVerts;
		std::vector<uint32_t> negativeIndices;
		PartitionTrianglesAlongPlane(vertices, indices, xPlane,positiveVerts,positiveIndices,negativeVerts,negativeIndices);

		// ysplit
		std::vector<Point3D> lowerPositiveVerts;
		std::vector<uint32_t> lowerPositiveIndices;
		std::vector<Point3D> upperPositiveVerts;
		std::vector<uint32_t> upperPositiveIndices;
		PartitionTrianglesAlongPlane(positiveVerts, positiveIndices, yPlane,upperPositiveVerts,upperPositiveIndices,lowerPositiveVerts,lowerPositiveIndices);

		std::vector<Point3D> lowerNegativeVerts;
		std::vector<uint32_t> lowerNegativeIndices;
		std::vector<Point3D> upperNegativeVerts;
		std::vector<uint32_t> upperNegativeIndices;
		PartitionTrianglesAlongPlane(negativeVerts, negativeIndices, yPlane,upperNegativeVerts,upperNegativeIndices,lowerNegativeVerts,lowerNegativeIndices);

		// zsplit
		std::vector<Point3D> octantVerts[s_num_children];
		std::vector<uint32_t> octantInds[s_num_children];
		PartitionTrianglesAlongPlane(lowerPositiveVerts, lowerPositiveIndices, zPlane, octantVerts[5], octantInds[5], octantVerts[1], octantInds[1]);
		PartitionTrianglesAlongPlane(upperPositiveVerts, upperPositiveIndices, zPlane, octantVerts[7], octantInds[7], octantVerts[3], octantInds[3]);
		PartitionTrianglesAlongPlane(lowerNegativeVerts, lowerNegativeIndices, zPlane, octantVerts[4], octantInds[4], octantVerts[0], octantInds[0]);
		PartitionTrianglesAlongPlane(upperNegativeVerts, upperNegativeIndices, zPlane, octantVerts[6], octantInds[6], octantVerts[2], octantInds[2]);

		//if (currDepth < 3)
		//{
		//	std::cout << currDepth << "- start tris[" << numTriangles << "] " << std::endl;
		//	size_t thisTotalTri{};
		//	for (size_t i = 0; i < s_num_children; i++)
		//	{
		//		thisTotalTri += static_cast<size_t>(octantInds[i].size()) / 3;
		//		std::cout << "oct_" << i << " tris[" << octantInds[i].size() / 3<< "] " << std::endl;
		//	}
		//	std::cout << "\ttotal:" << thisTotalTri << std::endl;
		//}

		Point3D position;
		float step = box.halfExt.x * 0.5f;
		assert(step != 0.0f);
		for (size_t i = 0; i < s_num_children; i++)
		{
			position.x = ((i & 1) ? step : -step);
			position.y = ((i & 2) ? step : -step);
			position.z = ((i & 4) ? step : -step);
			AABB childBox;
			childBox.center = box.center + position;
			childBox.halfExt = Point3D{ step,step,step };

			if(octantVerts->size())
				++m_boxesInsertCnt[i];
			node->children[i] = std::make_unique<OctNode>();
			node->children[i]->depth = currDepth;
			//node->children[i]->nodeID = ++m_nodes;
			node->children[i]->box = childBox;
			SplitNode(node->children[i].get(),childBox, octantVerts[i], octantInds[i]);
			//if (octantVerts[i].size())
			//{
			//	
			//}
		}
	}
	//std::cout << "Depth:" << currDepth << " Tris:" << numTriangles << std::endl;

}

void OctTree::PartitionTrianglesAlongPlane(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices, const Plane& plane,
	std::vector<Point3D>& positiveVerts, std::vector<uint32_t>& positiveIndices,
	std::vector<Point3D>& negativeVerts, std::vector<uint32_t>& negativeIndices)
{
	const uint32_t numTriangles = static_cast<uint32_t>(indices.size()) / 3;

	for (size_t i = 0; i < numTriangles; i++)
	{
		Point3D v[3];
		v[0] = vertices[indices[i*3 + 0]];
		v[1] = vertices[indices[i*3 + 1]];
		v[2] = vertices[indices[i*3 + 2]];

		Triangle t(v[0], v[1], v[2]);

		auto orientation = oGFX::BV::ClassifyTriangleToPlane(t, plane);

		switch (orientation)
		{
		case TriangleOrientation::COPLANAR:
		case TriangleOrientation::POSITIVE:
		{
			auto index = static_cast<uint32_t>(positiveVerts.size());
			for (uint32_t i = 0; i < 3; i++)
			{
				positiveVerts.push_back(v[i]);
				positiveIndices.push_back(index + i);
			}
		}
		break;
		case TriangleOrientation::NEGATIVE:
		{
			auto index = static_cast<uint32_t>(negativeVerts.size());
			for (uint32_t i = 0; i < 3; i++)
			{
				negativeVerts.push_back(v[i]);
				negativeIndices.push_back(index + i);
			}
		}
		break;
		case TriangleOrientation::STRADDLE:
		{
			//auto index = static_cast<uint32_t>(positiveVerts.size());
			//for (uint32_t i = 0; i < 3; i++)
			//{
			//	positiveVerts.push_back(v[i]);
			//	positiveIndices.push_back(index + i);
			//}
			++m_TrianglesSliced;
			m_trianglesRemaining += oGFX::BV::SliceTriangleAgainstPlane(t, plane,
				positiveVerts, positiveIndices,
				negativeVerts,negativeIndices);
			m_trianglesRemaining-=1;
		}
		break;
		}
		
	}
}

void OctTree::GatherTriangles(OctNode* node, std::vector<Point3D>& outVertices, std::vector<uint32_t>& outIndices,std::vector<uint32_t>& depth)
{
	if (node == nullptr) return;

	if (node->type == OctNode::LEAF)
	{
		const auto triCnt = node->indices.size() / 3;
		for (size_t i = 0; i < triCnt; i++)
		{
			const auto anchor = static_cast<uint32_t>(outVertices.size());
			depth.push_back(node->nodeID);

			Point3D v[3];
			v[0] = node->vertices[node->indices[i*3 + 0]];
			v[1] = node->vertices[node->indices[i*3 + 1]];
			v[2] = node->vertices[node->indices[i*3 + 2]];

			for (uint32_t j = 0; j < 3; j++)
			{
				outVertices.push_back(v[j]);
				outIndices.push_back(anchor + j);
			}
		}
	}
	else
	{
		for (size_t i = 0; i < s_num_children; i++)
		{
			if (node->children[i])
			{
				GatherTriangles(node->children[i].get(),outVertices,outIndices,depth);
			}
		}
	}	

}

void OctTree::GatherBox(OctNode* node, std::vector<AABB>& boxes, std::vector<uint32_t>& depth)
{
	if (node == nullptr) return;

	if (node->type == OctNode::LEAF)
	{
		boxes.push_back(node->box);
		depth.push_back(node->depth);
	}
	for (size_t i = 0; i < s_num_children; i++)
	{
		GatherBox(node->children[i].get(), boxes, depth);
	}

}
