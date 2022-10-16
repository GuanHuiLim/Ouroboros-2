/************************************************************************************//*!
\file           GfxTypes.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines some helper types used by the graphics

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "MathCommon.h"
#include <unordered_map>

namespace oGFX
{
using Color = glm::vec4;

namespace Colors
{
	constexpr glm::vec4 VIOLET		= { 0.580392156862745,0,0.827450980392157,1 };
	constexpr glm::vec4 INDIGO		= { 0.294117647058824,0,0.509803921568627,1 };
	constexpr glm::vec4 BLUE		= { 0,0,1,1 };
	constexpr glm::vec4 GREEN		= { 0,1,0,1 };
	constexpr glm::vec4 YELLOW		= { 1,1,0,1 };
	constexpr glm::vec4 ORANGE		= { 1,0.498039215686275,0,1 };
	constexpr glm::vec4 RED			= { 1,0,0,1 };
	constexpr glm::vec4 WHITE		= { 1,1,1,1 };
	constexpr glm::vec4 LIGHT_GREY	= { 0.1f,0.1f,0.1f,1.0f};

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
