#pragma once
#include "GfxRenderpass.h"
#include "vulkan/vulkan.h"

struct DebugRenderpass : public GfxRenderpass
{

	//DECLARE_RENDERPASS_SINGLETON(DebugRenderpass)

	VkRenderPass debugRenderpass{};
	VkPipeline linesPipeline{};
	VkPushConstantRange pushConstantRange;

	void Init() override;
	void Draw() override;
	void Shutdown() override;


	void CreatePushconstants();
	void CreateDebugRenderpass();
	void CreatePipeline();
	void InitDebugBuffers();
};

