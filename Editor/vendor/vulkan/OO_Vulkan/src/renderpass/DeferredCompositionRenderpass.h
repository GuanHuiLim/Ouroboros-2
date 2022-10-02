/************************************************************************************//*!
\file           DeferredCompositionRenderpass.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a deferred lighting composition pass

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "GfxRenderpass.h"
#include "vulkan/vulkan.h"

#include <memory>

struct DeferredCompositionRenderpass : public GfxRenderpass
{
	//DECLARE_RENDERPASS_SINGLETON(DeferredCompositionRenderpass)
	
	void Init() override;
	void Draw() override;
	void Shutdown() override;

	bool SetupDependencies() override;
	void CreatePSO() override;

	VkRenderPass renderpass_DeferredLightingComposition{};
	
	VkPipeline pso_DeferredLightingComposition{};

	uint64_t uboDynamicAlignment{};

	void CreatePipeline();
private:
	void CreateDescriptors();
	void CreatePipelineLayout();
	bool m_log{ false };
};

