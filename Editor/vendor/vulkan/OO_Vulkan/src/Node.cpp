/************************************************************************************//*!
\file           Node.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Node object destructor

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "Node.h"
#include "Mesh.h"

Node::~Node()
{
	for (auto& child : children)
	{
		delete child;
		child = nullptr;
	}
	//for (auto& mesh : meshes)
	//{
	//	delete mesh;
	//	mesh = nullptr;
	//}
}
