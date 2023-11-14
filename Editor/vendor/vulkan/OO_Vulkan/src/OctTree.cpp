/************************************************************************************//*!
\file           OctTree.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief               Octtree for assignment

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "OctTree.h"
#include "BoudingVolume.h"
#include "Collision.h"
#include "GraphicsWorld.h"
#include "Profiling.h"
#include <algorithm>
#include <iostream>

glm::vec3 g_max{-1e10};
glm::vec3 g_min{ 1e10 };

namespace oGFX {

	OctTree::OctTree(const AABB rootBox, int stopDepth)
{
	m_maxDepth = stopDepth;
	m_root.reset(nullptr);
	m_root = std::make_unique<OctNode>();
	m_root->box = rootBox;

}

void OctTree::Insert(ObjectInstance* entity, AABB objBox)
{
	PROFILE_SCOPED();
	NodeEntry entry;
	entry.box = objBox;
	entry.obj = entity;
	PerformInsert(m_root.get(), entry);

	auto bmax = entry.box.max();
	auto bmin = entry.box.min();

	g_max.x = std::max(g_max.x, bmax.x);
	g_max.y = std::max(g_max.y, bmax.y);
	g_max.z = std::max(g_max.z, bmax.z);

	g_min.x = std::min(g_min.x, bmin.x);
	g_min.y = std::min(g_min.y, bmin.y);
	g_min.z = std::min(g_min.z, bmin.z);
}

void OctTree::Remove(ObjectInstance* entity)
{
	PROFILE_SCOPED();
	OctNode* node = entity->treeNode;
	//OO_ASSERT(node && "Entity did not record the node");
	if (node == nullptr) return; // what could go wrong

	auto it = std::find_if(node->entities.begin(), node->entities.end(), [chk = entity](const NodeEntry& e) { return e.obj == chk; });
	if (it != node->entities.end()) 
	{
		std::swap(*it, node->entities.back());
		node->entities.pop_back();
		entity->treeNode = nullptr;
		--m_nodes;
		return;
	}

	OO_ASSERT(false && "Entity does not exist in node it points to");
}

void OctTree::GetEntitiesInFrustum(const Frustum& frust, std::vector<ObjectInstance*>& contained, std::vector<ObjectInstance*>& intersecting)
{
	PROFILE_SCOPED();
	GatherFrustEntities(m_root.get(), frust, contained, intersecting);	
}

void OctTree::GetActiveBoxList(std::vector<AABB>& boxes, std::vector<uint32_t>& depth)
{
	boxes.push_back(m_root->box);
	depth.push_back(0);
	GatherBoxWithDepth(m_root.get(), boxes, depth);
}

void OctTree::GetBoxesInFrustum(const Frustum& frust, std::vector<AABB>& contains, std::vector<AABB>& intersects)
{	
	GatherFrustBoxes(m_root.get(), frust, contains, intersects);
}

void OctTree::ClearTree()
{
	PerformClear(m_root.get());
	if (m_root->box.max() != g_max || m_root->box.min() != g_min) {
		ResizeTree(AABB{g_min,g_max});
		g_max = glm::vec3{ FLT_MIN };
		g_min = glm::vec3{ FLT_MAX };
	}
	m_nodes = 0;
}

void OctTree::ResizeTree(const AABB& box)
{
	PROFILE_SCOPED();
	ResizeTreeBounds(m_root.get(), box);
}

uint32_t OctTree::size() const
{
	return m_nodes;
}

void OctTree::GatherBoxWithDepth(OctNode* node, std::vector<AABB>& boxes, std::vector<uint32_t>& depth)
{
	if (node == nullptr) return;

	if (node->entities.size())
	{
		boxes.push_back(node->box);
		depth.push_back(node->depth);
	}
	for (size_t i = 0; i < s_num_children; i++)
	{
		GatherBoxWithDepth(node->children[i].get(), boxes, depth);
	}
}

void OctTree::GatherBox(OctNode* node, std::vector<AABB>& boxes)
{
	if (node == nullptr) return;

	if (node->entities.size())
		boxes.push_back(node->box);
	for (size_t i = 0; i < s_num_children; i++)
	{
		GatherBox(node->children[i].get(), boxes);
	}
}

void OctTree::GatherEntities(OctNode* node, std::vector<ObjectInstance*>& entities, std::vector<uint32_t>& depth)
{
	if (node == nullptr) return;
	
	for (size_t i = 0; i < node->entities.size(); i++)
	{
		entities.push_back(node->entities[i].obj);
		depth.push_back(node->depth);
	}		
	
	for (size_t i = 0; i < s_num_children; i++)
	{
		GatherEntities(node->children[i].get(), entities, depth);
	}
}

void OctTree::GatherFrustBoxes(OctNode* node, const Frustum& frust, std::vector<AABB>& contained, std::vector<AABB>& intersect)
{	
	if (node == nullptr) return;

	oGFX::coll::Collision result = oGFX::coll::AABBInFrustum(frust, node->box);
	switch (result)
	{
	case oGFX::coll::INTERSECTS:
		// add to testing list
		//if(node->entities.size())
			intersect.push_back(node->box);		
		break;
	case oGFX::coll::CONTAINS:
		// add to contained list
		//if (node->entities.size())
			contained.push_back(node->box);
		break;
	case oGFX::coll::OUTSIDE:
		// no action needed
		break;
	}
	// check each child
	for (size_t i = 0; i < s_num_children; i++)
	{
		GatherFrustBoxes(node->children[i].get(), frust, contained, intersect);
	}
}

void OctTree::GatherFrustEntities(OctNode* node, const Frustum& frust, std::vector<ObjectInstance*>& contained, std::vector<ObjectInstance*>& intersect)
{
	if (node == nullptr) return;

	oGFX::coll::Collision result = oGFX::coll::AABBInFrustum(frust, node->box);
	switch (result)
	{
	case oGFX::coll::INTERSECTS:
		// add to testing list
		for (size_t i = 0; i < node->entities.size(); i++)
		{
			intersect.push_back(node->entities[i].obj);
		}		
		break;
	case oGFX::coll::CONTAINS:
		// add to contained list
		for (size_t i = 0; i < node->entities.size(); i++)
		{
			contained.push_back(node->entities[i].obj);
		}
		break;
	case oGFX::coll::OUTSIDE:
		// no action needed
		break;
	}
	// check each child
	for (size_t i = 0; i < s_num_children; i++)
	{
		GatherFrustEntities(node->children[i].get(), frust, contained, intersect);
	}
}

void OctTree::SplitNode(OctNode* node)
{
	const uint32_t currDepth = node->depth + 1;

	Point3D position{};
	float step = node->box.halfExt.x * 0.5f;
	assert(step != 0.0f);
	for (size_t i = 0; i < s_num_children; i++)
	{
		position.x = ((i & 1) ? step : -step);
		position.y = ((i & 2) ? step : -step);
		position.z = ((i & 4) ? step : -step);
		AABB childBox;
		childBox.center = node->box.center + position;
		childBox.halfExt = Point3D{ step,step,step };
		node->children[i] = std::make_unique<OctNode>();
		node->children[i]->depth = currDepth;
		node->children[i]->box = childBox;
	}

	// distribute entities
	std::vector<NodeEntry> entCpy = std::move(node->entities);
	for (const NodeEntry& e : entCpy)
	{
		PerformInsert(node, e);
	}
}

void OctTree::PerformInsert(OctNode* node,const NodeEntry& entry)
{
	OO_ASSERT(node);

	const uint32_t currDepth = node->depth + 1;
	// we reached max depth, turn into leaf
	if (currDepth > m_maxDepth)
	{
		node->nodeID = ++m_nodes;
		node->entities.push_back(entry);
		entry.obj->treeNode = node;
		return;
	}

	if (node->children[0] == nullptr) 
	{
		SplitNode(node);
	}
	for (size_t i = 0; i < s_num_children; i++)
	{
		if (oGFX::coll::AabbContains(node->children[i]->box, entry.box))
		{
			PerformInsert(node->children[i].get(), entry);			
			return;
		}
	}

	// not contained in any child
	node->nodeID = ++m_nodes;
	node->entities.push_back(entry);	
	entry.obj->treeNode = node;
}

bool OctTree::PerformRemove(OctNode* node, const NodeEntry& entry)
{
	if (node == nullptr) return false;

	auto it = std::find_if(node->entities.begin(), node->entities.end(), [chk = entry.obj](const NodeEntry& e) { return e.obj == chk; });

	// Check if the element was found
	if (it != node->entities.end()) {
		// Element found, erase it from the vector
		node->entities.erase(it);
		--m_nodes;
		return true;
	}
	else {
		// Element not found
		// search children
		for (size_t i = 0; i < s_num_children; i++)
		{
			if (PerformRemove(node->children[i].get(), entry))
			{
				return true;
			}
		}
	}

	return false;
}

void OctTree::PerformClear(OctNode* node)
{
	if (node == nullptr) return;

	for (size_t i = 0; i < s_num_children; i++)
	{
		PerformClear(node->children[i].get());
	}

	for (size_t i = 0; i < node->entities.size(); i++)
	{
		node->entities[i].obj->treeNode = nullptr;
	}
	node->entities.clear();
}

void OctTree::ResizeTreeBounds(OctNode* node, const AABB& box)
{
	if (node == nullptr) return;

	node->box = box;

	Point3D position{};
	float step = node->box.halfExt.x * 0.5f;
	for (size_t i = 0; i < s_num_children; i++)
	{
		position.x = ((i & 1) ? step : -step);
		position.y = ((i & 2) ? step : -step);
		position.z = ((i & 4) ? step : -step);
		AABB childBox;
		childBox.center = node->box.center + position;
		childBox.halfExt = Point3D{ step,step,step };
		ResizeTreeBounds(node->children[i].get(), childBox);
	}

}

}// end namespace oGFX