/************************************************************************************//*!
\file           AnimationCondition.cpp
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
#include "pch.h"
#include "AnimationCondition.h"
#include "AnimationNode.h"
#include "AnimationGroup.h"
#include "AnimationTree.h"
#include "AnimationInternal.h"

#include <rttr/registration>

namespace oo::Anim
{
	Condition::CompareFnMap const Condition::comparisonFn_map = []()
	{
		rttr::type::register_comparators<bool>();
		rttr::type::register_comparators<float>();
		rttr::type::register_comparators<int>();

		Condition::CompareFnMap map;
		/*-------------------
		BOOL / TRIGGER
		-------------------*/
		map[P_TYPE::BOOL][Condition::CompareType::EQUAL] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs == rhs;
		};
		map[P_TYPE::TRIGGER][Condition::CompareType::EQUAL] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs == rhs;
		};

		/*-------------------
		FLOAT
		-------------------*/
		map[P_TYPE::FLOAT][Condition::CompareType::GREATER] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs > rhs;
		};
		map[P_TYPE::FLOAT][Condition::CompareType::LESS] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs < rhs;
		};
		/*-------------------
		INT
		-------------------*/
		map[P_TYPE::INT][Condition::CompareType::EQUAL] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs == rhs;
		};
		map[P_TYPE::INT][Condition::CompareType::NOT_EQUAL] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs != rhs;
		};
		map[P_TYPE::INT][Condition::CompareType::GREATER] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs > rhs;
		};
		map[P_TYPE::INT][Condition::CompareType::LESS] =
			[](DataType const& lhs, DataType const& rhs)
		{
			return lhs < rhs;
		};

		return map;
	}();
}
namespace oo::Anim::internal
{
	void SerializeCondition(rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Condition& condition)
	{
		writer.StartObject();
		{
			//properties
			{
				rttr::instance obj{ condition };
				auto properties = rttr::type::get<Condition>().get_properties();
				for (auto& prop : properties)
				{
					writer.Key(prop.get_name().data(), static_cast<rapidjson::SizeType>(prop.get_name().size()));
					rttr::variant val{ prop.get_value(obj) };
					internal::serializeDataFn_map.at(prop.get_type().get_id())(writer, val);
				}
			}
		}
		writer.EndObject();
	}
}
namespace oo::Anim
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
		registration::class_<Condition>("Animation Condition")
			.enumeration<Condition::CompareType>("Comparison Type")
			(
				value("GREATER", Condition::CompareType::GREATER),
				value("LESS", Condition::CompareType::LESS),
				value("EQUAL", Condition::CompareType::EQUAL),
				value("NOT_EQUAL", Condition::CompareType::NOT_EQUAL)
			)
			.property("paramID", &Condition::paramID)
			.property("comparison_type", &Condition::comparison_type)
			.property("type", &Condition::type)
			.property("value" , &Condition::value)
			.method(internal::serialize_method_name, &internal::SerializeCondition)
			;
	}


	Condition::Condition(ConditionInfo const& info)
	: comparison_type{ info.comparison }
	, type{ info._param->type }
	//if intial value is set, use that, otherwise use default value based on type 
	, value{ info.value.is_valid() ? info.value : internal::ConditionDefaultValue(info._param->type) }
	, paramID{ info._paramID }
	{
		//for triggers, only care if trigger boolean is true
		if (type == P_TYPE::TRIGGER)
			value = true;
	}

	bool Condition::Satisfied(AnimationTracker& tracker)
	{
		return internal::ConditionSatisfied(*this, tracker);
	}

	std::string Condition::GetName(AnimationTree const& tree)
	{
		assert(parameterIndex < tree.parameters.size());
		return tree.parameters[parameterIndex].name;
	}
}