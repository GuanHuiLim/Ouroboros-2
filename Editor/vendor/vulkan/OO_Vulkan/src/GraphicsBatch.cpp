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
#include "MathCommon.h"
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

	for (auto& light : m_world->GetAllOmniLightInstances())
	{
		constexpr glm::vec3 up{ 0.0f,1.0f,0.0f };
		constexpr glm::vec3 right{ 1.0f,0.0f,0.0f };
		constexpr glm::vec3 forward{ 0.0f,0.0f,-1.0f };

		light.view[0] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position)+-up ,		glm::vec3{ 0.0f, 0.0f,-1.0f });
		light.view[1] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position)+up,		glm::vec3{ 0.0f, 0.0f, 1.0f });
		light.view[2] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position)+-right,	glm::vec3{ 0.0f,1.0f, 0.0f });
		light.view[3] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position)+right,		glm::vec3{ 0.0f,1.0f, 0.0f });
		light.view[4] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position)+-forward,	glm::vec3{ 0.0f,-1.0f, 0.0f });
		light.view[5] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position)+forward,	glm::vec3{ 0.0f,-1.0f, 0.0f });

		light.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -100.0f, 100.0f);
		light.projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	}

	auto& allEmitters = m_world->GetAllEmitterInstances();
	m_particleList.clear();
	m_particleCommands.clear();
	/// Create parciles batch
	uint32_t emitterCnt = 0;
	auto* vr = VulkanRenderer::get();
	for (auto& emitter:allEmitters)
	{
		// note to support multiple textures permesh we have to do this per submesh 
		//setup instance data	
		// TODO: this is really bad fix this
		// This is per entity. Should be per material.
		uint32_t albedo = emitter.bindlessGlobalTextureIndex_Albedo;
		uint32_t normal = emitter.bindlessGlobalTextureIndex_Normal;
		uint32_t roughness = emitter.bindlessGlobalTextureIndex_Roughness;
		uint32_t metallic = emitter.bindlessGlobalTextureIndex_Metallic;
		constexpr uint32_t invalidIndex = 0xFFFFFFFF;
		if (albedo == invalidIndex)
			albedo = vr->whiteTextureID; 
		if (normal == invalidIndex)
			normal = vr->blackTextureID;
		if (roughness == invalidIndex)
			roughness = vr->whiteTextureID; 
		if (metallic == invalidIndex)
			metallic = vr->blackTextureID;

		// Important: Make sure this index packing matches the unpacking in the shader
		const uint32_t albedo_normal = albedo << 16 | (normal & 0xFFFF);
		const uint32_t roughness_metallic = roughness << 16 | (metallic & 0xFFFF);
		for (auto& pd : emitter.particles)
		{
			pd.instanceData.z=albedo_normal;
			pd.instanceData.w=roughness_metallic;
		}

		// copy list
		m_particleList.insert(m_particleList.end(), emitter.particles.begin(), emitter.particles.end());

		auto& model = m_renderer->g_globalModels[emitter.modelID];
		// set up the commands and number of particles
		oGFX::IndirectCommand cmd{};

		cmd.instanceCount = emitter.particles.size();
		// this is the number invoked by the graphics pipeline as the instance id (location = 15) etc..
		// the number represents the index into the InstanceData array see VulkanRenderer::UploadInstanceData();
		cmd.firstInstance = emitterCnt;
		for (size_t i = 0; i < emitter.submesh.size(); i++)
		{
			// create a draw call for each submesh using the same instance data
			if (emitter.submesh[i] == true)
			{
				const auto& subMesh = model.m_subMeshes[i];
				cmd.firstIndex = model.baseIndices + subMesh.baseIndices;
				cmd.indexCount = subMesh.indicesCount;
				cmd.vertexOffset = model.baseVertex + subMesh.baseVertex;
				m_particleCommands.push_back(cmd);
			}
		}	
		//increment instance data
		emitterCnt += cmd.instanceCount;

		// clear this so next draw we dont care
		emitter.particles.clear();
	}

}

const std::vector<oGFX::IndirectCommand>& GraphicsBatch::GetBatch(int32_t batchIdx)
{
	assert(batchIdx > -1 && batchIdx < GraphicsBatch::MAX_NUM);

	return m_batches[batchIdx];
}

const std::vector<oGFX::IndirectCommand>& GraphicsBatch::GetParticlesBatch()
{
	return m_particleCommands;
}

const std::vector<ParticleData>& GraphicsBatch::GetParticlesData()
{
	return m_particleList;
}
