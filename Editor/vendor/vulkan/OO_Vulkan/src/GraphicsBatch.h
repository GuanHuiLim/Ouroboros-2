/************************************************************************************//*!
\file           GraphicsBatch.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares GraphicsBatch, a generator for command lists for objects that require to be rendered.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "gpuCommon.h"
#include "GraphicsWorld.h"
#include <vector>
#include <array>
#include <mutex>
#include "Font.h"

class VulkanRenderer;

class GraphicsWorld;

constexpr size_t MAX_LIGHTS = 3;

class GraphicsBatch
{
public:
	enum DrawBatch
	{
		ALL_OBJECTS,
		SHADOW_OCCLUDER,
		SHADOW_RECV,
		SHADOW_LIGHT,
		GBUFFER_STATIC,
		GBUFFER_DYNAMIC,
		FORWARD_STATIC,
		FORWARD_DYNAMIC,
		ZPREPASS,
		LIGHT_SPOT,
		MAX_NUM
	};

	

	void Init(GraphicsWorld* gw,VulkanRenderer* renderer ,size_t maxObjects);
	void GenerateBatches();
	void ProcessLights();
	void ProcessGeometry();
	void ProcessUI();
	void ProcessParticleEmitters();
	const std::vector<oGFX::IndirectCommand>& GetBatch(int32_t batchIdx);
	const std::vector<oGFX::IndirectCommand>& GetParticlesBatch();
	const std::vector<ParticleData>& GetParticlesData();
	const std::vector<oGFX::UIVertex>& GetUIVertices();
	const std::vector<LocalLightInstance>& GetLocalLights();
	const std::vector<LocalLightInstance>& GetShadowCasters();
	size_t GetScreenSpaceUIOffset() const;
	// TODO :: need to return indices out if i am doing fill
	
	void GenerateTextGeometry(const UIInstance& ui);
	void GenerateSpriteGeometry(const UIInstance& ui);
	
	size_t m_numShadowCastGrids{};


	GraphicsWorld* m_world{ nullptr };
	VulkanRenderer* m_renderer{nullptr};

	std::array<std::vector<oGFX::IndirectCommand>, DrawBatch::MAX_NUM> m_batches;
	std::vector<ParticleData> m_particleList;
	std::vector<oGFX::IndirectCommand> m_particleCommands;
	std::vector<oGFX::UIVertex> m_uiVertices;
	std::mutex m_uiVertMutex;

	std::vector<LocalLightInstance>m_culledLights;
	std::vector<LocalLightInstance>m_shadowCasters;

	std::vector<DrawData> m_culledCameraObjects;

	struct CastersData {		
		std::vector<oGFX::IndirectCommand> m_commands [6];
		std::vector<DrawData> m_culledObjects [6];
	};
	std::vector<CastersData> m_casterData;

	static inline std::vector<oGFX::IndirectCommand> s_scratchBuffer;

	size_t m_SSVertOffset{};

};

