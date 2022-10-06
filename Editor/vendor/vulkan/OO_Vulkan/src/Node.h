/************************************************************************************//*!
\file           Node.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a node object which represents a scenegraph

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <vector>
#include <string>
#include "MathCommon.h"

struct Node
{
	std::string name;
	Node* parent{ nullptr };
	glm::mat4 transform{ 1.0f };
	std::vector<Node*> children;
	uint32_t meshRef{ static_cast<uint32_t>(-1) }; // references a mesh in the gfxMeshIndices
	~Node();
};

