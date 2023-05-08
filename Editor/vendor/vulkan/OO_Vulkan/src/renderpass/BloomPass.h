/************************************************************************************//*!
\file           BloomPass.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Nov 8, 2022
\brief              Declares a SSAO pass

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
#include "GpuVector.h"

#include <array>

struct BloomPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(BloomPass)

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	bool SetupDependencies() override;

	void CreatePSO() override;
	void CreatePipelineLayout();
	void CreateDescriptors();

	VulkanRenderpass renderpass_bright{};
	VulkanRenderpass renderpass_bloomDownsample{};
	VulkanRenderpass renderpass_bloomUpsample{};

	//VkPushConstantRange pushConstantRange;
	VkPipeline pso_bloom_bright{};
	VkPipeline pso_bloom_down{};
	VkPipeline pso_bloom_up{};
	VkPipeline pso_additive_composite{};
	VkPipeline pso_tone_mapping{};
	VkPipeline pso_vignette{};
	VkPipeline pso_fxaa{};

	static constexpr size_t MAX_BLOOM_SAMPLES = 5;
	vkutils::Texture2D Bloom_brightTarget;

	// TODO: compute i guess
	std::array<vkutils::Texture2D, MAX_BLOOM_SAMPLES> Bloom_downsampleTargets;
	std::array<vkutils::Texture2D, MAX_BLOOM_SAMPLES> Bloom_upsampleTargets;


private:

	void SetupRenderpass();
	void CreatePipeline();

};
