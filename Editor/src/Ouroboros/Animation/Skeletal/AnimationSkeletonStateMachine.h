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
		float transitionTimer{ 0.0f };
		float transitionDuration{ 0.0f };
		

		int currentState{ -1 };
		int nextState{ -1 };
		
		ParameterList paramList{};
	};

	struct ParameterList
	{
		std::vector<rttr::variant> params;
		 
		//operator[] overload to get the parameter 
		inline rttr::variant operator[](int index) const
		{
			return params[index];
		}
		inline rttr::variant& operator[](int index)
		{
			return params[index];
		}
	};

	struct Comparator
	{
		enum class Type
		{
			Equal,
			NotEqual,
			GreaterThan,
			LessThan,
			GreaterThanOrEqual,
			LessThanOrEqual
		};

		using CompareFn = bool(rttr::variant const&, rttr::variant const&);
		using Map = std::unordered_map< Type, CompareFn*>;
		static Map const comparisonFn_map;

		Type type{ Type::Equal };

		Type GetType() const { return type; }
	};

	struct Rule
	{
		bool Check(SM_Instance& instance) const
		{
			return Comparator::comparisonFn_map.at(comparator.GetType())(instance.paramList[parameterIndex], value);
		}

	private:
		int parameterIndex{ -1 };
		rttr::variant value{};
		Comparator comparator{};
	};

	struct Transition
	{
		bool CheckConditions(SM_Instance& instance) const
		{
			for (auto const& rule : rules)
			{
				if (rule.Check(instance) == false)
				{
					return false;
				}
			}
			return true;
		}

		int NextState() const { return nextState; }
		float Duration() const { return duration; }
	private:
		std::vector<Rule> rules{};
		int nextState{ -1 };
		float duration{ 0.f };
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