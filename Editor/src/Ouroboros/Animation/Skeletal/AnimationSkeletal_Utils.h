/************************************************************************************//*!
\file           AnimationSkeletal_Utils.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          Utility header for Skeletal Animation.


Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "../Anim_Utils.h"

namespace oo::SkAnim
{
	class Pose; //represents a hierarchy of bones of skeletal mesh transforms
	class Skeleton;
	class AnimationSkeletalSystem;
	struct AnimationSkeletalComponent;
	class AnimationSkeletonStateMachine;
	struct AnimationSkeletonStateMachineInstance;
	using SM_Instance = AnimationSkeletonStateMachineInstance;
	struct StateNode;
	struct Transition;
	struct Rule;
	struct Comparator;
	struct ParameterList;
	struct Trigger
	{
		bool value{ false };
		void Reset() { value = false; }
	};
}