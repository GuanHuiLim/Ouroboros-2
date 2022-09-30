/************************************************************************************//*!
\file           AnimationTracker.h
\project        PROJECT_NAME_HERE
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
#include "Anim_Utils.h"
#include "Ouroboros/ECS/GameObject.h"
#include <unordered_map>
namespace oo::Anim
{
	//tracks the progress of a gameobject's animation in an animation tree
	struct AnimationTracker
	{
		struct InTransitionInfo {
			bool in_transition{ false };
			//trackers for the destination node
			std::vector<ProgressTracker> trackers{};
			//how much time has elasped since the transition began
			float transition_timer{ 0.f };
			float transition_offset{ 0.f };
		};

		NodeRef currentNode;
		
		float timer{ 0.f };
		float normalized_timer{ 0.f };
		float global_timer{ 0.f };
		int num_iterations{ 0 };

		InTransitionInfo transition_info{};

		//event tracker, then FBX animations, then properties
		//these track the various timelines in a single animation
		std::vector<ProgressTracker> trackers;
		//a copy of the animation tree's parameters to be used for this
		//component only
		std::vector<Parameter> parameters;
	};

	
}