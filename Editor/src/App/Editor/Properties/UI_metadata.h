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
