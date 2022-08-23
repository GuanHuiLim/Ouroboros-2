#include "BspTree.h"
#include "BoudingVolume.h"
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <string>

BspTree::BspTree()
{
	m_maxNodesTriangles = 400;
}

BspTree::BspTree(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices, int maxTriangles)
{
	Init(vertices, indices, maxTriangles);
}

void BspTree::Init(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices, int maxTriangles)
{
	m_vertices = vertices;
	m_indices = indices;
	m_maxNodesTriangles = maxTriangles;
}

std::tuple< std::vector<Point3D>, std::vector<uint32_t>,std::vector<uint32_t> > BspTree::GetTriangleList()
{
	std::vector<Point3D> vertices;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> depth;

	GatherTriangles(m_root.get(), vertices, indices,depth);

	return std::tuple< std::vector<Point3D>, std::vector<uint32_t>,std::vector<uint32_t> >(vertices,indices,depth);
}

void BspTree::Rebuild()
{
	std::cout << "building using " << GetPartitionTypeString() << std::endl;
	for (size_t i = 0; i < s_num_children; i++) m_planePartitionCount[i] = 0;

	m_root.reset(nullptr);
	m_root = std::make_unique<BspNode>();
	m_swapToAutoPartition = false;

	uint32_t expectedTriangles = m_indices.size() / 3;
	m_classificationScale = 0;
	while (expectedTriangles > m_maxNodesTriangles)
	{
		expectedTriangles /= 2;
		++m_classificationScale;
	}
	m_classificationScale *=  m_indices.size() / 3;
	m_trianglesClassified = 0;
	
	m_trianglesSaved = 0;
	m_trianglesRemaining = m_indices.size() / 3;

	std::filesystem::path fileName = "tree/bsptree_" + std::to_string(m_maxNodesTriangles) + "_"
		+ GetPartitionTypeString()		
		+".txt";
		std::cout << fileName << std::endl;
	if (std::filesystem::exists(fileName))
	{
		std::cout << "Found matching tree file.. attempting to load" << std::endl;
		if (LoadTree(fileName) != true)
		{
			//failed to load tree, generate one;
			SplitNode(m_root.get(), m_root->m_splitPlane, m_vertices, m_indices);
			// now reserialize
			SerializeTree();
		}
	}
	else
	{
		std::cout << "no matching file.. building" << std::endl;
		SplitNode(m_root.get(), m_root->m_splitPlane, m_vertices, m_indices);
		SerializeTree();
	}

	for (size_t i = 0; i < s_num_children; i++)
		std::cout << "Inserted into plane [" << (i?"positive" :"negative") << "] - " << m_planePartitionCount[i] << " times\n";
}

bool BspTree::LoadTree(const std::filesystem::path& path)
{
	m_swapToAutoPartition = false;
	auto fs = std::ifstream(path);

	uint32_t maxTriangles{};
	fs >> maxTriangles;

	int32_t type{};
	fs >> type;
	m_type == static_cast<PartitionType>(type);

	size_t numVert, numIndx;
	fs >> numVert;
	fs >> numIndx;

	if (numVert != m_vertices.size() || numIndx != m_indices.size())
	{
		std::cout << "Scene Changed! unable to load bsp tree from file\n";
		return false;
	}

	// we ready to load now we can save the variables
	m_maxNodesTriangles = maxTriangles;

	std::vector<Plane> planes;
	while (fs)
	{
		Plane currPlane;
		for (size_t i = 0; i < 4; i++)
		{
			fs >> currPlane.normal[i];
			fs.ignore();
		}		
		planes.push_back(currPlane);
	}

	uint32_t index{};
	LoadNode(m_root.get(), planes, index, m_vertices, m_indices);
	std::cout << "Tree loaded" << std::endl;

	for (size_t i = 0; i < s_num_children; i++)
		std::cout << "Inserted into plane [" << (i?"positive" :"negative") << "] - " << m_planePartitionCount[i] << " times\n";

	return true;
}

void BspTree::SetTriangles(int maxTrianges)
{
	m_maxNodesTriangles = maxTrianges;
	m_maxNodesTriangles = std::max(static_cast<uint32_t>(m_maxNodesTriangles) , s_stop_triangles);
}

int BspTree::GetTriangles()
{
	return m_maxNodesTriangles;
}

bool BspTree::BuildSerialized(const std::string& path)
{
	std::filesystem::path fileName = path;
	if (std::filesystem::exists(fileName))
	{
		std::cout << "Found matching tree file.. attempting to load" << std::endl;
		if (LoadTree(fileName) != true)
		{
			std::cout << "Failed to load! Building.." << std::endl;
			//failed to load tree, generate one;
			SplitNode(m_root.get(), m_root->m_splitPlane, m_vertices, m_indices);
			// now reserialize
			SerializeTree();
		}
		return true;
	}
	return false;
}

float BspTree::progress()
{
	float classifyingProgress = static_cast<float>(m_trianglesClassified)/m_classificationScale;
	float totalProgress{ static_cast<float>(m_trianglesSaved) / m_trianglesRemaining };

	return classifyingProgress * 0.9f + totalProgress * 0.1f;
}

uint32_t BspTree::size() const
{
	return m_nodes;
}

void BspTree::SerializeTree()
{
	std::filesystem::path fileName = "tree/bsptree_" + std::to_string(m_maxNodesTriangles) + "_"
		+ GetPartitionTypeString()
		+".txt";
	if (std::filesystem::exists(fileName) == false)
	{
		std::filesystem::create_directory("tree/");
	}
	auto fs = std::ofstream(fileName);

	fs << std::to_string(m_maxNodesTriangles) << std::endl;
	fs << std::to_string(static_cast<int>(m_type)) << std::endl;
	fs << std::to_string(m_vertices.size()) << std::endl;
	fs << std::to_string(m_indices.size()) << std::endl;
	
	std::vector<Plane> planes;
	planes.reserve(m_nodes);

	GatherPlanes(m_root.get(), planes);
	for (size_t i = 0; i < planes.size(); i++)
	{
		fs 	<< planes[i].normal.x << ","
			<< planes[i].normal.y << ","
			<< planes[i].normal.z << ","
			<< planes[i].normal.w << ","
			<< std::endl;
	}

	fs.close();
	std::cout << "Tree serialized at [" << fileName.u8string() << "]\n";
}

void BspTree::SetPartitionType(PartitionType type)
{
	m_type = type;
	std::cout << "Set type to: " << (m_type == PartitionType::MEAN ? "Mean" : "Autopart") << std::endl;
}

BspTree::PartitionType BspTree::GetPartitionType()
{
	return m_type;
}

void BspTree::SplitNode(BspNode* node, const Plane& plane, const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices)
{
	const uint32_t currDepth = node->depth + 1;
	const uint32_t numTriangles = static_cast<uint32_t>(indices.size()) / 3;
	
	if (currDepth > m_maxDepth
		|| (numTriangles <= m_maxNodesTriangles))
	{
		node->type = BspNode::LEAF;
		node->nodeID = ++m_nodes;

		m_trianglesSaved += numTriangles;
		node->vertices =(vertices);
		node->indices = (indices);
	}
	else
	{
		node->type = BspNode::INTERNAL;

		// Get a nice splitting plane
		Plane splittingPlane = PickSplittingPlane(vertices, indices);
		node->m_splitPlane = splittingPlane;

		// planesplit
		std::vector<Point3D> splitVerts[s_num_children];
		std::vector<uint32_t> splitIndices[s_num_children];
		PartitionTrianglesAlongPlane(vertices, indices, splittingPlane,splitVerts[0],splitIndices[0],splitVerts[1],splitIndices[1]);

		// Split and recurse
		for (size_t i = 0; i < s_num_children; i++)
		{
			if(splitVerts[i].size())
				m_planePartitionCount[i] += splitVerts->size()/3;

			node->children[i] = std::make_unique<BspNode>();
			node->children[i]->depth = currDepth;

			SplitNode(node->children[i].get(),splittingPlane, splitVerts[i], splitIndices[i]);
		}
	}

}

void BspTree::PartitionTrianglesAlongPlane(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices, const Plane& plane,
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
			++m_TrianglesSliced;
			m_trianglesRemaining += oGFX::BV::SliceTriangleAgainstPlane(t, plane,
				positiveVerts, positiveIndices,
				negativeVerts,negativeIndices);
			m_trianglesRemaining -= 1;
		}
		break;
		}

	}
}

void BspTree::LoadNode(BspNode* node, const std::vector<Plane>& planes, uint32_t& index, const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices)
{
	const uint32_t currDepth = node->depth + 1;
	const uint32_t numTriangles = static_cast<uint32_t>(indices.size()) / 3;

	if (currDepth > m_maxDepth
		|| (numTriangles <= m_maxNodesTriangles))
	{
		node->type = BspNode::LEAF;
		node->nodeID = ++m_nodes;

		m_trianglesSaved += numTriangles;
		node->vertices =(vertices);
		node->indices = (indices);
	}
	else
	{
		node->type = BspNode::INTERNAL;

		// Get a nice splitting plane
		Plane splittingPlane = planes[index++];
		node->m_splitPlane = splittingPlane;

		// planesplit
		std::vector<Point3D> splitVerts[s_num_children];
		std::vector<uint32_t> splitIndices[s_num_children];
		PartitionTrianglesAlongPlane(vertices, indices, splittingPlane,splitVerts[0],splitIndices[0],splitVerts[1],splitIndices[1]);

		// Split and recurse
		for (size_t i = 0; i < s_num_children; i++)
		{
			if(splitVerts[i].size())
				m_planePartitionCount[i] += splitVerts->size()/3;

			node->children[i] = std::make_unique<BspNode>();
			node->children[i]->depth = currDepth;

			LoadNode(node->children[i].get(),planes,index, splitVerts[i], splitIndices[i]);
		}
	}

}

Plane BspTree::PickSplittingPlane(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices)
{	
	if (m_type != BspTree::PartitionType::AUTOPARTITION && m_swapToAutoPartition)
	{
		//swap each iteration
		m_swapToAutoPartition = false;
		return AutoPartition(vertices,indices);
	}
	else
	{
		m_swapToAutoPartition = true;
	}

	switch (m_type)
	{
	case BspTree::PartitionType::AUTOPARTITION:
	return AutoPartition(vertices,indices);
	break;
	case BspTree::PartitionType::MEAN:
	return MeanPartition(vertices, indices);
	break;
	case BspTree::PartitionType::AXIS_DICT:
	return AxisPartition(vertices, indices);
	break;
	default:
	std::cout << "Unable to find partition type.. using Autopartition\n";
	return AutoPartition(vertices,indices);
	break;
	}

}

Plane BspTree::AutoPartition(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices)
{
	Plane bestPlane;

	// Blend factor for optimizing for balance or splits (should be tweaked)
	const float K = 0.8f;
	// Variables for tracking best splitting plane seen so far
	float bestScore = FLT_MAX;
	// Try the plane of each polygon as a dividing plane
	size_t numTriangles = indices.size() / 3;

	for (int i = 0; i < numTriangles; ++i) {
		int numInFront = 0, numBehind = 0, numStraddling = 0;

		Point3D v1[3];
		v1[0] = vertices[indices[i*3 + 0]];
		v1[1] = vertices[indices[i*3 + 1]];
		v1[2] = vertices[indices[i*3 + 2]];
		Triangle t1(v1[0], v1[1], v1[2]);

		Plane plane = oGFX::BV::PlaneFromTriangle(t1);
		// Test against all other polygons
		for (int j = 0; j < numTriangles; j++) {
			// Ignore testing against self
			if (i == j) continue;

			Point3D v2[3];
			v2[0] = vertices[indices[j*3 + 0]];
			v2[1] = vertices[indices[j*3 + 1]];
			v2[2] = vertices[indices[j*3 + 2]];
			Triangle t2(v2[0], v2[1], v2[2]);
			// Keep standing count of the various poly-plane relationships

			switch (oGFX::BV::ClassifyTriangleToPlane(t2, plane)) 
			{
			case TriangleOrientation::COPLANAR:
			/* Coplanar polygons treated as being in front of plane */
			case TriangleOrientation::POSITIVE:
			numInFront++;
			break;
			case TriangleOrientation::NEGATIVE:
			numBehind++;
			break;
			case TriangleOrientation::STRADDLE:
			numStraddling++;
			break;
			}


		}
		// Compute score as a weighted combination (based on K, with K in range
		// 0..1) between balance and splits (lower score is better)

		++m_trianglesClassified;
		float score=K* numStraddling + (1.0f - K) * std::abs(numInFront - numBehind);
		if (score < bestScore) {
			bestScore = score;
			bestPlane = plane;
		}
	}
	//std::cout << "return split\n";
	return bestPlane;
}

Plane BspTree::MeanPartition(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices)
{
	Plane bestPlane;

	glm::vec3 mean;
	for (size_t i = 0; i < vertices.size(); i++)
	{
		mean += vertices[i];
	}
	mean /= vertices.size();

	glm::vec3 big[3];
	float f1{ -FLT_MAX }, f2{ -FLT_MAX }, f3{ -FLT_MAX };
	for (auto& v : vertices)
	{
		float val = glm::dot(v - mean, v - mean);
		if (val > f1)
		{
			f3 = f2;
			big[2] = big[1];
			f2 = f1;
			big[1] = big[0];

			f1 = val;
			big[0] = v;
		}
		else if (val > f2)
		{
			f3 = f2;
			big[2] = big[1];
			f2 = val;
			big[1] = v;
		}
		else if (val > f3)
		{
			f3 = val;
			big[2] = v;
		}
	}

	Plane p[3];
	for (size_t i = 0; i < 3; i++)
	{
		p[i].normal = { glm::normalize(big[i] - mean) , 0.0f };
		p[i].normal.w = std::sqrtf(std::abs(glm::dot(glm::vec3{ p[i].normal }, mean)));
	}
	

	int numInFront[3]{0}, numBehind[3]{0};
	for (size_t i = 0; i < vertices.size(); i++)
	{
		for (size_t x = 0; x < 3; x++)
		{
			switch (oGFX::BV::ClassifyPointToPlane(vertices[i], p[x]))
			{
			case COPLANAR:
			case POSITIVE:
			++numInFront[x];
			break;
			case NEGATIVE:
			++numBehind[x];
			break;
			}	
		}			
	}
	int32_t biggest = numInFront[0] * numBehind[0];
	int32_t test = numInFront[1] * numBehind[1];
	if (test > biggest)
	{
		biggest = test;
		test = numInFront[2] * numBehind[2];
		if (test > biggest) return p[2];
		else return p[1];
	}
	else
	{
		test = numInFront[2] * numBehind[2];
		if (test > biggest) return p[2];
		else return p[0];
	}	

	return p[0];
}

Plane BspTree::AxisPartition(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices)
{
	static std::vector<glm::vec3> axes;
	static auto once = [&]() { axes = oGFX::BV::GetAxisFromDictionary(3);
								std::for_each(axes.begin(), axes.end(), [](glm::vec3& v) { glm::normalize(v); });
								return true; 
							}();
	Plane bestPlane;
	glm::vec3 mean = std::accumulate(vertices.begin(), vertices.end(), glm::vec3{}) / (float)vertices.size();

	uint32_t numTris = indices.size() / 3;
	uint32_t best = 0;
	int numInfront = 0, numBehind = 0;
	Plane currPlane;
	for (size_t i = 0; i < axes.size(); i++)
	{
		currPlane = { axes[i], glm::length(glm::dot(axes[i],mean)) };
		int ni = 0, nb = 0;
		for (size_t v = 0; v < vertices.size(); v++)
		{			
			switch (oGFX::BV::ClassifyPointToPlane(vertices[i], currPlane))
			{
			case COPLANAR:
			case POSITIVE:
			++ni;
			break;
			case NEGATIVE:
			++nb;
			break;
				
			}		
		}
		if (ni * nb > numInfront * numBehind)
		{
			bestPlane = currPlane;
		}
	}

	return bestPlane;

}

void BspTree::GatherPlanes(BspNode* node, std::vector<Plane>& planes)
{
	if (node == nullptr) return;

	planes.push_back(node->m_splitPlane);

	for (size_t i = 0; i < s_num_children; i++)
	{
		GatherPlanes(node->children[i].get(), planes);
	}
}

void BspTree::GatherTriangles(BspNode* node, std::vector<Point3D>& outVertices, std::vector<uint32_t>& outIndices,std::vector<uint32_t>& depth)
{
	if (node == nullptr) return;

	if (node->type == BspNode::LEAF)
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

std::string BspTree::GetPartitionTypeString()
{
	switch (m_type)
	{
	case BspTree::PartitionType::AUTOPARTITION:
	return "auto";
	break;
	case BspTree::PartitionType::MEAN:
	return "mean";
	break;
	case BspTree::PartitionType::AXIS_DICT:
	return "dict";
	break;
	default:
	return "Error";
	break;
	}
}
