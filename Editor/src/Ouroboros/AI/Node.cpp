/************************************************************************************//*!
\file          Node.cpp
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b
\par           email: muhammadamirul.b\@digipen.edu
\date          September 13, 2022
\brief         File contains the definition of the node in the A* Pathfinding Algorithm

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "Node.h"

namespace oo
{
	Node::Node(bool _walkable, Vector3 _worldPos, int _gridX, int _gridY)
	: walkable(_walkable),
	  worldPosition(_worldPos),
	  gridX(_gridX),
	  gridY(_gridY),
	  gCost(-1),
	  hCost(-1),
	  parent(nullptr)
	{

	}

	int Node::getFCost() const
	{
		return gCost + hCost;
	}
}
