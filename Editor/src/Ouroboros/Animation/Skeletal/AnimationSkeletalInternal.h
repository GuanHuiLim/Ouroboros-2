/************************************************************************************//*!
\file           AnimationSkeletalInternal.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          Internal functions for skeletal animation.


Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "AnimationSkeletal_Utils.h"

namespace oo::SkAnim::internal
{
	void CheckTransitionConditions(SM_Instance& instance, AnimationSkeletonStateMachine const& statemachine);

	void TryTransitionIfPossible(SM_Instance& instance, AnimationSkeletonStateMachine const& statemachine);

	void TryUpdateTransitionState(SM_Instance& instance, AnimationSkeletonStateMachine const& statemachine);

	void UpdateState(SM_Instance& instance, AnimationSkeletonStateMachine const& statemachine);

	void ConsumeTriggers(SM_Instance& instance, Transition const& transition);
}


namespace oo::SkAnim
{
	
}
