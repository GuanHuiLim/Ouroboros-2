/************************************************************************************//*!
\file           AnimationSkeletonStateMachine.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          
A state machine involving the skeletal mesh and the bones to be
used for animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AnimationSkeletonStateMachine.h"

namespace oo::SkAnim
{
	
}

namespace oo::SkAnim::internal
{
	void UpdateStateMachineProgress(oo::GameObject go, AnimationSkeletalComponent& comp)
	{
		//check transition conditions
		//TODO
		
		//try to transition if possible
		//TODO
		
		//blend current state and next state
		//TODO
		
		//if transition done, set current state to next state
		//TODO
		
		//update current state node
		//TODO
	}
}