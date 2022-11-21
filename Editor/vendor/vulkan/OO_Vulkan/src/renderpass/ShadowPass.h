/************************************************************************************//*!
\file           ShadowPass.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a shadowpass

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

struct ShadowPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(ShadowPass)

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	bool SetupDependencies() override;
	void CreatePSO() override;
	
	vkutils::Texture2D shadow_depth{};

	// This is for ImGui
	std::array<ImTextureID, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> deferredImg{};
	ImTextureID shadowImg{};

	VkExtent2D shadowmapSize = { 4096, 4096};

	VulkanRenderpass renderpass_Shadow{};

	VkPipeline pso_ShadowDefault{};

private:
	void SetupRenderpass();
	void SetupFramebuffer();
	void CreatePipeline();
};

