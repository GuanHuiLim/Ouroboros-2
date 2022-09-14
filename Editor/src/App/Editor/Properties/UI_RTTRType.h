#pragma once
#include <unordered_map>
#include <rttr/type.h>


class UI_RTTRType
{
public:
	static void Init();
	enum class UItypes
	{
		BOOL_TYPE = 0,
		FLOAT_TYPE,
		STRING_TYPE,
		UUID_TYPE,

		VEC2_TYPE,
		VEC3_TYPE,
		VEC4_TYPE,
		MAT3_TYPE,
		MAT4_TYPE,
		QUAT_TYPE,

		PATH_TYPE,

		DOUBLE_TYPE,
		SIZE_T_TYPE,
		ENTITY_TYPE,
	};

	inline static std::unordered_map<rttr::type::type_id, UItypes> types;
	
};
