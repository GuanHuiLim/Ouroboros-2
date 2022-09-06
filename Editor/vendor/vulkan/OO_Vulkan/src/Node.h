#pragma once
#include <vector>
#include <string>

namespace oGFX { struct Mesh; }

struct Node
{
	Node* parent{ nullptr };
	uint32_t index{};
	std::vector<Node*> children;
	std::vector<oGFX::Mesh*> meshes;
	std::string name;
	~Node();
};

