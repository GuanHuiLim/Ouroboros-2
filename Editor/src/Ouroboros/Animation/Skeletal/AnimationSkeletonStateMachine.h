/************************************************************************************//*!
\file           AnimationSkeletonStateMachine.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A state machine involving the skeletal mesh and the bones to be 
used for animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "Ouroboros/ECS/GameObject.h"
#include "AnimationSkeletalComponent.h"

namespace oo::SkAnim
{
	class AnimationSkeletonStateMachine
	{

	private:

	};

	struct AnimationSkeletonStateMachineInstance
	{
		bool canTransition{ false };
		bool inTransition{ false };

		int currentState{ -1 };
		int nextState{ -1 };
		
	};
}


namespace oo::SkAnim::internal
{
	void UpdateStateMachineProgress(oo::GameObject go, AnimationSkeletalComponent& comp);
}