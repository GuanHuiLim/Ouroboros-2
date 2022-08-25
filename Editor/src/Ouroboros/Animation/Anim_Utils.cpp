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
		};;
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
}

namespace oo::Anim
{
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
}