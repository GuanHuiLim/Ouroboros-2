/************************************************************************************//*!
\file          UI_metadata.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         used for declaring rttr variables if developer wants to change some fields
			   for the variable.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
class UI_metadata
{
public:
	//for oo::Asset (affects asset browser)
	//takes in an integer corresponding to the AssetInfo::Type enum
	static const constexpr unsigned char ASSET_TYPE = 255;
	//for Drag bars (int, float, vec2, vec3, vec4, etc...)
	//takes in float
	static const constexpr unsigned char DRAG_SPEED = 100;
	//for all variables
	//takes in bool
	static const constexpr unsigned char HIDDEN = 0;
};
