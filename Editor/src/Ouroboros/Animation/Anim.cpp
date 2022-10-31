/************************************************************************************//*!
\file           Anim.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Definitions for animation related classes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "Anim.h"
#include "AnimationInternal.h"
#include "AnimationSystem.h"
#include "Archetypes_Ecs/src/A_Ecs.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "Project.h"
#include "App/Editor/UI/Tools/MeshHierarchy.h"

#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <rttr/registration>
#include <random>
namespace oo::Anim::internal
{

} //namespace oo::Anim::internal

namespace oo::Anim
{
	/*-------------------------------
	Timeline
	-------------------------------*/
	Timeline::Timeline(TimelineInfo const& info)
		: type{ info.type }
		, rttr_property{info.rttr_property}
		, rttr_type{ info.rttr_property.get_type()}
		, name{info.timeline_name}
		, component_hash{info.component_hash}
		, children_index{ info.children_index }
	{
		get_componentFn = Ecs::ECSWorld::get_component_Fn(component_hash);

		//verify able to retrieve the component info
		assert(get_componentFn != nullptr);
	}
	/*Timeline::Timeline(TYPE _type, DATATYPE _datatype, std::string const _name)
		: type{_type}
		, datatype{_datatype}
		, name{_name}
		, rttr_type{rttr::type::get<TransformComponent>()}
	{
		rttr::property::get_type("")
	}*/

	/*-------------------------------
	AnimationTree
	-------------------------------*/
	std::unordered_map<std::string, AnimationTree> AnimationTree::map;

	AnimationTree* AnimationTree::Create(std::string const name)
	{
		AnimationTree tree;
		tree.name = name;
		AnimationTree::map.emplace(name, std::move(tree));
		auto& createdTree = AnimationTree::map[name];
		//create a default group and assign to tree
		GroupInfo info{ .name{"Group 1"},.tree{&createdTree} };
		internal::AddGroupToTree(AnimationTree::map[name], info);

		return &(AnimationTree::map[name]);
	}

	/*-------------------------------
	ProgressTracker
	-------------------------------*/
	ProgressTracker::ProgressTracker(const Timeline::TYPE _type) :
		type{ _type }
	{

	}
	ProgressTracker ProgressTracker::Create(Timeline::TYPE type)
	{
		ProgressTracker tracker{type};
		switch (type)
		{
		case Timeline::TYPE::PROPERTY:
			tracker.updatefunction = &internal::UpdateProperty_Animation;
			break;
		case Timeline::TYPE::FBX_ANIM:
			tracker.updatefunction = &internal::UpdateFBX_Animation;
			break;
		case Timeline::TYPE::SCRIPT_EVENT:
			tracker.updatefunction = &internal::UpdateEvent;
			break;
		default:
			break;
		}

		return tracker;
	}
	/*-------------------------------
	Animation
	-------------------------------*/
	void Animation::LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource)
	{
		/*
		static constexpr auto get_node_hierarchy = [](oGFX::Skeleton const* skeleton, 
			std::string boneName, std::vector<int>& children_index) 
		{
			//skeleton should have nodes!!
			assert(skeleton->m_boneNodes);
			//find the node
			std::stack<oGFX::BoneNode*> stack;
			std::unordered_map<uint32_t, bool> visited{};
			stack.push(skeleton->m_boneNodes);
			visited[skeleton->m_boneNodes->m_BoneIndex] = false;
			oGFX::BoneNode* targetbone{nullptr};
			while (stack.empty() == false)
			{
				auto curr = stack.top();
				stack.pop();

				if (visited[curr->m_BoneIndex] == false)
					visited[curr->m_BoneIndex] = true;

				if (curr->mName == boneName)
				{
					targetbone = curr;
					break;
				}

				for (auto child : curr->mChildren)
				{
					stack.push(child);
					visited[child->m_BoneIndex] = false;
				}
			}
			//we should have found the target bone by now
			assert(targetbone);


		};*/



		Assimp::Importer importer;
		uint flags = 0;
		flags |= aiProcess_Triangulate;
		flags |= aiProcess_GenSmoothNormals;
		flags |= aiProcess_ImproveCacheLocality;
		flags |= aiProcess_CalcTangentSpace;
		flags |= aiProcess_FindInstances; // this step is slow but it finds duplicate instances in FBX
		//flags |= aiProcess_LimitBoneWeights; // limmits bones to 4
		const aiScene* scene = importer.ReadFile(filepath, flags
		);

		if (!scene)
		{
			assert(false);
			//return {}; // Dont explode...
			//throw std::runtime_error("Failed to load model! (" + file + ")");
		}
		assert(scene->HasAnimations());

		//generate hierarchy map
		std::unordered_map<std::string, std::vector<int>> bone_hierarchy_map{};
		{
			std::function<void(oGFX::BoneNode*)> traverse_recursive = [&](oGFX::BoneNode* node)
			{
				if (node->mChildren.empty())
					return;

				int idx = 0;
				std::vector<int> children_index = bone_hierarchy_map[node->mName];
				//add a new element
				children_index.emplace_back(idx);
				for (auto child : node->mChildren)
				{
					//child should not be null!!
					assert(child);
					//set it to the correct index
					children_index.back() = idx;
					//assign it to the map
					bone_hierarchy_map[child->mName] = children_index;
					//recurse
					traverse_recursive(child);
					//increment index
					++idx;
				}

			};

			auto current = resource->skeleton->m_boneNodes;
			std::unordered_map< uint32_t, bool> visited{};
			std::vector<int> children_idx{};
			bone_hierarchy_map[current->mName] = children_idx;
			traverse_recursive(current);
		}


		std::cout << "Animated scene\n";
		for (size_t i = 0; i < scene->mNumAnimations; i++)
		{
			std::cout << "Anim name: " << scene->mAnimations[i]->mName.C_Str() << std::endl;
			std::cout << "Anim frames: " << scene->mAnimations[i]->mDuration << std::endl;
			std::cout << "Anim ticksPerSecond: " << scene->mAnimations[i]->mTicksPerSecond << std::endl;
			std::cout << "Anim duration: " << static_cast<float>(scene->mAnimations[i]->mDuration) / scene->mAnimations[i]->mTicksPerSecond << std::endl;
			std::cout << "Anim numChannels: " << scene->mAnimations[i]->mNumChannels << std::endl;
			std::cout << "Anim numMeshChannels: " << scene->mAnimations[i]->mNumMeshChannels << std::endl;
			std::cout << "Anim numMeshChannels: " << scene->mAnimations[i]->mNumMorphMeshChannels << std::endl;

			Animation anim{};
			anim.name = scene->mAnimations[i]->mName.C_Str();

			for (size_t x = 0; x < scene->mAnimations[i]->mNumChannels; x++)
			{
				auto& channel = scene->mAnimations[i]->mChannels[x];
				oGFX::BoneNode* curr = resource->skeleton->m_boneNodes;
				std::string boneName{ channel->mNodeName.C_Str() };
				//guarding for safety
				assert(resource->strToBone.contains(boneName));
				auto boneindex = resource->strToBone[boneName];
				//bone should exist in the skeleton!!
				assert(bone_hierarchy_map.contains(boneName));
				std::vector<int> children_index = bone_hierarchy_map[boneName];
				//get_node_hierarchy(curr, boneindex, children_index);
				/*--------
				position
				--------*/
				{
					TimelineInfo timeline_info{
					.type{Timeline::TYPE::FBX_ANIM},
					.component_hash{Ecs::ECSWorld::get_component_hash<TransformComponent>()},
					.rttr_property{ rttr::type::get<TransformComponent>().get_property("Position")},
					.timeline_name{channel->mNodeName.C_Str()},
					.children_index = children_index };

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumPositionKeys; y++)
					{
						auto& key = channel->mPositionKeys[y];
						KeyFrame kf{ .data{ glm::vec3{key.mValue.x,key.mValue.y,key.mValue.z} },
						.time{ static_cast<float>(key.mTime) } };

						auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				/*--------
				rotation
				--------*/
				{
					TimelineInfo timeline_info{
					.type{Timeline::TYPE::FBX_ANIM},
					.component_hash{Ecs::ECSWorld::get_component_hash<TransformComponent>()},
					.rttr_property{ rttr::type::get<TransformComponent>().get_property("Quaternion")},
					.timeline_name{channel->mNodeName.C_Str()} ,
					.children_index = children_index };

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumRotationKeys; y++)
					{
						auto& key = channel->mRotationKeys[y];
						KeyFrame kf{ .data{ glm::quat{key.mValue.w, key.mValue.x,key.mValue.y,key.mValue.z} },
						.time{ static_cast<float>(key.mTime) } };

						auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				/*--------
				scale
				--------*/
				{
					TimelineInfo timeline_info{
					.type{Timeline::TYPE::FBX_ANIM},
					.component_hash{Ecs::ECSWorld::get_component_hash<TransformComponent>()},
					.rttr_property{ rttr::type::get<TransformComponent>().get_property("Scaling")},
					.timeline_name{channel->mNodeName.C_Str()} ,
					.children_index = children_index };

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumScalingKeys; y++)
					{
						auto& key = channel->mScalingKeys[y];
						KeyFrame kf{ .data{ glm::vec3{key.mValue.x,key.mValue.y,key.mValue.z} },
						.time{ static_cast<float>(key.mTime) } };

						auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				/*std::cout << "\tKeys name: " << channel->mNodeName.C_Str() << std::endl;
				for (size_t y = 0; y < channel->mNumPositionKeys; y++)
				{
					std::cout << "\t Key_" << std::to_string(y) << " time: " << channel->mPositionKeys[y].mTime << std::endl;
					auto& pos = channel->mPositionKeys[y].mValue;
					std::cout << "\t Key_" << std::to_string(y) << " value: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
				}*/
			}

			auto createdAnim = Animation::AddAnimation(anim);
			assert(createdAnim);
		}
		//std::cout << std::endl;
		
	}
	Animation* Animation::AddAnimation(Animation& anim)
	{
		//Animation::ID_to_index[anim.animation_ID] = static_cast<uint>(Animation::animation_storage.size());
		Animation::name_to_ID[anim.name] = anim.animation_ID;
		size_t key = anim.animation_ID;
		auto [iter, result] = Animation::animation_storage.emplace(key, std::move(anim));
		assert(result == true);
		auto& createdAnim = Animation::animation_storage[key];

		return &createdAnim;
	}


	/*-------------------------------
	AnimationComponent
	-------------------------------*/
}
