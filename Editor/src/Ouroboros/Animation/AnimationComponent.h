/************************************************************************************//*!
\file           AnimationComponent.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          BRIEF_HERE

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
		AnimationTree* animTree{ nullptr };
		AnimationTracker tracker{};
		Ecs::EntityID root_objectID{};
	};
}

namespace oo
{
	class AnimationComponent
	{
		friend class oo::Anim::AnimationSystem;
		Anim::IAnimationComponent actualComponent;
		//if using fbx animations this would be the root bone gameobject
		Ecs::EntityID root_object; 
	public:
		Anim::IAnimationComponent& GetActualComponent();
		void Set_Root_Entity(Ecs::EntityID entity);
		void SetAnimationTree(std::string const& name);
		void SetParameter(std::string const& name, Anim::Parameter::DataType value);

		Anim::AnimationTree* GetAnimationTree();
		Anim::AnimationTracker& GetTracker();
		Anim::Group* GetGroup(std::string const& name);

		//adds a node to the group in the animation tree attached to this component
		//returns nullptr if no animation tree
		Anim::Node* AddNode(std::string const& groupName, Anim::NodeInfo& info);
		
		//adds a link between two nodes
		//returns nullptr if link was not added due to error(src or dst not found)
		Anim::Link* AddLink(std::string const& groupName, std::string const& src, std::string const& dst);
		
		
		Anim::Parameter* AddParameter(Anim::ParameterInfo const& info);

		Anim::Condition* AddCondition(std::string const& groupName, std::string const& linkName, Anim::ConditionInfo info);

		//adds animation to a node
		//Anim::Animation* AddAnimation(std::string const& groupName, std::string const& nodeName, std::string const name = { "Unnamed Animation" });

		//add timeline to animation
		Anim::Timeline* AddTimeline(std::string const& groupName, std::string const& nodeName, Anim::TimelineInfo const& info);

		/*Anim::Timeline* AddTimeline(std::string const& groupName, std::string const& nodeName,
			std::string const& timelineName, Anim::Timeline::TYPE type, Anim::Timeline::DATATYPE datatype);*/

		//adds a keyframe to a node's animation asset
		Anim::KeyFrame* AddKeyFrame(std::string const& groupName, std::string const nodeName, 
			std::string const& timelineName, Anim::KeyFrame keyframe);

		//Anim::ScriptEvent* AddScriptEvent(std::string const& groupName);


		//TimelineInfo const& info
	};
}

