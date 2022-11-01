/************************************************************************************//*!
\file           AnimationCondition.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           August 26, 2022
\brief          
A transition condition between two nodes

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Anim_Utils.h"

namespace oo::Anim
{
	struct Condition
	{
		enum class CompareType : int
		{
			GREATER,
			LESS,
			EQUAL,
			NOT_EQUAL
		};
		using DataType = rttr::variant;
		using CompareFn = bool(DataType const&, DataType const&);
		using CompareFnMap = std::unordered_map< P_TYPE, std::unordered_map<CompareType, CompareFn*>>;

		CompareType comparison_type;
		P_TYPE type;
		DataType value{};
		//used to track the parameter's index in the animation tree's vector
		size_t paramID{ internal::invalid_ID };
		uint32_t parameterIndex{};
		CompareFn* compareFn{ nullptr };
		static CompareFnMap const comparisonFn_map;

		Condition() = default;
		Condition(ConditionInfo const& info);
		bool Satisfied(AnimationTracker& tracker);
		std::string GetName(AnimationTree const& tree);

		RTTR_ENABLE();
	};

	struct ConditionInfo
	{
		Condition::CompareType comparison{};
		std::string parameter_name{};
		//initial value, leave empty for default
		Condition::DataType value{};
		//dont fill this
		size_t _paramID{ internal::invalid_ID };
		//dont fill this
		Parameter* _param{ nullptr };
	};
}