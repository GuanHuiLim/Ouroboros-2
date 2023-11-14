/************************************************************************************//*!
\file           OctTree.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Octtree for assignment

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "MathCommon.h"
#include "Geometry.h"
#include "VulkanUtils.h"

#include <vector>
#include <memory>

struct ObjectInstance;
namespace oGFX {

struct OctNode;
struct NodeEntry;

class OctTree
{
public:
	inline static constexpr uint32_t s_num_children = 8;
	inline static constexpr uint32_t s_stop_depth = 5;
public:
	OctTree(const AABB rootBox = { Point3D{-500.0f},Point3D{500.0f} } ,int stopDepth = s_stop_depth);

	void Insert(ObjectInstance* entity, AABB box);

	void Remove(ObjectInstance* entity);

	void GetActiveBoxList(std::vector<AABB>& boxes, std::vector<uint32_t>& depth);
	void GetEntitiesInFrustum(const Frustum& frust, std::vector<ObjectInstance*>& contains, std::vector<ObjectInstance*>& intersect);
	void GetBoxesInFrustum(const Frustum& frust, std::vector<AABB>& contains, std::vector<AABB>& intersect);

	void ClearTree();
	void ResizeTree(const AABB& box);
	uint32_t size() const;

private:
	std::unique_ptr<OctNode> m_root{};

	uint32_t m_entitiesCnt{};
	uint32_t m_maxDepth{ s_stop_depth };

	uint32_t m_nodes{};
	uint32_t m_boxesInsertCnt[s_num_children];

	void GatherBoxWithDepth(OctNode* node,std::vector<AABB>& boxes, std::vector<uint32_t>& depth);
	void GatherBox(OctNode* node,std::vector<AABB>& boxes);
	void GatherEntities(OctNode* node,std::vector<ObjectInstance*>& entities, std::vector<uint32_t>& depth);
	void GatherFrustBoxes(OctNode* node, const Frustum& frust,std::vector<AABB>& contained, std::vector<AABB>& intersect);
	void GatherFrustEntities(OctNode* node, const Frustum& frust,std::vector<ObjectInstance*>& contained, std::vector<ObjectInstance*>& intersect);
	
	void PerformInsert(OctNode* node, const NodeEntry& entry);
	bool PerformRemove(OctNode* node, const NodeEntry& entry);
	void SplitNode(OctNode* node);
	void PerformClear(OctNode* node);

	void ResizeTreeBounds(OctNode* node, const AABB& box);
};

struct OctNode
{
	AABB box{};
	uint32_t depth{};
	uint32_t nodeID{};

	std::vector<NodeEntry> entities;
	std::unique_ptr<OctNode> children[OctTree::s_num_children];
};

struct NodeEntry 
{
	ObjectInstance* obj{ nullptr };
	AABB box{};
};

}// end namespace oGFX