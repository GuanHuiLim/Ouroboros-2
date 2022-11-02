/************************************************************************************//*!
\file           Animation.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
An animation asset created via editor or imported from fbx to be referenced by nodes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "Animation.h"
#include "AnimationTimeline.h"
#include "AnimationKeyFrame.h"
#include "AnimationInternal.h"
#include "Project.h"

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"

#include <rttr/registration>

namespace oo::Anim::internal
{
	void SerializeAnimation(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Animation& anim)
	{
		writer.StartObject();
		{
			//properties
			{
				rttr::instance obj{ anim };
				auto properties = rttr::type::get<Animation>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}			
			//script events
			{
				auto serialize_fn = rttr::type::get<ScriptEvent>().get_method(internal::serialize_method_name);
				writer.Key("script events", static_cast<rapidjson::SizeType>(std::string("script events").size()));
				writer.StartArray();
					for (auto& script_event : anim.events)
					{
						serialize_fn.invoke({}, writer, script_event);
					}
				writer.EndArray();
			}
			//timelines
			{
				auto serialize_fn = rttr::type::get<Timeline>().get_method(internal::serialize_method_name);
				writer.Key("timelines", static_cast<rapidjson::SizeType>(std::string("timelines").size()));
				writer.StartArray();
				for (auto& timeline : anim.timelines)
				{
					serialize_fn.invoke({}, writer, timeline);
				}
				writer.EndArray();
			}
		}
		writer.EndObject();
	}

	void LoadAnimation(rapidjson::GenericObject<false, rapidjson::Value>& object, Animation& anim)
	{
		rttr::instance obj{ anim };
		//properties
		{
			auto properties = rttr::type::get<Animation>().get_properties();
			for (auto& prop : properties)
			{
				auto& value = object.FindMember(prop.get_name().data())->value;

				assert(internal::loadDataFn_map.contains(prop.get_type().get_id()));
				rttr::variant val{ internal::loadDataFn_map.at(prop.get_type().get_id())(value) };
				prop.set_value(obj, val);
			}
		}
		//script events
		{
			auto script_events = object.FindMember("script events")->value.GetArray();
			auto load_fn = rttr::type::get<ScriptEvent>().get_method(internal::load_method_name);
			assert(load_fn);
			for (auto& evnt : script_events)
			{
				ScriptEvent new_evnt{};
				auto result = load_fn.invoke({}, evnt.GetObj(), new_evnt);
				assert(result.is_valid());
				anim.events.emplace_back(std::move(new_evnt));
			}
		}
		//timelines
		{
			auto timelines = object.FindMember("timelines")->value.GetArray();
			auto load_fn = rttr::type::get<Timeline>().get_method(internal::load_method_name);
			assert(load_fn);
			for (auto& timeline : timelines)
			{
				Timeline new_timeline{};
				auto result = load_fn.invoke({}, timeline.GetObj(), new_timeline);
				assert(result.is_valid());
				anim.timelines.emplace_back(std::move(new_timeline));
			}
		}
	}
}

namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
	registration::class_<Animation>("Animation")
		.property("name", &Animation::name)
		.property("looping", &Animation::looping)
		.property("animation_length", &Animation::animation_length)
		.property("animation_ID", &Animation::animation_ID)
		.method(internal::serialize_method_name, &internal::SerializeAnimation)
		.method(internal::load_method_name, &internal::LoadAnimation)
		;
	}

	std::unordered_map< std::string, size_t> Animation::name_to_ID{};
	std::map<size_t, Animation> Animation::animation_storage = []() {
		decltype(Animation::animation_storage) container{};
		//container.reserve(internal::expected_num_anims);

		//create empty animation
		Animation empty_anim{};
		empty_anim.name = Animation::empty_animation_name;

		Animation::name_to_ID[empty_anim.name] = empty_anim.animation_ID;
		//Animation::name_to_index[empty_anim.name] = static_cast<uint>(container.size());
		auto key = empty_anim.animation_ID;
		container.emplace(key, std::move(empty_anim));
		return container;
	}();


	void PrintNodeHierarchy(const aiScene* scene)
	{
		//to do later
	}

	void Animation::LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource)
	{


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
		if (scene->HasAnimations() == false) return;

		PrintNodeHierarchy(scene);

		std::cout << "Bone hierarchy: " << std::endl;
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

					//debugging print
					for (size_t i = 0; i < children_index.size(); ++i) std::cout << " |";
					std::cout << child->mName + "  hierarchy: ";
					for (auto& index : children_index) std::cout << index << ',';
					std::cout << std::endl;

					//recurse
					traverse_recursive(child);
					//increment index
					++idx;
				}

			};

			auto current = resource->skeleton->m_boneNodes;

			std::cout << "Root bone: " << current->mName << std::endl;

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
			std::cout << "-------------------------------------------------------" << std::endl;

			Animation anim{};
			anim.name = scene->mAnimations[i]->mName.C_Str();

			for (size_t x = 0; x < scene->mAnimations[i]->mNumChannels; x++)
			{
				std::cout << "Anim channel: " << scene->mAnimations[i]->mChannels[x]->mNodeName.C_Str() << std::endl;

				auto& channel = scene->mAnimations[i]->mChannels[x];
				//oGFX::BoneNode* curr = resource->skeleton->m_boneNodes;
				std::string boneName{ channel->mNodeName.C_Str() };
				//guarding for safety
				//assert(resource->strToBone.contains(boneName));
				if (resource->strToBone.contains(boneName) == false)
				{
					std::cout << "Skipped: " << scene->mAnimations[i]->mChannels[x]->mNodeName.C_Str() << std::endl;
					continue;
				}

				//auto boneindex = resource->strToBone[boneName];
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
					.timeline_name{channel->mNodeName.C_Str() + std::string{" position"}},
					.children_index = children_index,
					.hierarchy_provided = true};

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumPositionKeys; y++)
					{
						auto& key = channel->mPositionKeys[y];
						KeyFrame kf{ glm::vec3{key.mValue.x,key.mValue.y,key.mValue.z},
						static_cast<float>(key.mTime)};

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
					.timeline_name{channel->mNodeName.C_Str() + std::string{" rotation"}} ,
					.children_index = children_index,
					.hierarchy_provided = true };
					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumRotationKeys; y++)
					{
						auto& key = channel->mRotationKeys[y];
						KeyFrame kf{glm::quat{key.mValue.w, key.mValue.x,key.mValue.y,key.mValue.z} ,
						static_cast<float>(key.mTime)};

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
					.timeline_name{channel->mNodeName.C_Str() + std::string{" scale"}} ,
					.children_index = children_index,
					.hierarchy_provided = true };

					auto timeline = internal::AddTimelineToAnimation(anim, timeline_info);
					assert(timeline);

					for (size_t y = 0; y < channel->mNumScalingKeys; y++)
					{
						auto& key = channel->mScalingKeys[y];
						KeyFrame kf{glm::vec3{key.mValue.x,key.mValue.y,key.mValue.z} ,
						static_cast<float>(key.mTime)};

						auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				std::cout << "Loaded: " << scene->mAnimations[i]->mChannels[x]->mNodeName.C_Str() << std::endl;
			}

			auto createdAnim = Animation::AddAnimation(std::move(anim));
			assert(createdAnim);
		}
		//std::cout << std::endl;

	}
	Animation* Animation::AddAnimation(Animation&& anim)
	{
		//Animation::ID_to_index[anim.animation_ID] = static_cast<uint>(Animation::animation_storage.size());
		Animation::name_to_ID[anim.name] = anim.animation_ID;
		size_t key = anim.animation_ID;
		auto [iter, result] = Animation::animation_storage.emplace(key, std::move(anim));
		assert(result == true);
		auto& createdAnim = Animation::animation_storage[key];

		//auto assetmanager = Project::GetAssetManager();

		return &createdAnim;
	}
}