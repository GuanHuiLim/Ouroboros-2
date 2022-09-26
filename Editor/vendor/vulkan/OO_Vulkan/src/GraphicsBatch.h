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

