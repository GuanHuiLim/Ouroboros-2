#include "NGXWrapper.h"
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
#include "nvsdk_ngx_helpers.h"

#include "VulkanRenderer.h"

#pragma optimize("" ,off)
void NGXWrapper::Init()
{
	VulkanRenderer& vr = *VulkanRenderer::get();

	size_t applicationID = 0;
	const wchar_t* dataPath = L".";
	const char* projectID = "a0f57b54-1daf-4934-90ae-c4035c19df04";
	const char* version = "versionSkillIssue";
	NVSDK_NGX_EngineType et = NVSDK_NGX_EngineType::NVSDK_NGX_ENGINE_TYPE_CUSTOM;
	// this kills the renderdoc
	NVSDK_NGX_Result nvRes = NVSDK_NGX_Result_Fail;
	nvRes = NVSDK_NGX_VULKAN_Init_with_ProjectID(projectID,et, version, dataPath
		, vr.m_instance.instance, vr.m_device.physicalDevice, vr.m_device.logicalDevice
		,vkGetInstanceProcAddr,vkGetDeviceProcAddr);

	m_initialized = NVSDK_NGX_FAILED(nvRes) == false;
	if (NVSDK_NGX_FAILED(nvRes))
	{
		if (nvRes == NVSDK_NGX_Result_FAIL_FeatureNotSupported || nvRes == NVSDK_NGX_Result_FAIL_PlatformError)
			printf("NVIDIA NGX not available on this hardware/platform., code = 0x%08x, info: %ls \n", nvRes, GetNGXResultAsString(nvRes));
		else
			printf("Failed to initialize NGX, error code = 0x%08x, info: %ls \n", nvRes, GetNGXResultAsString(nvRes));
		__debugbreak();
	}

	m_ngxParameters = nullptr;
	nvRes = NVSDK_NGX_Result_Fail;
	nvRes = NVSDK_NGX_VULKAN_GetCapabilityParameters(&m_ngxParameters);
	if (NVSDK_NGX_FAILED(nvRes)) 
	{
		printf("NVSDK_NGX_GetCapabilityParameters failed, code = 0x%08x, info: %ls\n", nvRes, GetNGXResultAsString(nvRes));
		// shutdown ngx
		this->Shutdown();
		__debugbreak();
	}

	NVSDK_NGX_FeatureDiscoveryInfo dis;        
	memset(&dis, 0, sizeof(NVSDK_NGX_FeatureDiscoveryInfo));

	std::wstring ApplicationDataPath = L".";
	dis.SDKVersion                   = NVSDK_NGX_Version_API;
	dis.FeatureID                    = NVSDK_NGX_Feature_SuperSampling;
	dis.Identifier.IdentifierType    = NVSDK_NGX_Application_Identifier_Type_Application_Id;        
	dis.ApplicationDataPath          = ApplicationDataPath.data();
	dis.Identifier.v.ApplicationId   = 0;

	if (!IsFeatureSupported(&dis))
	{
		printf("Requested NGX DLSS Feature not supported\n");
		__debugbreak();
		//return 1;
	}        

#if defined(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver)        \
		    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor) \
		    && defined (NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor)

	// If NGX Successfully initialized then it should set those flags in return
	int needsUpdatedDriver = 0;
	unsigned int minDriverVersionMajor = 0;
	unsigned int minDriverVersionMinor = 0;
	NVSDK_NGX_Result ResultUpdatedDriver = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &needsUpdatedDriver);
	NVSDK_NGX_Result ResultMinDriverVersionMajor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMajor, &minDriverVersionMajor);
	NVSDK_NGX_Result ResultMinDriverVersionMinor = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_MinDriverVersionMinor, &minDriverVersionMinor);
	if (ResultUpdatedDriver == NVSDK_NGX_Result_Success &&
		ResultMinDriverVersionMajor == NVSDK_NGX_Result_Success &&
		ResultMinDriverVersionMinor == NVSDK_NGX_Result_Success)
	{
		if (needsUpdatedDriver)
		{
			printf("NVIDIA DLSS cannot be loaded due to outdated driver. Minimum Driver Version required : %u.%u\n", minDriverVersionMajor, minDriverVersionMinor);
			//shutdown ngx
			__debugbreak();
		}
		else
		{
			printf("NVIDIA DLSS Minimum driver version was reported as : %u.%u\n", minDriverVersionMajor, minDriverVersionMinor);
		}
	}
	else
	{
		printf("NVIDIA DLSS Minimum driver version was not reported.\n");
	}
#endif

	int dlssAvailable = 0;
	NVSDK_NGX_Result ResultDLSS = m_ngxParameters->Get(NVSDK_NGX_Parameter_SuperSampling_Available, &dlssAvailable);
	if (ResultDLSS != NVSDK_NGX_Result_Success || !dlssAvailable)
	{
		// More details about what failed (per feature init result)
		NVSDK_NGX_Result FeatureInitResult = NVSDK_NGX_Result_Fail;
		NVSDK_NGX_Parameter_GetI(m_ngxParameters, NVSDK_NGX_Parameter_SuperSampling_FeatureInitResult, (int*)&FeatureInitResult);
		printf("NVIDIA DLSS not available on this hardward/platform., FeatureInitResult = 0x%08x, info: %ls\n", FeatureInitResult, GetNGXResultAsString(FeatureInitResult));
		// shutdown ngx
		Shutdown();
		__debugbreak();
	}
}

void NGXWrapper::Shutdown()
{
	if (m_initialized == false) return;

	VulkanRenderer& vr = *VulkanRenderer::get();

	if (DLSSisActive())
	{
		ReleaseDLSSFeatures();
	}

	NVSDK_NGX_VULKAN_DestroyParameters(m_ngxParameters);
	NVSDK_NGX_Result shutdownResult = NVSDK_NGX_VULKAN_Shutdown1(vr.m_device.logicalDevice);
	if (NVSDK_NGX_FAILED(shutdownResult))
	{
		printf("NVIDIA DLSS shutdown failed\n");
		__debugbreak();
	}
	m_initialized = false;
}

bool NGXWrapper::DLSSisActive()
{
	return m_dlssFeature != nullptr;
}

bool NGXWrapper::InitializeDLSSFeatures(glm::ivec2 optimalRenderSize, glm::ivec2 displayOutSize, int isContentHDR, bool depthInverted, float depthScale, bool enableSharpening, bool enableAutoExposure, NVSDK_NGX_PerfQuality_Value qualValue, unsigned int renderPreset)
{
	VulkanRenderer& vr = *VulkanRenderer::get();

	if (m_initialized == false)
	{
		printf("Attempt to InitializeDLSSFeature without NGX being initialized.\n");
		return false;
	}

	unsigned int CreationNodeMask = 1;
	unsigned int VisibilityNodeMask = 1;
	NVSDK_NGX_Result ResultDLSS = NVSDK_NGX_Result_Fail;

	int MotionVectorResolutionLow = 1; // we let the Snippet do the upsampling of the motion vector
	// Next create features	
	int DlssCreateFeatureFlags = NVSDK_NGX_DLSS_Feature_Flags_None;
	DlssCreateFeatureFlags |= MotionVectorResolutionLow ? NVSDK_NGX_DLSS_Feature_Flags_MVLowRes : 0;
	DlssCreateFeatureFlags |= isContentHDR ? NVSDK_NGX_DLSS_Feature_Flags_IsHDR : 0;
	DlssCreateFeatureFlags |= depthInverted ? NVSDK_NGX_DLSS_Feature_Flags_DepthInverted : 0;
	DlssCreateFeatureFlags |= enableSharpening ? NVSDK_NGX_DLSS_Feature_Flags_DoSharpening : 0;
	DlssCreateFeatureFlags |= enableAutoExposure ? NVSDK_NGX_DLSS_Feature_Flags_AutoExposure : 0;

	NVSDK_NGX_DLSS_Create_Params DlssCreateParams;

	memset(&DlssCreateParams, 0, sizeof(DlssCreateParams));

	DlssCreateParams.Feature.InWidth = optimalRenderSize.x;
	DlssCreateParams.Feature.InHeight = optimalRenderSize.y;
	DlssCreateParams.Feature.InTargetWidth = displayOutSize.x;
	DlssCreateParams.Feature.InTargetHeight = displayOutSize.y;
	DlssCreateParams.Feature.InPerfQualityValue = qualValue;
	DlssCreateParams.InFeatureCreateFlags = DlssCreateFeatureFlags;

	VkCommandBuffer cmd = vr.GetCommandBuffer();

	ResultDLSS = NGX_VULKAN_CREATE_DLSS_EXT(cmd, CreationNodeMask, VisibilityNodeMask, &m_dlssFeature, m_ngxParameters, &DlssCreateParams);
	if (NVSDK_NGX_FAILED(ResultDLSS))
	{
		printf("Failed to create DLSS Features = 0x%08x, info: %ls\n", ResultDLSS, GetNGXResultAsString(ResultDLSS));
		__debugbreak();
		return false;
	}

	vr.SubmitSingleCommand(cmd);

	return true;
}

void NGXWrapper::ReleaseDLSSFeatures()
{
	OO_ASSERT(m_dlssFeature);

	NVSDK_NGX_Result ResultDLSS = (m_dlssFeature != nullptr) ? NVSDK_NGX_VULKAN_ReleaseFeature(m_dlssFeature) : NVSDK_NGX_Result_Success;
	if (NVSDK_NGX_FAILED(ResultDLSS))
	{
		printf("Failed to NVSDK_NGX_D3D12_ReleaseFeature, code = 0x%08x, info: %ls\n", ResultDLSS, GetNGXResultAsString(ResultDLSS));
	}
	m_dlssFeature = nullptr;
}

NVSDK_NGX_Resource_VK TextureToResourceVK(vkutils::Texture*  tex, VkImageSubresourceRange subresources)
{
	
	NVSDK_NGX_Resource_VK resourceVK = {};
	VkImageView imageView = tex->view;
	VkFormat format = tex->format;
	VkImage image = tex->image.image;
	VkImageSubresourceRange subresourceRange = { 1, subresources.baseMipLevel, subresources.levelCount, subresources.baseArrayLayer, subresources.layerCount };

	return NVSDK_NGX_Create_ImageView_Resource_VK(imageView, image, subresourceRange, format, tex->width, tex->height, tex->usage & VK_IMAGE_USAGE_STORAGE_BIT);
}

void NGXWrapper::EvaluateSuperSampling(VkCommandBuffer commandList
	, vkutils::Texture* unresolvedColor
	, vkutils::Texture* resolvedColor
	, vkutils::Texture* motionVectors
	, vkutils::Texture* depth
	, vkutils::Texture* exposure
	, VkViewport viewport
	, bool bResetAccumulation, bool bUseNgxSdkExtApi
	, glm::vec2 jitterOffset, glm::vec2 mVScale)
{
	OO_ASSERT(m_initialized);
	OO_ASSERT(m_dlssFeature);

	VulkanRenderer& vr = *VulkanRenderer::get();

	NVSDK_NGX_Coordinates renderingOffset = { (unsigned int)0,
											  (unsigned int)0 };
	NVSDK_NGX_Dimensions  renderingSize = { (unsigned int)(viewport.width  - viewport.x),
											(unsigned int)(viewport.height - viewport.y)};

	VkImageSubresourceRange subresources{ VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1 };
	NVSDK_NGX_Resource_VK unresolvedColorResource = TextureToResourceVK(unresolvedColor, subresources);
	NVSDK_NGX_Resource_VK resolvedColorResource = TextureToResourceVK(resolvedColor, subresources);
	NVSDK_NGX_Resource_VK motionVectorsResource = TextureToResourceVK(motionVectors, subresources);
	NVSDK_NGX_Resource_VK depthResource = TextureToResourceVK(depth, subresources);
	NVSDK_NGX_Resource_VK exposureResource = TextureToResourceVK(exposure, subresources); // might need to change

	NVSDK_NGX_Result Result;

	NVSDK_NGX_VK_DLSS_Eval_Params VkDlssEvalParams{};
	memset(&VkDlssEvalParams, 0, sizeof(VkDlssEvalParams));

	VkDlssEvalParams.Feature.pInColor = &unresolvedColorResource;
	VkDlssEvalParams.Feature.pInOutput = &resolvedColorResource;
	VkDlssEvalParams.pInDepth = &depthResource;
	VkDlssEvalParams.pInMotionVectors = &motionVectorsResource;
	VkDlssEvalParams.pInExposureTexture = &exposureResource;
	VkDlssEvalParams.InJitterOffsetX = jitterOffset.x * -1.0f;
	VkDlssEvalParams.InJitterOffsetY = jitterOffset.y * -1.0f;
	VkDlssEvalParams.InReset = bResetAccumulation;
	VkDlssEvalParams.InMVScaleX = mVScale.x;
	VkDlssEvalParams.InMVScaleY = mVScale.y;
	VkDlssEvalParams.InColorSubrectBase = renderingOffset;
	VkDlssEvalParams.InDepthSubrectBase = renderingOffset;
	VkDlssEvalParams.InTranslucencySubrectBase = renderingOffset;
	VkDlssEvalParams.InMVSubrectBase = renderingOffset;
	VkDlssEvalParams.InRenderSubrectDimensions = renderingSize;

	Result = NGX_VULKAN_EVALUATE_DLSS_EXT(commandList, m_dlssFeature, m_ngxParameters, &VkDlssEvalParams);

	if (NVSDK_NGX_FAILED(Result))
	{
		printf("Failed to NVSDK_NGX_VULKAN_EvaluateFeature for DLSS, code = 0x%08x, info: %ls\n", Result, GetNGXResultAsString(Result));
	}


}

bool NGXWrapper::QueryOptimalSettings(glm::uvec2 inDisplaySize, NVSDK_NGX_PerfQuality_Value inQualValue, DlssRecommendedSettings* outRecommendedSettings)
{
	if (!m_initialized)
	{
		outRecommendedSettings->m_ngxRecommendedOptimalRenderSize = inDisplaySize;
		outRecommendedSettings->m_ngxDynamicMaximumRenderSize     = inDisplaySize;
		outRecommendedSettings->m_ngxDynamicMinimumRenderSize     = inDisplaySize;

		printf("NGX was not initialized when querying Optimal Settings\n");
		return false;
	}

	NVSDK_NGX_Result Result = NGX_DLSS_GET_OPTIMAL_SETTINGS(m_ngxParameters,
		inDisplaySize.x, inDisplaySize.y, inQualValue,
		&outRecommendedSettings->m_ngxRecommendedOptimalRenderSize.x, &outRecommendedSettings->m_ngxRecommendedOptimalRenderSize.y,
		&outRecommendedSettings->m_ngxDynamicMaximumRenderSize.x, &outRecommendedSettings->m_ngxDynamicMaximumRenderSize.y,
		&outRecommendedSettings->m_ngxDynamicMinimumRenderSize.x, &outRecommendedSettings->m_ngxDynamicMinimumRenderSize.y,
		&outRecommendedSettings->m_ngxRecommendedSharpness);

	if (NVSDK_NGX_FAILED(Result))
	{
		outRecommendedSettings->m_ngxRecommendedOptimalRenderSize   = inDisplaySize;
		outRecommendedSettings->m_ngxDynamicMaximumRenderSize       = inDisplaySize;
		outRecommendedSettings->m_ngxDynamicMinimumRenderSize       = inDisplaySize;
		outRecommendedSettings->m_ngxRecommendedSharpness           = 0.0f;

		printf("Querying Optimal Settings failed! code = 0x%08x, info: %ls\n", Result, GetNGXResultAsString(Result));
		return false;
	}


}

bool NGXWrapper::IsFeatureSupported(NVSDK_NGX_FeatureDiscoveryInfo* dis)
{
	VulkanRenderer& vr = *VulkanRenderer::get();

	NVSDK_NGX_Result             res;
	NVSDK_NGX_FeatureRequirement req;

	uint32_t nDeviceExtensions;
	VkExtensionProperties* deviceExtensions;
	res = NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements(vr.m_instance.instance, vr.m_device.physicalDevice, dis, &nDeviceExtensions, &deviceExtensions);
	if (res != NVSDK_NGX_Result_Success)
	{
		printf("GetFeatureRequirements error: NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements returned 0x%08x info: %ls\n", res, GetNGXResultAsString(res));
		return false;
	}
	else
	{
		printf("NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements returned %d device extension requirements: ", nDeviceExtensions);
		for (uint32_t i = 0; i < nDeviceExtensions; i++)
		{
			printf("%s, ", deviceExtensions[i].extensionName);
		}
		printf("\n");
	}

	uint32_t nInstanceExtensions;
	VkExtensionProperties* instanceExtensions;
	res = NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements(dis, &nInstanceExtensions, &instanceExtensions);
	if (res != NVSDK_NGX_Result_Success)
	{
		printf("GetFeatureRequirements error: NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements returned 0x%08x info: %ls\n", res, GetNGXResultAsString(res));
		return 1;
	}
	else
	{
		printf("NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements returned %d instance extension requirements: ", nInstanceExtensions);
		for (uint32_t i = 0; i < nInstanceExtensions; i++)
		{
			printf("%s, ", instanceExtensions[i].extensionName);
		}
		printf("\n");
	}

	res = NVSDK_NGX_VULKAN_GetFeatureRequirements(vr.m_instance.instance, vr.m_device.physicalDevice, dis, &req);

	if (res != NVSDK_NGX_Result_Success && res != NVSDK_NGX_Result_FAIL_NotImplemented)
	{
		printf("GetFeatureRequirements error: 0x%08x info: %ls\n", res, GetNGXResultAsString(res));
		return false;
	}

	printf("GetFeatureRequirements returned 0x%08x: Min GPU Arch: 0x%08x MinOS: %s\n", req.FeatureSupported, req.MinHWArchitecture, req.MinOSVersion);

	return true;
}
