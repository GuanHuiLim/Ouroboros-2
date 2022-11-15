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
#include <vector>
#include <array>

class GraphicsWorld;
class VulkanRenderer;

class GraphicsBatch
{
public:
	enum DrawBatch
	{
		ALL_OBJECTS,
		SHADOW_CAST,
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

	static GraphicsBatch Init(GraphicsWorld* gw,VulkanRenderer* renderer ,size_t maxObjects);
	void GenerateBatches();
	const std::vector<oGFX::IndirectCommand>& GetBatch(int32_t batchIdx);
	

private:
	GraphicsWorld* m_world{ nullptr };
	VulkanRenderer* m_renderer{nullptr};

	std::array<std::vector<oGFX::IndirectCommand> , DrawBatch::MAX_NUM> m_batches;

	static inline std::vector<oGFX::IndirectCommand> s_scratchBuffer;

};

