/************************************************************************************//*!
\file           BspTree.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              BSP for assignment

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "MathCommon.h"
#include "Geometry.h"

#include <vector>
#include <tuple>
#include <memory>
#include <filesystem>

struct BspNode;

class BspTree
{
public:
	inline static constexpr uint32_t s_num_children = 2;
	inline static constexpr uint32_t s_stop_depth = 15;
	inline static constexpr uint32_t s_stop_triangles = 30;

	enum class PartitionType
	{
		AUTOPARTITION=0,
		MEAN=1,
		AXIS_DICT=2,
	};
public:
	BspTree();
	BspTree(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices, int maxTrangles = s_stop_triangles);
	void Init(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices, int maxTrangles = s_stop_triangles);

	std::tuple< std::vector<Point3D>, std::vector<uint32_t>,std::vector<uint32_t> > GetTriangleList();

	void Rebuild();
	void SetTriangles(int maxTrianges);
	int GetTriangles();

	bool BuildSerialized(const std::string& path);
	
	float progress();
	uint32_t size() const;

	void SerializeTree();

	void SetPartitionType(PartitionType type);
	PartitionType GetPartitionType();
	std::string GetPartitionTypeString();

	uint32_t m_trianglesSaved{};
	uint32_t m_trianglesRemaining{};
	uint32_t m_classificationScale{};
	uint32_t m_trianglesClassified{};
	uint32_t m_maxNodesTriangles{ s_stop_triangles };

private:
	std::unique_ptr<BspNode> m_root{};

	PartitionType m_type{PartitionType :: AUTOPARTITION};
	uint32_t m_nodes{};
	uint32_t m_TrianglesSliced{};
	std::vector<Point3D> m_vertices; std::vector<uint32_t> m_indices;
	uint32_t m_maxDepth{ s_stop_depth };

	bool m_swapToAutoPartition = false;

	uint32_t m_planePartitionCount[s_num_children]{};
	bool LoadTree(const std::filesystem::path& path);

	void SplitNode(BspNode* node,const Plane& plane,const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices);
	void PartitionTrianglesAlongPlane(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices,const Plane& plane,
		std::vector<Point3D>& positiveVerts, std::vector<uint32_t>& positiveIndices,
		std::vector<Point3D>& negativeVerts, std::vector<uint32_t>& negativeIndices
	);

	void LoadNode(BspNode* node,const std::vector<Plane>& planes, uint32_t& index,const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices);
	Plane PickSplittingPlane(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices);

	Plane AutoPartition(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices);
	Plane MeanPartition(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices);
	Plane AxisPartition(const std::vector<Point3D>& vertices, const std::vector<uint32_t>& indices);

	void GatherPlanes(BspNode* node, std::vector<Plane>& planes);
	void GatherTriangles(BspNode* node,std::vector<Point3D>& vertices, std::vector<uint32_t>& indices,std::vector<uint32_t>& depth);

};

struct BspNode
{
	enum Type
	{
		INTERNAL,
		LEAF,
	}type{ Type::INTERNAL };

	Plane m_splitPlane{};

	uint32_t depth{};
	uint32_t nodeID{};

	std::vector<Point3D> vertices;
	std::vector<uint32_t> indices;
	std::unique_ptr<BspNode> children[BspTree::s_num_children];
};
