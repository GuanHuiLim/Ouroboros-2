#include "gpuCommon.h"

void oGFX::IndirectCommandsHelper(Node * node, std::vector<oGFX::IndirectCommand>& m_DrawIndirectCommandsCPU)
{	
	if (node->meshes.size())
	{
		//for (size_t i = 0; i < OBJECT_INSTANCE_COUNT; i++)
		{
			oGFX::IndirectCommand indirectCmd{};
			indirectCmd.instanceCount = 1;
			
			// TODO: handle
			//indirectCmd.firstInstance = static_cast<uint32_t>(m /* *OBJECT_INSTANCE_COUNT + i*/);

			// @todo: Multiple primitives
			// A glTF node may consist of multiple primitives, so we may have to do multiple commands per mesh
			indirectCmd.firstIndex = node->meshes[0]->indicesOffset;
			indirectCmd.indexCount = node->meshes[0]->indicesCount;
			indirectCmd.vertexOffset = node->meshes[0]->vertexOffset;

			// for counting
			//vertexCount += node->meshes[0]->vertexCount;
			m_DrawIndirectCommandsCPU.emplace_back(indirectCmd);

		}
		//m++;
	}
	for (auto& child : node->children)
	{
		IndirectCommandsHelper(child, m_DrawIndirectCommandsCPU);
	}

}