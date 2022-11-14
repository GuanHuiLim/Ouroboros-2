/************************************************************************************//*!
\file           AnimationNode.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A basic building block of an animation tree which references an animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"
#include "AnimationGroup.h"
#include "AnimationTimeline.h"
namespace oo::Anim
{
	struct Node
	{
		using SerializeFn = void(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>&, Node&);
		static SerializeFn* const serializeFn;

		GroupRef group{};
		std::string name{};
		//animation asset loaded from file
		Asset anim_asset{};
		AnimRef anim{};
		////used to get the animation's index
		//size_t animation_ID{ internal::invalid_ID };
		////index of the animation in the animation vector
		//uint animation_index{ internal::invalid_index };
		////Animation animation{};
		float speed{ 1.f };
		glm::vec3 position{};

		size_t node_ID{ internal::invalid_ID };

		//trackers to be given to the animation component 
		//upon reaching this node
		std::vector<ProgressTracker> trackers{};
		//outgoing links to other nodes
		std::vector<LinkRef> outgoingLinks{};

		//Node(Group& _group, std::string const _name = "Unnamed Node");
		Node() = default;
		Node(NodeInfo& info);
		//void SetAnimation(Asset asset);
		//void SetAnimation(Asset asset);
		Animation& GetAnimation();
		oo::Asset GetAnimationAsset();
		AnimRef SetAnimationAsset(oo::Asset asset);
		
		bool HasAnimation();

		RTTR_ENABLE();
	};

	struct NodeInfo
	{
		std::string name{ "Unnamed Node" };
		std::string animation_name{};
		float speed{ 1.f };
		glm::vec3 position{ 0.f,0.f,0.f };

		//dont fill this up
		GroupRef group{};
		size_t nodeID{ internal::invalid_ID };
	};
}