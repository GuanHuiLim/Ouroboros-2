/************************************************************************************//*!
\file           GBufferRenderPass.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a gbuffer pass

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "GfxRenderpass.h"
#include "vulkan/vulkan.h"
#include "imgui/imgui.h"
#include "VulkanTexture.h"

#include <array>

struct GBufferRenderPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(GBufferRenderPass)

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	bool SetupDependencies() override;

	void CreatePSO() override;

	std::array<vkutils::Texture2D, GBufferAttachmentIndex::MAX_ATTACHMENTS> attachments{};

	// This is for ImGui
	std::array<ImTextureID, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> deferredImg{};

	VkRenderPass renderpass_GBuffer{};
	VkFramebuffer framebuffer_GBuffer{};

	//VkPushConstantRange pushConstantRange;
	VkPipeline pso_GBufferDefault{};

private:
	void SetupRenderpass();
	void SetupFramebuffer();
	void CreatePipeline();

};
