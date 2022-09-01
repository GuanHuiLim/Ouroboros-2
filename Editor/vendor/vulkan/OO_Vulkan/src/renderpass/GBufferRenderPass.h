#pragma once

#include "GfxRenderpass.h"
#include "vulkan/vulkan.h"
#include "imgui/imgui.h"
#include "VulkanFramebufferAttachment.h"
#include "VulkanTexture.h"

#include <array>

struct GBufferRenderPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(GBufferRenderPass)

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	void CreatePSO() override;


	std::array<vkutils::Texture2D, GBufferAttachmentIndex::MAX_ATTACHMENTS> attachments;

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