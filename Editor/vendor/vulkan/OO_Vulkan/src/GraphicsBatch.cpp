#include "GraphicsBatch.h"

#include "VulkanRenderer.h"
#include "GraphicsWorld.h"
#include "gpuCommon.h"
#include <cassert>

GraphicsBatch GraphicsBatch::Init(GraphicsWorld* gw, VulkanRenderer* renderer, size_t maxObjects)
{
	assert(gw != nullptr);
	assert(renderer != nullptr);

	GraphicsBatch gb;
	gb.m_world = gw;
	gb.m_renderer = renderer;

	for (auto& batch : gb.m_batches)
	{
		batch.reserve(maxObjects);
	}
	s_scratchBuffer.reserve(maxObjects);

	return gb;
}

void AppendBatch(std::vector<oGFX::IndirectCommand>& dest, std::vector<oGFX::IndirectCommand>& src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}

void GraphicsBatch::GenerateBatches()
{
	using Batch = GraphicsBatch::DrawBatch;
	using Flags = ObjectInstanceFlags;

	auto& entities = m_world->GetAllObjectInstances();

	// clear old batches
	for (auto& batch : m_batches)
	{
		batch.clear();
	}

	int32_t currModelID{ -1 };
	for (auto& ent: entities)
	{
		auto& model = m_renderer->models[ent.modelID];

		if (ent.modelID != currModelID) // check if we are using the same model
		{
			// clear the buffer to prepare for this model
			s_scratchBuffer.clear();

			oGFX::IndirectCommand indirectCmd{};
			indirectCmd.instanceCount = 1;

			// this is the number invoked by the graphics pipeline as the instance id (location = 15) etc..
			// the number is flattened in GraphicsBatches
			indirectCmd.firstInstance = 0;

			indirectCmd.firstIndex = model.mesh->indicesOffset;
			indirectCmd.indexCount = model.mesh->indicesCount;
			indirectCmd.vertexOffset = model.mesh->vertexOffset;

			s_scratchBuffer.emplace_back(indirectCmd);
			//for (auto& node :model.nodes)
			//{
			//	uint32_t counter = 0;
			//	oGFX::IndirectCommandsHelper(node, s_scratchBuffer,counter);
			//}
		}
		
		if (ent.flags & Flags::SHADOW_CASTER)
		{
			AppendBatch(m_batches[Batch::SHADOW_CAST], s_scratchBuffer);
		}

		if (ent.flags & Flags::DYNAMIC_INSTANCE)
		{
			if (ent.flags & Flags::TRANSPARENT)
			{
				AppendBatch(m_batches[Batch::FORWARD_DYNAMIC], s_scratchBuffer);
			}
			else
			{
				AppendBatch(m_batches[Batch::GBUFFER_DYNAMIC], s_scratchBuffer);
			}
		}

		if (ent.flags & Flags::ENABLE_ZPREPASS)
		{
			AppendBatch(m_batches[Batch::ZPREPASS], s_scratchBuffer);
		}

		if (ent.flags & Flags::EMITTER)
		{
			AppendBatch(m_batches[Batch::LIGHT_SPOT], s_scratchBuffer);
		}

		// append to the batches
		AppendBatch(m_batches[Batch::ALL_OBJECTS], s_scratchBuffer);
	}

	for (auto& batch : m_batches)
	{
		// set up first instance index
		std::for_each(batch.begin(), batch.end(),
			[x = uint32_t{ 0 }](oGFX::IndirectCommand& c) mutable { 
			c.firstInstance = c.firstInstance == 0 ? x++ : x - 1;
		});
	}

}

const std::vector<oGFX::IndirectCommand>& GraphicsBatch::GetBatch(int32_t batchIdx)
{
	assert(batchIdx > -1 && batchIdx < GraphicsBatch::MAX_NUM);

	return m_batches[batchIdx];
}
