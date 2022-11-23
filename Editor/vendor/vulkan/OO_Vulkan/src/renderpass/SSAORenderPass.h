/************************************************************************************//*!
\file           SSAORenderPass.h
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

struct SSAORenderPass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(SSAORenderPass)

	void Init() override;
	void Draw() override;
	void Shutdown() override;

	void InitRandomFactors();

	bool SetupDependencies() override;

	void CreatePSO() override;
	void CreatePipelineLayout();
	void CreateDescriptors();

	std::array<vkutils::Texture2D, GBufferAttachmentIndex::MAX_ATTACHMENTS> attachments{};

	// This is for ImGui
	std::array<ImTextureID, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> deferredImg{};

	VulkanRenderpass renderpass_SSAO{};

	//VkPushConstantRange pushConstantRange;
	VkPipeline pso_SSAO{};
	// TODO: compute i guess
	VkPipeline pso_SSAO_blur{};

	vkutils::Texture2D SSAO_renderTarget;
	vkutils::Texture2D SSAO_finalTarget;
	vkutils::Texture2D randomNoise_texture;

	GpuVector<glm::vec3> randomVectorsSSBO;

private:


	std::vector<glm::vec4> ssaoNoise;
	std::vector<glm::vec3> ssaoKernel;
	void SetupRenderpass();
	void CreatePipeline();

};
