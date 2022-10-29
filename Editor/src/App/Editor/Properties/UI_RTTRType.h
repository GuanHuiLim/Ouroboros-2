/************************************************************************************//*!
\file          UI_RTTRType.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Maps rttr values into an Enum 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
		UINT_TYPE,
		INT_TYPE,
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
		ASSET_TYPE,
		MESH_INFO_TYPE,
	};

	inline static std::unordered_map<rttr::type::type_id, UItypes> types;
	
};
