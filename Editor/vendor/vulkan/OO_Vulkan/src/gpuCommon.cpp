/************************************************************************************//*!
\file           gpuCommon.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              used to be have a helper

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "gpuCommon.h"

void oGFX::IndirectCommandsHelper(Node * node, std::vector<oGFX::IndirectCommand>& m_DrawIndirectCommandsCPU, uint32_t& counter)
{	
	//for (auto& mesh: node->meshes)
	//{
	//	oGFX::IndirectCommand indirectCmd{};
	//	indirectCmd.instanceCount = 1;
	//	
	//	// this is the number invoked by the graphics pipeline as the instance id (location = 15) etc..
	//	// the number is flattened in GraphicsBatches
	//	indirectCmd.firstInstance = counter++; 
	//
	//	// A node may consist of multiple primitives, so we may have to do multiple commands per mesh
	//	indirectCmd.firstIndex = mesh->indicesOffset;
	//	indirectCmd.indexCount = mesh->indicesCount;
	//	indirectCmd.vertexOffset = mesh->vertexOffset;
	//
	//	m_DrawIndirectCommandsCPU.emplace_back(indirectCmd);
	//}
	//for (auto& child : node->children)
	//{
	//	IndirectCommandsHelper(child, m_DrawIndirectCommandsCPU, counter);
	//}

}
