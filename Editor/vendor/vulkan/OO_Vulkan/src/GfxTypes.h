#pragma once

#include "MathCommon.h"
#include <unordered_map>

namespace oGFX
{
using Color = glm::vec4;

namespace Colors
{
	constexpr glm::vec4 VIOLET	= { 0.580392156862745,0,0.827450980392157,1 };
	constexpr glm::vec4 INDIGO	= { 0.294117647058824,0,0.509803921568627,1 };
	constexpr glm::vec4 BLUE	= { 0,0,1,1 };
	constexpr glm::vec4 GREEN	= { 0,1,0,1 };
	constexpr glm::vec4 YELLOW	= { 1,1,0,1 };
	constexpr glm::vec4 ORANGE	= { 1,0.498039215686275,0,1 };
	constexpr glm::vec4 RED		= { 1,0,0,1 };
	constexpr glm::vec4 WHITE	= { 1,1,1,1 };

	inline static std::vector<Color>c{
		{ 0,1,0,1 },								// GREEN	
		{ 1,1,0,1 },								// YELLOW	
		{ 0,0,1,1 },								// BLUE	
		{ 1,0.498039215686275,0,1 },				// ORANGE	
		{ 1,0,0,1 },								// RED		
		{ 0.580392156862745,0,0.827450980392157,1 },// VIOLET	
		{ 0.294117647058824,0,0.509803921568627,1 },// INDIGO	
		{ 1,1,1,1 },								// WHITE
		{ 1,1,1,1 },								// WHITE
	};

	
}// end namespace colors

}// end namespace oGFX