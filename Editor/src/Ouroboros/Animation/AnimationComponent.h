/************************************************************************************//*!
\file           AnimationComponent.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          This component allows a gameobject to have animations

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim.h"
namespace oo::Anim
{
	class IAnimationComponent
	{
	public:
		std::string animTree_name{};
		size_t animTree_ID{internal::invalid_ID};
		AnimationTree* animTree{ nullptr };
		AnimationTracker tracker{};
		Ecs::EntityID root_objectID{};
		AnimationSkeleton skeleton{};
		
		inline void Reset()
		{
			IAnimationComponent temp{};
			*this = temp;
		}
	};
}

namespace oo
{
	class AnimationComponent
	{
		friend class oo::Anim::AnimationSystem;
		Anim::IAnimationComponent actualComponent{};
		oo::Asset anim_tree_asset{};
		//if using fbx animations this would be the root bone gameobject
		Ecs::EntityID root_object{};
	public:
		Anim::IAnimationComponent& GetActualComponent();

		//ignore for now
		void Set_Root_Entity(Ecs::EntityID entity);

		//set animation tree via name 
		//*note that if multiple trees with same name exists, any one of them could be chosen
		void SetAnimationTree(std::string name);

		//set animation tree via unique id
		void SetAnimationTree(size_t id);

		//set animation tree via asset
		void SetAnimationTree(oo::Asset asset);

		//get name of animation tree, or empty string if no animation tree attached
		std::string GetAnimationTreeName();
		bool HasAnimationTree();
		void SetParameter(std::string const& name, Anim::Parameter::DataType value);
		void SetParameterByID(size_t id, Anim::Parameter::DataType value);
		void SetParameterByIndex(uint index, Anim::Parameter::DataType value);

		size_t GetParameterID(std::string const& name);
		uint GetParameterIndex(std::string const& name);

		Anim::AnimationTree* GetAnimationTree();
		oo::Asset GetAnimationTreeAsset();
		Anim::AnimationTracker& GetTracker();
		Anim::GroupRef GetGroup(std::string const& name);

		//adds a node to the group in the animation tree attached to this component
		//returns nullptr if no animation tree
		Anim::NodeRef AddNode(std::string const& groupName, Anim::NodeInfo& info);

		//removes a node from a group
		//returns false if no removal happened
		bool RemoveNode(Anim::TargetNodeInfo const& info);

		//adds a link between two nodes
		//returns nullptr if link was not added due to error(src or dst not found)
		Anim::LinkRef AddLink(std::string const& groupName, std::string const& src, std::string const& dst);

		//removes a link from a group
		void RemoveLink(Anim::TargetLinkInfo const& info);

		//add a parameter to an animation tree
		Anim::Parameter* AddParameter(Anim::ParameterInfo const& info);

		//remove a parameter from the animation tree
		void RemoveParameter(Anim::TargetParameterInfo const& info);

		//add condition to a link
		Anim::Condition* AddCondition(std::string const& groupName, std::string const& linkName, Anim::ConditionInfo info);

		//remove condition from a link
		void RemoveCondition(Anim::TargetConditionInfo const& info);

		//add timeline to animation
		Anim::TimelineRef AddTimeline(std::string const& groupName, std::string const& nodeName, Anim::TimelineInfo& info);


		//adds a keyframe to a node's animation asset
		Anim::KeyFrame* AddKeyFrame(std::string const& groupName, std::string const nodeName, 
			std::string const& timelineName, Anim::KeyFrame keyframe);

		Anim::ScriptEvent* AddScriptEvent(std::string const& groupName, std::string const nodeName,
			std::string const& timelineName, Anim::ScriptEvent scriptevent);

		Anim::AnimRef SetNodeAnimation(Anim::SetNodeAnimInfo const& info);

		void ReInsertKeyFrame(Anim::TargetTimelineInfo info, uint keyframe_index, float new_time);

		

		RTTR_ENABLE();
	};
}

