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


constexpr bool DEBUG_PRINT = false;

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
				[[maybe_unused]] auto result = prop.set_value(obj, val);
				assert(result);
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
		.property("frames_per_second", &Animation::frames_per_second)
		//.property("total_frames", &Animation::total_frames)
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
		empty_anim.name			= internal::empty_animation_name;
		empty_anim.animation_ID	= internal::empty_animation_UID;

		Animation::name_to_ID[empty_anim.name] = empty_anim.animation_ID;
		//Animation::name_to_index[empty_anim.name] = static_cast<uint>(container.size());
		auto key = empty_anim.animation_ID;
		container.emplace(key, std::move(empty_anim));
		return container;
	}();

	void PrintNode(decltype(aiScene::mRootNode) node, std::vector<uint> const& hierarchy)
	{
		std::ostringstream buffer;
		for (auto const& i : hierarchy)
		{
			(void)(i);
			buffer << "|";
		}
		
		buffer << "->" << node->mName.C_Str();

		std::cout << buffer.str() << std::endl;
	}

	void PrintRecusive(decltype(aiScene::mRootNode) node, std::vector<uint> hierarchy = {})
	{
		PrintNode(node, hierarchy);
		if (node->mNumChildren == 0) return;

		//recurse on children
		hierarchy.emplace_back(0);
		for (uint i = 0; i < node->mNumChildren; i++)
		{
			hierarchy.back() = i;
			PrintRecusive(node->mChildren[i], hierarchy);
		}
	}

	void PrintNodeHierarchy(const aiScene* scene)
	{
		if constexpr (DEBUG_PRINT)
		{
			std::cout << "----------|Node Hierarchy|----------" << std::endl;

			auto node = scene->mRootNode;
			PrintRecusive(node);
		}
		
		
	}

	std::vector<Animation*> Animation::LoadAnimationFromFBX(std::string const& filepath, ModelFileResource* resource)
	{
		std::ostringstream os;

		Assimp::Importer importer;
		uint flags = 0;
		flags |= aiProcess_Triangulate;
		flags |= aiProcess_GenSmoothNormals;
		flags |= aiProcess_ImproveCacheLocality;
		flags |= aiProcess_CalcTangentSpace;
		//attempt to optimize animation data, currently not working as intended
		flags |= aiProcess_FindInvalidData; 
		flags |= aiProcess_FindInstances; // this step is slow but it finds duplicate instances in FBX
		//flags |= aiProcess_LimitBoneWeights; // limmits bones to 4
		const aiScene* scene = importer.ReadFile(filepath, flags
		);

		if (!scene)
		{
			assert(false);
			return {};
		}
		if (scene->HasAnimations() == false) return {};

#ifdef EDITOR_DEBUG
		PrintNodeHierarchy(scene);
#endif // EDITOR_DEBUG

		

		os << "--------------|Bone hierarchy|--------------" << std::endl;
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
					for (size_t i = 0; i < children_index.size(); ++i) os << " |";
					os << child->mName + "  hierarchy: ";
					for (auto& index : children_index) os << index << ',';
					os << std::endl;

					//recurse
					traverse_recursive(child);
					//increment index
					++idx;
				}

			};

			auto current = resource->skeleton->m_boneNodes;

			os << "Root bone: " << current->mName << std::endl;

			std::vector<int> children_idx{};
			bone_hierarchy_map[current->mName] = children_idx;
			traverse_recursive(current);
		}

		std::string prefix_name = std::filesystem::path{ filepath }.stem().string() + "_";

		os << "Animated scene\n";
		std::vector<Animation*> anims;
		for (size_t i = 0; i < scene->mNumAnimations; i++)
		{
			os << "Anim name: " << scene->mAnimations[i]->mName.C_Str() << std::endl;
			os << "Anim frames: " << scene->mAnimations[i]->mDuration << std::endl;
			os << "Anim ticksPerSecond: " << scene->mAnimations[i]->mTicksPerSecond << std::endl;
			os << "Anim duration: " << static_cast<float>(scene->mAnimations[i]->mDuration) / scene->mAnimations[i]->mTicksPerSecond << std::endl;
			os << "Anim numChannels: " << scene->mAnimations[i]->mNumChannels << std::endl;
			os << "Anim numMeshChannels: " << scene->mAnimations[i]->mNumMeshChannels << std::endl;
			os << "-------------------------------------------------------" << std::endl;

			Animation anim{};
			anim.name = prefix_name + scene->mAnimations[i]->mName.C_Str();

			if (scene->mAnimations[i]->mTicksPerSecond != 0)
				anim.frames_per_second = static_cast<float>(scene->mAnimations[i]->mTicksPerSecond);
			else
				anim.frames_per_second = 25.f;

			//anim.total_frames	  = static_cast<float>(scene->mAnimations[i]->mDuration);
			auto total_frames	  = static_cast<float>(scene->mAnimations[i]->mDuration);
			anim.animation_length = total_frames / anim.frames_per_second;



			for (size_t x = 0; x < scene->mAnimations[i]->mNumChannels; x++)
			{
				os << "Anim channel: " << scene->mAnimations[i]->mChannels[x]->mNodeName.C_Str() << std::endl;

				auto& channel = scene->mAnimations[i]->mChannels[x];
				//oGFX::BoneNode* curr = resource->skeleton->m_boneNodes;
				std::string boneName{ channel->mNodeName.C_Str() };
				//guarding for safety
				//assert(resource->strToBone.contains(boneName));
				if (resource->strToBone.contains(boneName) == false)
				{
					os << "Skipped: " << scene->mAnimations[i]->mChannels[x]->mNodeName.C_Str() << std::endl;
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
						static_cast<float>(key.mTime) / anim.frames_per_second };

						[[maybe_unused]] auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);

						os << y << "- Keyframe Position: " << key.mValue.x << "," << key.mValue.y << "," << key.mValue.z << std::endl;
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
						static_cast<float>(key.mTime) / anim.frames_per_second };

						[[maybe_unused]] auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
						os << y << "- Keyframe rotation: " << key.mValue.x << "," << key.mValue.y << "," << key.mValue.z << "," << key.mValue.w << std::endl;
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
						static_cast<float>(key.mTime) / anim.frames_per_second };

						[[maybe_unused]] auto keyframe = internal::AddKeyframeToTimeline(*timeline, kf);
						assert(keyframe);
					}
				}
				os << "Loaded: " << scene->mAnimations[i]->mChannels[x]->mNodeName.C_Str() << std::endl;
			}

			auto createdAnim = Animation::AddAnimation(std::move(anim));
			assert(createdAnim);
			anims.emplace_back(createdAnim);
		}
		if constexpr (DEBUG_PRINT)
		{
			std::cout << os.str();
		}

		return anims;
	}
	Animation* Animation::AddAnimation(Animation&& anim)
	{
		//Animation::ID_to_index[anim.animation_ID] = static_cast<uint>(Animation::animation_storage.size());

		//remove existing dupe
		bool existing = Animation::name_to_ID.contains(anim.name);
		if (existing)
		{
			[[maybe_unused]] auto old_anim = internal::RetrieveAnimation(Animation::name_to_ID[anim.name]);
			assert(old_anim);
			//anim.animation_ID = old_anim->animation_ID;
			RemoveAnimation(anim.name);
		}
		bool existInStorage = Animation::animation_storage.contains(anim.animation_ID);
		assert(existInStorage == false); 
		Animation::name_to_ID[anim.name] = anim.animation_ID;
		size_t key = anim.animation_ID;
		auto [iter, result] = Animation::animation_storage.emplace(key, std::move(anim));
		assert(result);//should not have overwritten anything as dupes are removed beforehand
		auto& createdAnim = Animation::animation_storage[key];

		//auto assetmanager = Project::GetAssetManager();

		return &createdAnim;
	}

	void Animation::RemoveAnimation(std::string const& name)
	{
		assert(Animation::name_to_ID.contains(name));
		assert(Animation::animation_storage.contains(Animation::name_to_ID[name]));

		Animation::animation_storage.erase(Animation::name_to_ID[name]);
		Animation::name_to_ID.erase(name);

	}
	float Animation::TimeFromFrame(size_t frame)
	{
		return (1.f / frames_per_second) * static_cast<float>(frame);
	}

	Animation Animation::ExtractAnimation(float start_time, float end_time, Animation& anim, bool& result)
	{
		if (start_time > end_time)
		{
			assert(false); //how can start time more than end time
			result = false;
			return {};
		}

		Animation new_anim{};

		new_anim.frames_per_second = anim.frames_per_second;

		//copy keyframes in timelines
		for (auto& timeline : anim.timelines)
		{
			if (timeline.keyframes.empty()) continue;

			Timeline new_timeline{ timeline };
			new_timeline.keyframes.clear();

			auto& keyframes = timeline.keyframes;

			size_t start_keyframe_index{ 0 };
			size_t end_keyframe_index{ keyframes.size() };
			float start_keyframe_timing{ 0.f};

			{//find starting keyframe index
				size_t index{ 0ull };
				for (auto& kf : keyframes)
				{
					if (internal::Equal(kf.time, start_time))
					{
						break;
					}
					if (kf.time > start_time)
					{
						start_keyframe_timing = kf.time - start_time;
						break;
					}
					++index;
				}
				//if first keyframe is last in array, 
				if (index == keyframes.size())
				{
					auto keyframe = keyframes.back();
					keyframe.time = 0.f;
					new_timeline.keyframes.emplace_back(keyframe); //starting keyframe
					//keyframe.time = end_time - start_time;
					//new_timeline.keyframes.emplace_back(keyframe); //ending keyframe
					new_anim.timelines.emplace_back(std::move(new_timeline));
					continue;
					
					//assert(false);//start keyframe not found!
					//result = false;
					//return {};
					
				}
				start_keyframe_index = index;
			}
			{//find ending keyframe index
				size_t index{ 0ull };
				for (auto& kf : keyframes)
				{
					if (internal::Equal(kf.time, end_time))
						break;

					if (kf.time > end_time)
					{
						if (index == 0ull)
						{
							assert(false);//end keyframe cant be the first keyframe!
							result = false;
							return {};
						}
						--index;
						break;
					}
					++index;
				}
				end_keyframe_index = index;
			}
			if (end_keyframe_index == keyframes.size()) //end frame is the end
			{
				end_keyframe_index = keyframes.size() - 1ull;
			}
			auto keyframe_timing = start_keyframe_timing;
			for (size_t index = start_keyframe_index; index < end_keyframe_index; ++index)
			{
				//if keyframe isnt the first
				if (index > start_keyframe_index)
				{//increment start keyframe time by difference between previous and current keyframe
					keyframe_timing += keyframes[index].time - keyframes[index - 1ull].time;
				}
				auto keyframe = keyframes[index];
				keyframe.time = keyframe_timing;

				new_timeline.keyframes.emplace_back(keyframe);
			}
			assert(new_timeline.keyframes.size() > 1ull);
			new_anim.timelines.emplace_back(std::move(new_timeline));
		}

		result = true;
		return new_anim;
	}

	Animation Animation::ExtractAnimation(size_t start_frame, size_t end_frame, Animation& anim, bool& result)
	{
		float start_time = anim.TimeFromFrame(start_frame);
		float end_time = anim.TimeFromFrame(end_frame);

		return ExtractAnimation(start_time, end_time, anim, result);
	}

}