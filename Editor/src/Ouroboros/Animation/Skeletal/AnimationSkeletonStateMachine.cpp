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
#include "AnimationSkeletalInternal.h"
namespace oo::SkAnim
{
	Comparator::Map const Comparator::comparisonFn_map = []() {
		rttr::type::register_comparators<bool>();
		rttr::type::register_comparators<float>();
		rttr::type::register_comparators<int>();
		
		Map map{};

		map[Type::Equal] = [](rttr::variant const& lhs, rttr::variant const& rhs)				{ return lhs == rhs; };
		map[Type::NotEqual] = [](rttr::variant const& lhs, rttr::variant const& rhs)			{ return lhs != rhs; };
		map[Type::GreaterThan] = [](rttr::variant const& lhs, rttr::variant const& rhs)			{ return lhs > rhs; };
		map[Type::LessThan] = [](rttr::variant const& lhs, rttr::variant const& rhs)			{ return lhs < rhs; };
		map[Type::GreaterThanOrEqual] = [](rttr::variant const& lhs, rttr::variant const& rhs)	{ return lhs >= rhs; };
		map[Type::LessThanOrEqual] = [](rttr::variant const& lhs, rttr::variant const& rhs)		{ return lhs <= rhs; };

		return map;
	}();
}

namespace oo::SkAnim::internal
{
	void UpdateStateMachineProgress(oo::GameObject go, AnimationSkeletalComponent& comp)
	{
		//check transition conditions
		CheckTransitionConditions(comp.stateMachineInstance, *comp.stateMachine);
		
		//try to transition if possible
		TryTransitionIfPossible(comp.stateMachineInstance, *comp.stateMachine);
		
		//blend current state and next state if in transition
		if (comp.stateMachineInstance.inTransition)
		{
			//blend current state and next state
			BlendStates(comp.stateMachineInstance, *comp.stateMachine);
		}
		
		//if transition done, set current state to next state
		TryUpdateTransitionState(comp.stateMachineInstance, *comp.stateMachine);

		//update current state node
		UpdateState(comp.stateMachineInstance, *comp.stateMachine);
	}
}