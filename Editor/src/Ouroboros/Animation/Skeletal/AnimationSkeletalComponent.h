/************************************************************************************//*!
\file           AnimationSkeletalComponent.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          A component for skeletal mesh based animation.


Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "../Anim_Utils.h"
#include "AnimationSkeletonStateMachine.h"
#include "AnimationSkeleton.h"

namespace oo::SkAnim
{
	// contains animation state and pose for a particular character or object
	struct AnimationSkeletalComponent
	{
		/*--------------------
		serialized data
		----------------------*/


		/*--------------------
		non-serialized data
		----------------------*/
		//animation state machine
		AnimationSkeletonStateMachine* stateMachine{nullptr};
		//animation state machine instance
		SM_Instance stateMachineInstance{};
		//animation skeleton
		Skeleton skeleton{};
		//animation pose
		Pose pose{};
	};

	
}
