#include "Node.h"
#include "Mesh.h"

Node::~Node()
{
	for (auto& child : children)
	{
		delete child;
		child = nullptr;
	}
	for (auto& mesh : meshes)
	{
		delete mesh;
		mesh = nullptr;
	}
}
