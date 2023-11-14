#pragma once

#include "MathCommon.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>



// nvidia DLSS
#include "nvsdk_ngx_defs.h"
#include "nvsdk_ngx_vk.h"
#include "nvsdk_ngx_params.h"
#include "nvsdk_ngx_helpers_vk.h"

namespace vkutils {
class Texture;
}

struct NVSDK_NGX_FeatureDiscoveryInfo;
struct NVSDK_NGX_Parameter;

struct DlssRecommendedSettings
{
	float      m_ngxRecommendedSharpness         = 0.01f; // in ngx sdk 3.1, dlss sharpening is deprecated
	glm::uvec2 m_ngxRecommendedOptimalRenderSize = {~(0u), ~(0u)};
	glm::uvec2 m_ngxDynamicMaximumRenderSize     = {~(0u), ~(0u)};
	glm::uvec2 m_ngxDynamicMinimumRenderSize     = {~(0u), ~(0u)};
};

class NGXWrapper
{
public:
	NVSDK_NGX_Parameter* m_ngxParameters{};
	bool m_initialized{ false };
	NVSDK_NGX_Handle* m_dlssFeature{ nullptr };

	void Init();
	void Shutdown();

	void OnResize();

	bool DLSSisActive();
	bool InitializeDLSSFeatures(glm::ivec2 optimalRenderSize, glm::ivec2 displayOutSize
		, int isContentHDR, bool depthInverted
		, float depthScale = 1.0f, bool enableSharpening = false
		, bool enableAutoExposure = false, NVSDK_NGX_PerfQuality_Value qualValue = NVSDK_NGX_PerfQuality_Value_MaxPerf
		, unsigned int renderPreset = 0);
	void ReleaseDLSSFeatures();

	void EvaluateSuperSampling(
		VkCommandBuffer commandList,
		vkutils::Texture* unresolvedColor,
		vkutils::Texture* resolvedColor,
		vkutils::Texture* motionVectors,
		vkutils::Texture* depth,
		vkutils::Texture* exposure,
		VkViewport viewport,
		bool bResetAccumulation = false,
		bool bUseNgxSdkExtApi = false,
		glm::vec2 jitterOffset = { 0.0f, 0.0f },
		glm::vec2 mVScale = { 1.0f, 1.0f });

	bool QueryOptimalSettings(glm::uvec2 inDisplaySize
		, NVSDK_NGX_PerfQuality_Value inQualValue
		, DlssRecommendedSettings *outRecommendedSettings);

	bool IsFeatureSupported(NVSDK_NGX_FeatureDiscoveryInfo* dis);
	

};

