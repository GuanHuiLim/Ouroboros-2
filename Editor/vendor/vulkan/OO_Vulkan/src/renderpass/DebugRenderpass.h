#pragma once

#include "GfxRenderpass.h"
#include "vulkan/vulkan.h"

#include <array>

// This render pass support the drawing for debug objects, typically for editor or debugging purpose and not shipped in the final product.
// What kinds of things should this renderer do at this level? (non-exhaustive)
// - Drawing of points, lines, triangles, alpha tested or not.
// - Drawing of wireframe and solid meshes, alpha tested or not.
// - Drawing of debug text. (although you can just piggyback ImGui)
// - Support of timed and persistent debug drawing. (Draw for 5 seconds, draw permanently, etc)
class DebugDrawRenderpass : public GfxRenderpass
{
public:

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	VkRenderPass debugRenderpass{};

private:

	void CreateDebugRenderpass();
	void CreatePipeline();
	void InitDebugBuffers();

	struct DebugDrawPSOSelector
	{
		std::array<VkPipeline, 6> psos = {};

		// Ghetto... Need a more robust solution
		VkPipeline GetPSO(bool isDepthTest, bool isWireframe, bool isPoint)
		{
			int i = isWireframe ? 1 : 0;
			i = isPoint ? 2 : i;
			int j = isDepthTest ? 1 : 0;
			const int idx = i + 2 * j;
			return psos[idx];
		}

	}m_DebugDrawPSOSelector;
};

