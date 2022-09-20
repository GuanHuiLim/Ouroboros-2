/************************************************************************************//*!
\file          NavGraphComponent.h
\project       Ouroboros
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b
\par           email: muhammadamirul.b\@digipen.edu
\date          September 13, 2022
\brief         File contains the declaration for NavGraphComponent

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <Quaternion/include/Transform.h>

#include <rttr/type>
#include <vector>

#include "Node.h"

namespace oo
{
	class NavGraphComponent
	{
		using Vector2 = glm::vec2;
		using Vector3 = Transform::vec3;

	public:
		//Modifiable Data
		Vector2 gridWorldSize;
		std::vector<std::vector<Node*>> grid;
		float nodeRadius;
		float nodeHeight;
		bool drawGizmos = false;

		RTTR_ENABLE();

	private:
		float nodeDiameter;
		int gridSizeX, gridSizeY;
		int MaxSize;
		Vector3 worldBottomLeft;

		friend class NavSystem;
	};
}
