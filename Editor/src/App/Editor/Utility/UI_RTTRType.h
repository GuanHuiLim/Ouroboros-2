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
		STRING_TYPE,
		UUID_TYPE,
	};

	inline static std::unordered_map<rttr::type::type_id, UItypes> types;
	
};
