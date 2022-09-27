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
		static std::mt19937 mt{ std::random_device{}() };
		static std::uniform_int_distribution<size_t> distrib{ 0 };
		return distrib(mt);
	};
}

namespace oo::Anim
{
	std::unordered_map< std::string, uint> Animation::name_to_index{};
	std::unordered_map< size_t, uint> Animation::ID_to_index{};
	std::vector<Animation> Animation::animation_storage = []() {
		decltype(Animation::animation_storage) container{};
		container.reserve(internal::expected_num_anims);

		//create empty animation
		Animation empty_anim{};
		empty_anim.name = Animation::empty_animation_name;
		empty_anim.animation_ID = internal::generateUID();

		Animation::ID_to_index[empty_anim.animation_ID] = container.size();
		Animation::name_to_index[empty_anim.name] = container.size();

		container.emplace_back(std::move(empty_anim));
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
}