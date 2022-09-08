#pragma once

#include "GfxRenderpass.h"
#include "vulkan/vulkan.h"

#include "VulkanFramebufferAttachment.h"

#include <memory>

struct DeferredCompositionRenderpass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(DeferredCompositionRenderpass)
	
	void Init() override;
	void Draw() override;
	void Shutdown() override;

	void CreatePSO() override;

	VkRenderPass renderpass_DeferredLightingComposition{};
	VkPipelineLayout layout_DeferredLightingComposition{};
	VkPipeline pso_DeferredLightingComposition{};

	uint64_t uboDynamicAlignment{};

	void CreatePipeline();
private:
	void CreateDescriptors();
};

