/************************************************************************************//*!
\file          Node.h
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b
\par           email: muhammadamirul.b\@digipen.edu
\date          September 13, 2022
\brief         File contains the declaration of the node in the A* Pathfinding Algorithm

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <Quaternion/include/Transform.h>

namespace oo
{
	class Node
	{
		using Vector3 = glm::vec3;

	public:
		bool walkable;
		Vector3 worldPosition;
		int gridX;
		int gridY;

		int gCost;
		int hCost;
		Node* parent;

		Node(bool _walkable, Vector3 _worldPos, int _gridX, int _gridY);

		int getFCost() const;
	};
}
