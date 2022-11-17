/************************************************************************************//*!
\file           GbufferParticlePass.h
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

struct GbufferParticlePass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(GbufferParticlePass)

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	bool SetupDependencies() override;

	void CreatePSO() override;

	VkRenderPass renderpass_GbufferSecondsPass{};
	VkFramebuffer framebuffer_GBufferSecondPass{};

	//VkPushConstantRange pushConstantRange;
	VkPipeline pso_GBufferParticles{};
	
private:
	void SetupRenderpass();
	void SetupFramebuffer();
	void CreatePipeline();

};
