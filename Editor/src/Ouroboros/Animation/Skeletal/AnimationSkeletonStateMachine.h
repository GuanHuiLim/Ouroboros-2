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
#include "rttr/variant.h"

namespace oo::SkAnim
{
	class AnimationSkeletonStateMachine
	{
	public:
		StateNode const* GetState(int state) const
		{
			if (state < 0 || state > nodes.size())
			{
				assert(false);
				return nullptr;
			}
			return &(nodes.at(state).get_value<StateNode>());
		}
	private:
		std::vector<rttr::variant> nodes{};
	};

	struct AnimationSkeletonStateMachineInstance
	{
		bool canTransition{ false };
		bool inTransition{ false };
		Transition const* transition{nullptr};

		int currentState{ -1 };
		int nextState{ -1 };
		
	};

	struct Rule
	{
		bool Check(SM_Instance& instance) const
		{

		}

	private:
		int parameterIndex{ -1 };
		rttr::variant value;
		
	};

	struct Transition
	{
		bool CheckConditions(SM_Instance& instance) const
		{
			for (auto const& rule : rules)
			{
				rule.Check(instance);
			}
		}

	private:
		std::vector<Rule> rules;
	};
	struct StateNode
	{
		std::vector<Transition> transitions{};
	};


}


namespace oo::SkAnim::internal
{
	void UpdateStateMachineProgress(oo::GameObject go, AnimationSkeletalComponent& comp);
}