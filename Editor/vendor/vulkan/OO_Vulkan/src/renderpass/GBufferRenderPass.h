#pragma once

#include "GfxRenderpass.h"
#include "vulkan/vulkan.h"
#include "imgui.h"
#include "VulkanFramebufferAttachment.h"

#include <array>

struct GBufferRenderPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(GBufferRenderPass)

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	void CreatePSO() override;
	
	VulkanFramebufferAttachment att_albedo;
	VulkanFramebufferAttachment att_position;
	VulkanFramebufferAttachment att_normal;
	VulkanFramebufferAttachment att_material;
	VulkanFramebufferAttachment att_depth;

	// This is for ImGui
	std::array<ImTextureID, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> deferredImg;

	VkRenderPass renderpass_GBuffer;
	VkFramebuffer framebuffer_GBuffer;

	//VkPushConstantRange pushConstantRange;
	VkPipeline pso_GBufferDefault;

private:
	void SetupRenderpass();
	void SetupFramebuffer();
	void CreatePipeline();

};