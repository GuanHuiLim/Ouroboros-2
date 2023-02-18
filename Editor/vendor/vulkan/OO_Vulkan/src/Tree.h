/************************************************************************************//*!
\file           Tree.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Tree structure for assignment

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Geometry.h"
#include "BoudingVolume.h"

#include <memory>
#include <vector>
#include <algorithm>

using BoundingVolume = oGFX::AABB;
static constexpr size_t numChilden = 2;

template<typename dataT,typename BVType>
std::vector<uint32_t> PartitionObjects(const std::vector<dataT>& entities,uint32_t objs[], uint32_t numObjects)
{
	std::vector<uint32_t> result;
	result.reserve(numChilden + 2);
	//result.push_back(0);

	std::sort(objs,objs+numObjects,[&](uint32_t a, uint32_t b) {		
			return entities[a].position.x > entities[b].position.x;
		});

	for (uint32_t i = 0; i < numChilden; i++)
	{
		result.push_back(numObjects / (uint32_t)numChilden * i);
		//auto& ent = entities[objs[i]];
		//float f = ent.GetBVHeuristic<AABB>();
	}

	result.push_back(numObjects);
	return result;
}

template <typename BVType>
struct TreeNode
{
	enum Type
	{
		INTERNAL,
		LEAF,
	}type;

	BVType BV;
	uint32_t depth{};
	
	std::vector<uint32_t> objects;
	std::unique_ptr<TreeNode<BVType>> children[numChilden];
};

template <typename T, typename BVType>
BVType makeBV(const std::vector<T>& entities, uint32_t objects[], uint32_t numOjbects) {
	if constexpr (std::is_same_v<BVType, oGFX::AABB>)
	{
		oGFX::AABB bv;

		glm::vec3 min{FLT_MAX};
		glm::vec3 max{-FLT_MAX};
		for (size_t i = 0; i < numOjbects; i++)
		{
			auto& ent = entities[objects[i]];
			auto eMax = ent.aabb.center + ent.aabb.halfExt;
			auto eMin = ent.aabb.center - ent.aabb.halfExt;
			for (uint32_t x = 0; x < 3; x++)
			{
				max[x] = std::max(eMax[x], max[x]);
				min[x] = std::min(eMin[x], min[x]);
			}
		}
		bv.center = (max + min) * 0.5f;
		bv.halfExt = max-bv.center;

		return bv;
	}
	if constexpr (std::is_same_v<BVType, oGFX::Sphere>)

	{
		oGFX::Sphere bv = entities[objects[0]].sphere;

		//float dist = 0;
		//glm::vec3 center{ 0.0f };
		for (size_t i = 1; i < numOjbects; i++)
		{
			const auto& otherSphere = entities[objects[i]].sphere;
			glm::vec3 dir = glm::normalize(otherSphere.center - bv.center);
			oGFX::BV::ExpandSphereAboutPoint(bv, otherSphere.center + dir * otherSphere.radius);
		}
		//center /= numOjbects;
		//max = (max - centre);
		//bv.radius = std::max({ max[0],max[1],max[2] });
		//bv.radius = std::sqrt(glm::dot(max - min, max - min));
		//bv.center = center;

		return bv;
	}
};

//template <typename T>
//AABB makeBV(const std::vector<T>& entities, uint32_t objects[], uint32_t numOjbects)
//{
//	
//}
//
//template <typename T>
//Sphere makeBV(const std::vector<T>& entities, uint32_t objects[], uint32_t numOjbects)
//{
//	
//}

template <typename dataT, typename BVType>
struct Tree
{
	std::unique_ptr<TreeNode<BVType>> root{};
public:
	void Clear()
	{
		root.reset(nullptr);
	}
	//Visitor structure
	template <typename dataT, typename BVType>
	static void TopDownTree(const std::vector<dataT>& entities, TreeNode<BVType>* parent, uint32_t objects[], uint32_t numOjbects)
	{
		constexpr uint32_t MIN_OBJECTS_AT_LEAF = 1;
		if (numOjbects <= MIN_OBJECTS_AT_LEAF)
		{
			BVType bv = makeBV<dataT, BVType>(entities, objects, numOjbects);
			parent->BV = bv;
			parent->type = TreeNode<BVType>::Type::LEAF;
			parent->objects.reserve(numOjbects);
			for (size_t i = 0; i < numOjbects; i++)
			{
				parent->objects.push_back(objects[i]);
			}
		}
		else
		{
			BVType bv = makeBV<dataT, BVType>(entities, objects, numOjbects);
			parent->BV = bv;
			parent->type = TreeNode<BVType>::Type::INTERNAL;
			parent->objects.reserve(numOjbects);
			for (size_t i = 0; i < numOjbects; i++)
			{
				parent->objects.push_back(objects[i]);
			}
			auto partitions = PartitionObjects<dataT, BVType >(entities, objects, numOjbects);
			for (size_t i = 0; i < numChilden; i++)
			{
				parent->children[i] = std::make_unique<TreeNode<BVType>>();
				parent->children[i]->depth = parent->depth + 1;
				uint32_t k = partitions[i];
				uint32_t n = partitions[i + 1];
				TopDownTree<dataT, BVType>(entities, parent->children[i].get(), &objects[k], n - k);
			}
		}
		
	}

	template <typename BVType>
	void mergeNodes(std::vector<std::unique_ptr<TreeNode<BVType>>>& nodes, std::unique_ptr<TreeNode<BVType>>& left, std::unique_ptr<TreeNode<BVType>>& right)
	{
		size_t other{};
		float val{ 0 };
		std::sort(nodes.begin(), nodes.end(), [](std::unique_ptr<TreeNode<BVType>>& l, std::unique_ptr<TreeNode<BVType>>& r)
			{
				return l->BV.center.x < r->BV.center.x;
			});

		for (size_t i = 1; i < nodes.size(); i++)
		{
			glm::vec3 v = nodes[other]->BV.center - nodes[i]->BV.center;
			float dist = glm::dot(v, v);
			if (dist > val)
			{
				val = dist;
				other = i;
			}
		}

		std::swap(nodes[0], left);
		std::swap(nodes[other], right);

		std::swap(nodes[other], nodes.back());
		nodes.pop_back();
		std::swap(nodes[0], nodes.back());
		nodes.pop_back();
	}

	template <typename dataT, typename BVType>
	void BottomUpTree(const std::vector<dataT>& entities, TreeNode<BVType>* parent, uint32_t objects[], uint32_t numOjbects)
	{
		std::vector<std::unique_ptr<TreeNode<BVType>>> nodes;
		for (auto& e : entities)
		{
			nodes.emplace_back(std::make_unique<TreeNode<BVType>>());
			if constexpr (std::is_same_v<BVType, oGFX::AABB>)
			{
				nodes.back()->BV = e.aabb;
			}
			if constexpr (std::is_same_v<BVType, oGFX::Sphere>)
			{
				nodes.back()->BV = e.sphere;
			}
		}

		while (nodes.size() > 1)
		{
			std::unique_ptr<TreeNode<BVType>> left{}, right{};

			mergeNodes<BVType>(nodes, left, right);

			nodes.emplace_back(std::make_unique<TreeNode<BVType>>());
			{
				if constexpr (std::is_same_v<BVType, oGFX::AABB>)
				{
					oGFX::AABB ab;
					glm::vec3 max = left->BV.max();
					glm::vec3 min = left->BV.min();
					for (uint32_t i = 0; i < 3; i++)
					{
						max[i] = std::max(right->BV.max()[i], max[i]);
						min[i] = std::min(right->BV.min()[i], min[i]);
					}
					nodes.back()->BV.halfExt = (max - min) * 0.5f;
					nodes.back()->BV.center = max - nodes.back()->BV.halfExt;
					//glm::vec3 cen = max - min;
				}
				if constexpr (std::is_same_v<BVType, oGFX::Sphere>)
				{
					oGFX::Sphere s;
					auto lr = left->BV.radius;
					auto rr = right->BV.radius;
					glm::vec3 dir = left->BV.center - right->BV.center;
					float dist = std::sqrt(glm::dot(dir, dir));
					dir = glm::normalize(dir);
					if ((dist + rr) < lr || (dist + lr) < rr )
					{
						//degenerate case
						if (lr > rr) 
						{
							s.radius = lr;
							s.center = left->BV.center;
						}
						else
						{
							s.radius = rr;
							s.center = right->BV.center;
						}						
					}
					else
					{
						s = right->BV;
						oGFX::BV::ExpandSphereAboutPoint(s, left->BV.center+ dir*lr);
					}
					nodes.back()->BV = s;
				}
			}
			std::swap(nodes.back()->children[0],left);
			std::swap(nodes.back()->children[1],right);
		}
		std::swap(nodes.front(),root);

		UpdateDepth<BVType>(root, 0);

	}

	template<typename BVType>
	void UpdateDepth(std::unique_ptr<TreeNode<BVType>>& parent, uint32_t depth)
	{
		if (parent == nullptr)
			return;
		
		parent->depth = depth;

		for (size_t i = 0; i < numChilden; i++)
		{
			UpdateDepth(parent->children[i], depth + 1);
		}

	}

	template <typename dataT, typename BVType>
	static void getDrawList(std::vector<std::pair<uint32_t,BVType>>& vec, TreeNode<BVType>* parent)
	{
		if (parent == nullptr) 
			return;

		for (size_t i = 0; i < numChilden; i++)
		{
			getDrawList<dataT,BVType>(vec, parent->children[i].get());
		}
		vec.push_back({ parent->depth,parent->BV });

	}
};


