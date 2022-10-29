/************************************************************************************//*!
\file           Anim_Utils.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          Utility definitions for Animation

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Anim_Utils.h"
#include "Archetypes_Ecs/src/A_Ecs.h"

#include <rttr/instance.h>
namespace oo::Anim::internal
{
	template<typename T>
	auto assign_to_map = [](std::unordered_map< size_t, rttr::instance(*)(void*)>& map) {
		//map.emplace(, &conversion_fn<T>);

		map[Ecs::ECSWorld::get_component_hash<T>()] = [](void* ptr) {
			T& ref = *(static_cast<T*>(ptr));
			return rttr::instance{ ref };
		};
	};
	std::unordered_map< size_t, rttr::instance(*)(void*)> hash_to_instance{};

	void Initialise_hash_to_instance()
	{
		if (hash_to_instance.empty() == false) return;

		assign_to_map<TransformComponent>(hash_to_instance);

		/*hash_to_instance =
			[]() {
			std::unordered_map< size_t, rttr::instance(*)(void*)> map;

			assign_to_map<TransformComponent>(map);

			return map;
		}();*/
	}

	size_t generateUID()
	{
		static std::mt19937_64 mt{ std::random_device{}() };
		static std::uniform_int_distribution<size_t> distrib{ 0 };
		return distrib(mt);
	};
}

namespace oo::Anim
{
	std::unordered_map< std::string, size_t> Animation::name_to_ID{};
	std::map<size_t, Animation> Animation::animation_storage = []() {
		decltype(Animation::animation_storage) container{};
		//container.reserve(internal::expected_num_anims);

		//create empty animation
		Animation empty_anim{};
		empty_anim.name = Animation::empty_animation_name;

		Animation::name_to_ID[empty_anim.name] = empty_anim.animation_ID;
		//Animation::name_to_index[empty_anim.name] = static_cast<uint>(container.size());
		auto key = empty_anim.animation_ID;
		container.emplace(key, std::move(empty_anim));
		return container;
	}();

	Condition::CompareFnMap Condition::comparisonFn_map = []()
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


	Parameter::SerializeFnMap Parameter::serializeFn_map = 
		[]() {
		Parameter::SerializeFnMap map;
		
		map[P_TYPE::BOOL] =
		[](rapidjson::PrettyWriter<rapidjson::OStreamWrapper>& writer, Parameter& param)
		{
			writer.String("BOOL", static_cast<rapidjson::SizeType>(std::string("BOOL").size()));
			writer.Bool(param.value.get_value<bool>());
		};

		return map;
		}();

}