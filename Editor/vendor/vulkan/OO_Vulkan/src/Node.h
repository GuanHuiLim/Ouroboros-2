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

