/************************************************************************************//*!
\file           GraphicsBatch.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines GraphicsBatch, a generator for command lists for objects that require to be rendered.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "GraphicsBatch.h"

#include "VulkanRenderer.h"
#include "GraphicsWorld.h"
#include "gpuCommon.h"
#include <cassert>
#include "Profiling.h"

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
	PROFILE_SCOPED("Generate graphics batch");
	using Batch = GraphicsBatch::DrawBatch;
	using Flags = ObjectInstanceFlags;

	auto& entities = m_world->GetAllObjectInstances();

	// clear old batches
	for (auto& batch : m_batches)
	{
		batch.clear();
	}

	int32_t currModelID{ -1 };
	int32_t cnt{ 0 };
	for (auto& ent: entities)
	{
		auto& model = m_renderer->g_globalModels[ent.modelID];

		// skip entities dont want to render
		if (ent.isRenderable() == false)
		{
			// still increment instance
			++cnt;
			continue;
		}

		if (ent.modelID != currModelID) // check if we are using the same model
		{
			s_scratchBuffer.clear();
			for (size_t i = 0; i < model.m_subMeshes.size(); i++)
			{
				if (ent.submesh[i] == true)
				{
					const auto& subMesh = model.m_subMeshes[i];
					// clear the buffer to prepare for this model
					oGFX::IndirectCommand indirectCmd{};
					indirectCmd.instanceCount = 1;

					// this is the number invoked by the graphics pipeline as the instance id (location = 15) etc..
					// the number represents the index into the InstanceData array see VulkanRenderer::UploadInstanceData();
					indirectCmd.firstInstance = cnt++;

					indirectCmd.firstIndex = model.baseIndices + subMesh.baseIndices;
					indirectCmd.indexCount = subMesh.indicesCount;
					indirectCmd.vertexOffset = model.baseVertex + subMesh.baseVertex;

					s_scratchBuffer.emplace_back(indirectCmd);
				}
			}			
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

		if (ent.flags & Flags::SHADOW_ENABLED)
		{
			// get shadow enabled lights
			AppendBatch(m_batches[Batch::SHADOW_LIGHT], s_scratchBuffer);
		}

		// append to the batches
		AppendBatch(m_batches[Batch::ALL_OBJECTS], s_scratchBuffer);
		
	}

	for (auto& batch : m_batches)
	{
		// set up first instance index
		//std::for_each(batch.begin(), batch.end(),
		//	[x = uint32_t{ 0 }](oGFX::IndirectCommand& c) mutable { 
		//	c.firstInstance = c.firstInstance == 0 ? x++ : x - 1;
		//});
	}

}

const std::vector<oGFX::IndirectCommand>& GraphicsBatch::GetBatch(int32_t batchIdx)
{
	assert(batchIdx > -1 && batchIdx < GraphicsBatch::MAX_NUM);

	return m_batches[batchIdx];
}
