/************************************************************************************//*!
\file           VulkanRenderer.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares the vulkan renderer class. 
The entire class encapsulates the vulkan renderer and acts as an interface for external engines

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "MeshModel.h"

#include "GfxTypes.h"
#include "VulkanUtils.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanRenderpass.h"
#include "GpuVector.h"
#include "gpuCommon.h"
#include "DescriptorBuilder.h"
#include "DescriptorAllocator.h"
#include "DescriptorLayoutCache.h"
#include "FramebufferCache.h"
#include "Geometry.h"
#include "Collision.h"

#include "TaskManager.h"

#include "Camera.h"

#include "imgui/imgui.h"

#include "GfxSampler.h"
#include "GfxRenderpass.h"

#include "GraphicsWorld.h"
#include "GraphicsBatch.h"

#include "TexturePacker.h"
#include "Font.h"

#include "TaskManager.h"

#include <vector>
#include <array>
#include <set>
#include <string>
#include <mutex>
#include <deque>
#include <functional>
#include <memory>

// dlss
#include "NGXWrapper.h"

struct Window;


int Win32SurfaceCreator(ImGuiViewport* vp, ImU64 device, const void* allocator, ImU64* outSurface);

enum FSR2 : uint8_t {
	TCR_AUTOGEN,
	AUTOGEN_REACTIVE,
	COMPUTE_LUMINANCE_PYRAMID,
	RECONSTRUCT_PREVIOUS_DEPTH,
	DEPTH_CLIP,
	LOCK,
	ACCUMULATE,
	RCAS,
	MAX_SIZE
};

enum class UPSCALING_TYPE : uint8_t
{
	NONE,
	DLSS,
	FSR2,
};
ENUM_OPERATORS_GEN(UPSCALING_TYPE, uint8_t);
enum class UPSCALING_QUALITY : uint8_t
{
	NATIVE,
	QUALITY,
	BALANCED,
	PERFORMANCE,
	ULTRA_PERFORMANCE,
	CUSTOM,
	NONE,
};
ENUM_OPERATORS_GEN(UPSCALING_QUALITY, uint8_t);

// Moving all the Descriptor Set Layout out of the VulkanRenderer class abomination...
struct SetLayoutDB // Think of a better name? Very short and sweet for easy typing productivity?
{
    // For GPU Scene
    inline static VkDescriptorSetLayout gpuscene{VK_NULL_HANDLE};
    // For UBO with the corresponding swap chain image
    inline static VkDescriptorSetLayout FrameUniform{VK_NULL_HANDLE};
    // For unbounded array of texture descriptors, used in bindless approach
    inline static VkDescriptorSetLayout bindless{VK_NULL_HANDLE};
	// For lighting
	inline static VkDescriptorSetLayout Lighting{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout skypass{VK_NULL_HANDLE};

	inline static VkDescriptorSetLayout imguiCB{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout imguiTexture{VK_NULL_HANDLE};

	inline static VkDescriptorSetLayout lights{VK_NULL_HANDLE};
	// 
	inline static VkDescriptorSetLayout ForwardDecal{VK_NULL_HANDLE};

	inline static VkDescriptorSetLayout SSAO{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout SSAOBlur{VK_NULL_HANDLE};

	inline static VkDescriptorSetLayout util_fullscreenBlit{VK_NULL_HANDLE};

	inline static VkDescriptorSetLayout compute_singleTexture{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_doubleImageStore{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_shadowPrepass{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_singleSSBO{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_AMDSPD{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_Radiance{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_prefilter{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_brdfLUT{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_histogram{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_luminance{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_brightPixels{VK_NULL_HANDLE};
	inline static VkDescriptorSetLayout compute_tonemap{VK_NULL_HANDLE};

	// FSR2
	inline static VkDescriptorSetLayout compute_fsr2[FSR2::MAX_SIZE]{};

};

struct Attachments_imguiBinding {
	inline static std::array<ImTextureID, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> deferredImg{};

	inline static ImTextureID shadowImg{};
};


// Moving all the Descriptor Set Layout out of the VulkanRenderer class abomination...
struct PSOLayoutDB
{
	inline static VkPipelineLayout defaultPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout imguiPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout fullscreenBlitPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout lightingPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout forwardDecalPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout SSAOPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout SSAOBlurPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout BloomPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout tonemapPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout doubleImageStoreLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout brightPixelsLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout singleSSBOlayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout shadowPrepassPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout AMDSPDPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout RadiancePSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout prefilterPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout BRDFLUTPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout skypassPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout histogramPSOLayout{ VK_NULL_HANDLE };
	inline static VkPipelineLayout luminancePSOLayout{ VK_NULL_HANDLE };

	// FSR2
	inline static VkPipelineLayout fsr2_PSOLayouts[FSR2::MAX_SIZE]{};
};

// Moving all constant buffer structures into this CB namespace.
// Important: Take extra care of the alignment and memory layout. Must match the shader side.
namespace CB
{
	struct FrameContextUBO
	{
		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 viewProjection{ 1.0f };
		glm::mat4 inverseViewProjection{ 1.0f };
		glm::mat4 inverseView{ 1.0f };
		glm::mat4 inverseProjection{ 1.0f };
		glm::vec4 cameraPosition{ 1.0f };
		glm::mat4 prevViewProjection{ 1.0f };
		glm::vec4 renderTimer{ 0.0f, 0.0f, 0.0f, 0.0f };

		glm::mat4 projectionJittered;
		glm::mat4 viewProjJittered;
		glm::mat4 inverseProjectionJittered;
		glm::mat4 prevViewProjJittered;
		glm::vec2 currJitter;
		glm::vec2 prevJitter;

		// These variables area only to speedup development time by passing adjustable values from the C++ side to the shader.
		// Bind this to every single shader possible.
		// Remove this upon shipping the final product.
		glm::vec4 vector4_values0{};
		glm::vec4 vector4_values1{};
		glm::vec4 vector4_values2{};
		glm::vec4 vector4_values3{};
		glm::vec4 vector4_values4{};
		glm::vec4 vector4_values5{};
		glm::vec4 vector4_values6{};
		glm::vec4 vector4_values7{};
		glm::vec4 vector4_values8{};
		glm::vec4 vector4_values9{};
	};

	struct AMDSPD_UBO
	{
		uint32_t	mips;
		uint32_t	numWorkGroups;
		uint32_t	workGroupOffset[2];
		glm::vec2	invInputSize; 
		glm::vec2	padding;
	};

	struct AMDSPD_ATOMIC {
		uint32_t counter[6];
	};

}

class VulkanRenderer
{
public:

	static VulkanRenderer* s_vulkanRenderer;
	static constexpr int MAX_FRAME_DRAWS = 2;

	struct Attachments {
		std::array<vkutils::Texture2D, GBufferAttachmentIndex::MAX_ATTACHMENTS> gbuffer{};
		vkutils::Texture2D shadowMask{};

		vkutils::Texture2D SSAO_renderTarget{};
		vkutils::Texture2D SSAO_finalTarget{};
		vkutils::Texture2D randomNoise_texture{};

		vkutils::Texture2D shadow_depth{};

		vkutils::Texture2D lighting_target{};

		vkutils::Texture2D fullres_HDR{};

		static constexpr size_t MAX_BLOOM_SAMPLES = 5;
		vkutils::Texture2D Bloom_brightTarget;
		vkutils::Texture2D SD_target[2];
		std::array<vkutils::Texture2D, MAX_BLOOM_SAMPLES> Bloom_downsampleTargets;

		//FSR2 
		vkutils::Texture2D fsr_exposure_mips;
		vkutils::Texture2D fsr_reconstructed_prev_depth;
		vkutils::Texture2D fsr_dilated_depth;
		vkutils::Texture2D fsr_dilated_velocity[MAX_FRAME_DRAWS];
		vkutils::Texture2D fsr_lock_input_luma;
		vkutils::Texture2D fsr_dilated_reactive_masks;
		vkutils::Texture2D fsr_prepared_input_color;
		vkutils::Texture2D fsr_reactive_mask;
		vkutils::Texture2D fsr_new_locks;
		vkutils::Texture2D fsr_lock_status[MAX_FRAME_DRAWS];
		vkutils::Texture2D fsr_upscaled_color[MAX_FRAME_DRAWS];
		vkutils::Texture2D fsr_luma_history[MAX_FRAME_DRAWS];
	}attachments;

	inline static uint64_t totalTextureSizeLoaded = 0;

	static constexpr int MAX_OBJECTS = 2048;
	static constexpr VkFormat G_DEPTH_FORMAT = VK_FORMAT_D24_UNORM_S8_UINT;
	static constexpr VkFormat G_NORMALS_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;
	static constexpr VkCompareOp G_DEPTH_COMPARISON = VK_COMPARE_OP_GREATER_OR_EQUAL;
	static constexpr VkFormat G_HDR_FORMAT_ALPHA = VK_FORMAT_R16G16B16A16_SFLOAT;
	static constexpr VkFormat G_HDR_FORMAT = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	static constexpr VkFormat G_VELOCITY_FORMAT = VK_FORMAT_R16G16_SFLOAT;
	static constexpr VkFormat G_NON_HDR_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

	static int ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface);

#define OBJECT_INSTANCE_COUNT 128

	 PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName{ nullptr };
	 PFN_vkCmdDebugMarkerBeginEXT pfnDebugMarkerRegionBegin{ nullptr };
	 PFN_vkCmdDebugMarkerEndEXT pfnDebugMarkerRegionEnd{ nullptr };

	~VulkanRenderer();

	static VulkanRenderer* get();

	bool Init(const oGFX::SetupInfo& setupSpecs, Window& window);
	
	void ReloadShaders();

	void CreateInstance(const oGFX::SetupInfo& setupSpecs);
	void CreateDebugCallback();
	void DestroyDebugMessenger();
	void CreateSurface(const oGFX::SetupInfo& setupSpecs, Window& window);
	void AcquirePhysicalDevice(const oGFX::SetupInfo& setupSpecs);
	void CreateLogicalDevice(const oGFX::SetupInfo& setupSpecs);
	void InitVMA(const oGFX::SetupInfo& setupSpecs);
	void SetupSwapchain();
	void CreateDefaultRenderpass();
	void CreateDefaultDescriptorSetLayout();

	void FullscreenBlit(VkCommandBuffer cmd, vkutils::Texture& src,VkImageLayout srcFinal, vkutils::Texture& dst,VkImageLayout dstFinal);
	void BlitFramebuffer(VkCommandBuffer cmd, vkutils::Texture& src,VkImageLayout srcFinal, vkutils::Texture& dst,VkImageLayout dstFinal);
	VkCommandBuffer m_finalBlitCmd{ VK_NULL_HANDLE };

	void CreateDefaultPSOLayouts();
	void CreateDefaultPSO();
	//void CreateDepthBufferImage();
	void CreateFramebuffers(); 
	void CreateCommandBuffers();


	VkCommandBuffer GetCommandBuffer();
	void SubmitSingleCommandAndWait(VkCommandBuffer cmd);
	void SubmitSingleCommand(VkCommandBuffer cmd);
	void QueueCommandBuffer(VkCommandBuffer cmd);
	std::vector<VkCommandBuffer>sequencedBuffers;
	std::queue<Task>m_taskList;
	std::vector<Task>m_sequentialTasks;
	TaskCompletionCallback drawCallRecrodingCompleted{ Task([](void*) {}) };
	void AddRenderer(GfxRenderpass* pass);
	
	ImTextureID myImg{};

	bool useSSAO = true;
    bool m_imguiInitialized = false;
	bool m_initialized = false;

	//---------- Device ----------

	VulkanInstance m_instance{};
	VulkanDevice m_device{};
	VulkanSwapchain m_swapchain{};
	std::vector<VkFramebuffer> swapChainFramebuffers;
	uint32_t swapchainIdx{ 0 };
	VkDebugUtilsMessengerEXT m_debugMessenger{};

	//---------- DescriptorSet ----------

	// For Deferred Lighting onwards
	VkDescriptorSet descriptorSet_DeferredComposition{VK_NULL_HANDLE};
	// For unbounded array of texture descriptors, used in bindless approach
	VkDescriptorSet descriptorSet_bindless{VK_NULL_HANDLE};
	// For GPU Scene
	VkDescriptorSet descriptorSet_gpuscene{VK_NULL_HANDLE};

	VkDescriptorSet descriptorSet_lights{VK_NULL_HANDLE};

	VkDescriptorSet descriptorSet_bones{VK_NULL_HANDLE};

	VkDescriptorSet descriptorSet_objInfos{VK_NULL_HANDLE};

	VkDescriptorSet descriptorSet_SSAO{VK_NULL_HANDLE};
	VkDescriptorSet descriptorSet_SSAOBlur{VK_NULL_HANDLE};

	VkDescriptorSet descriptorSet_fullscreenBlit{VK_NULL_HANDLE};
	// For UBO with the corresponding swap chain image
	std::vector<VkDescriptorSet> descriptorSets_uniform;

	void SetWorld(GraphicsWorld* world);
	void InitWorld(GraphicsWorld* world);
	void DestroyWorld(GraphicsWorld* world);
	GraphicsWorld* currWorld{ nullptr };
	uint32_t renderIteration{ 0};
	int32_t m_numShadowcastLights{0};
	uint32_t renderTargetInUseID{ 0 };
	float renderClock{ 0.0f };
	float deltaTime{ 0.0016f };

	int32_t GetPixelValue(uint32_t fbID, glm::vec2 uv);

	GraphicsBatch batches;

	bool deferredRendering = true;

	void CreateLightingBuffers();
	void UploadLights();
	void UploadBones();

	void CreateSynchronisation();
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets_GPUScene();
	void CreateDescriptorSets_Lights();

	struct ImGUIStructures
	{
		VkDescriptorPool descriptorPools{};
		VkRenderPass renderPass{};
		std::vector<VkFramebuffer> buffers;
	}m_imguiConfig{};

	void InitImGUI();
	void ResizeGUIBuffers();
	void DebugGUIcalls();
	void DestroyImGUI();
	void RestartImgui();
	void PerformImguiRestart();
	void ImguiSoftDestroy();
	ImDrawData m_imguiDrawData;
	std::vector<ImDrawList*> m_imguiDrawList;
	void SubmitImguiDrawList(ImDrawData* drawData);
	void InvalidateDrawLists();

	std::mutex m_imguiShutdownGuard;

	void InitializeRenderBuffers();
	void DestroyRenderBuffers();
	void GenerateCPUIndirectDrawCommands();
	void UploadInstanceData();
	void UploadUIData();
	uint32_t commandCount{};
	// Contains the instanced data
	GpuVector<oGFX::InstanceData> instanceBuffer;
	GpuVector<oGFX::InstanceData> shadowCasterInstanceBuffer;

	bool PrepareFrame();
	void BeginDraw();
	void RenderFrame();
	void RenderFunc(bool shouldRunDebugDraw);
	void Present();

	void UpdateUniformBuffers();

	// Immediate command sending helper
	VkCommandBuffer beginSingleTimeCommands();
	// Immediate command sending helper
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	uint32_t CreateTexture(uint32_t width, uint32_t height, unsigned char* imgData, bool generateMips = true);
	uint32_t CreateTexture(const std::string& fileName);
	uint32_t CreateCubeMapTexture(const std::string& folder);

	bool ReloadTexture(uint32_t textureID, const std::string& file);
	void UnloadTexture(uint32_t textureID);
	void GenerateMipmaps(vkutils::Texture& texture);
	void GenerateRadianceMap(VkCommandBuffer cmdlist , vkutils::CubeTexture& texture);
	void GeneratePrefilterMap(VkCommandBuffer cmdlist , vkutils::CubeTexture& texture);
	void GenerateBRDFLUT(VkCommandBuffer cmdlist , vkutils::Texture2D& texture);

	oGFX::Font* LoadFont(const std::string& filename);
	oGFX::TexturePacker CreateFontAtlas(const std::string& filename, oGFX::Font& font);

	struct TextureInfo
	{
		std::string name;
		uint32_t width;
		uint32_t height;
		VkFormat format;
		uint32_t mips;
	};
	TextureInfo GetTextureInfo(uint32_t handle);

	void InitDebugBuffers();
	bool UploadDebugDrawBuffers();

	// This naming is rather confusing... VertexBufferObject but it contains an index buffer inside?
	struct IndexedVertexBuffer
	{
		GpuVector<oGFX::Vertex> VtxBuffer;
		GpuVector<uint32_t> IdxBuffer;
		uint32_t VtxOffset{};
		uint32_t IdxOffset{};
	};

	std::mutex g_mut_globalMeshBuffers;
	IndexedVertexBuffer g_GlobalMeshBuffers;

	GpuVector<ParticleData> g_particleDatas;
	GpuVector<oGFX::IndirectCommand> g_particleCommandsBuffer;

	GpuVector<oGFX::DebugVertex>g_DebugDrawVertexBufferGPU;
	GpuVector<uint32_t> g_DebugDrawIndexBufferGPU;
	std::vector<oGFX::DebugVertex> g_DebugDrawVertexBufferCPU;
	std::vector<uint32_t> g_DebugDrawIndexBufferCPU;

	// ui pass
	GpuVector<oGFX::UIVertex> g_UIVertexBufferGPU;
	GpuVector<uint32_t> g_UIIndexBufferGPU;
	std::array<GpuVector<UIData>,3> g_UIDatas;

	ModelFileResource* GetDefaultCube();
	oGFX::Font* GetDefaultFont();

	ModelFileResource* LoadModelFromFile(const std::string& file);
	ModelFileResource* LoadMeshFromBuffers(std::vector<oGFX::Vertex>& vertex, std::vector<uint32_t>& indices, gfxModel* model);
	void LoadSubmesh(gfxModel& mdl, SubMesh& submesh, aiMesh* aimesh, ModelFileResource* modelFile);
	void LoadBoneInformation(ModelFileResource& fileData, oGFX::Skeleton& skeleton, aiMesh& aimesh, std::vector<BoneWeight>& boneWeights, uint32_t& vCnt);
	void BuildSkeletonRecursive(ModelFileResource& fileData, aiNode* ainode, oGFX::BoneNode* node, glm::mat4 parentXform = glm::mat4(1.0f), std::string prefix = std::string("\t"));
	const oGFX::Skeleton* GetSkeleton(uint32_t modelID);
	oGFX::CPUSkeletonInstance* CreateSkeletonInstance(uint32_t modelID);

	bool ResizeSwapchain();

	void UpdateRenderResolution();
	void SetUpscaler(UPSCALING_TYPE upscaler);
	void SetQuality(UPSCALING_QUALITY quality);
	float changedRenderResolution = 1.0f;
	float renderResolution = 1.0f;
	uint32_t renderWidth{};
	uint32_t renderHeight{};
	uint32_t m_JitterIndex = 0;
	bool m_useJitter = true;
	float prevjitterX;
	float prevjitterY;
	float jitterX;
	float jitterY;
	uint32_t fsrFrameCount;
	int32_t jitterPhaseCount;
	
	UPSCALING_TYPE m_upscaleType = UPSCALING_TYPE::NONE;
	UPSCALING_QUALITY m_upscaleQuality = UPSCALING_QUALITY::NATIVE;

	struct PERF_QUALITY_ITEM
	{
		NVSDK_NGX_PerfQuality_Value PerfQuality;
		const char* PerfQualityText;
		bool                        PerfQualityAllowed;
		bool                        PerfQualityDynamicAllowed;
	};
	
	std::vector<PERF_QUALITY_ITEM>        PERF_QUALITY_LIST =
	{
		{NVSDK_NGX_PerfQuality_Value_DLAA,             "DLAA"       , false, false}, // basically native
		{NVSDK_NGX_PerfQuality_Value_MaxQuality,       "Quality"    , false, false},
		{NVSDK_NGX_PerfQuality_Value_Balanced,         "Balanced"   , false, false},
		{NVSDK_NGX_PerfQuality_Value_MaxPerf,          "Performance", false, false},
		{NVSDK_NGX_PerfQuality_Value_UltraPerformance, "UltraPerf"  , false, false},
	};
	std::unordered_map<NVSDK_NGX_PerfQuality_Value, DlssRecommendedSettings> m_RecommendedSettingsMap;
	glm::ivec2 m_recommendedSettingsLastSize = {~0, ~0, }; // in case we need to refresh the recommended settings map
	NGXWrapper m_NGX;
	void FillReccomendedSettings(glm::ivec2 displaySize);
	void PrepareDLSS();

	float rcas_sharpness = 1.0f;

	Window* windowPtr{ nullptr };

	//textures
	std::mutex g_mut_Textures;
	std::vector<vkutils::Texture2D> g_Textures;

	vkutils::CubeTexture g_cubeMap;
	vkutils::CubeTexture g_radianceMap;
	vkutils::CubeTexture g_prefilterMap;
	vkutils::Texture2D g_brdfLUT;

	std::vector<ImTextureID> g_imguiIDs;
	std::mutex g_mute_imguiTextureMap;
	std::unordered_map<ImTextureID, vkutils::Texture*>g_imguiToTexture;
	vkutils::Texture2D g_imguiFont;

	uint32_t whiteTextureID = static_cast<uint32_t>(-1);
	uint32_t blackTextureID = static_cast<uint32_t>(-1);
	uint32_t normalTextureID = static_cast<uint32_t>(-1);
	uint32_t pinkTextureID = static_cast<uint32_t>(-1);

	uint32_t GetDefaultCubeID();
	uint32_t GetDefaultPlaneID();
	uint32_t GetDefaultSpriteID();

	// - Synchronisation
	std::vector<VkSemaphore> presentSemaphore;
	std::vector<VkSemaphore> renderSemaphore;
	std::vector<VkFence> drawFences;
	VkSemaphore frameCountSemaphore{VK_NULL_HANDLE};

	// - Pipeline
	VkPipeline pso_utilFullscreenBlit{ VK_NULL_HANDLE };
	VkPipeline pso_utilAMDSPD{ VK_NULL_HANDLE };
	VkPipeline pso_radiance{ VK_NULL_HANDLE };
	VkPipeline pso_prefilter{ VK_NULL_HANDLE };
	VkPipeline pso_brdfLUT{ VK_NULL_HANDLE };

	VulkanRenderpass renderPass_default{};
	VulkanRenderpass renderPass_default_noDepth{};
	VulkanRenderpass renderPass_HDR{};
	VulkanRenderpass renderPass_HDR_noDepth{};
	VulkanRenderpass renderPass_blit{};

	GpuVector<oGFX::IndirectCommand> indirectCommandsBuffer;
	GpuVector<oGFX::IndirectCommand> shadowCasterCommandsBuffer;
	uint32_t indirectDrawCount{};

	GpuVector<LocalLightInstance> globalLightBuffer;

	// - Descriptors

	VkDescriptorPool descriptorPool{};
	VkDescriptorPool samplerDescriptorPool{};

	//std::vector<VkDescriptorSet> samplerDescriptorSets;
	uint32_t bindlessGlobalTexturesNextIndex = 0;

	// SSBO
	std::vector<glm::mat4> boneMatrices{};
	GpuVector<glm::mat4> gpuBoneMatrixBuffer;

	std::vector<BoneWeight> g_skinningBoneWeights;
	GpuVector<BoneWeight> gpuSkinningWeightsBuffer;

	// SSBO
	std::vector<GPUTransform> gpuTransform{};
	GpuVector<GPUTransform> gpuTransformBuffer;
	
	// SSBO
	std::vector<GPUTransform> gpuShadowCasterTransform{};
	GpuVector<GPUTransform> gpuShadowCasterTransformBuffer;

	// SSBO
	std::vector<GPUObjectInformation> objectInformation;
	GpuVector<GPUObjectInformation> objectInformationBuffer;
	GpuVector<GPUObjectInformation> casterObjectInformationBuffer;
	
	// SSBO
	std::vector<oGFX::AllocatedBuffer> vpUniformBuffer{};
	oGFX::AllocatedBuffer SPDatomicBuffer;
	oGFX::AllocatedBuffer SPDconstantBuffer;

	oGFX::AllocatedBuffer FSR2constantBuffer[MAX_FRAME_DRAWS];
	oGFX::AllocatedBuffer FSR2rcasBuffer[MAX_FRAME_DRAWS];
	oGFX::AllocatedBuffer FSR2luminanceCB[MAX_FRAME_DRAWS];
	oGFX::AllocatedBuffer FSR2autoGen[MAX_FRAME_DRAWS];

	std::vector<oGFX::AllocatedBuffer> imguiVertexBuffer;
	std::vector<oGFX::AllocatedBuffer> imguiIndexBuffer;
	std::vector<oGFX::AllocatedBuffer> imguiConstantBuffer;

	oGFX::AllocatedBuffer lightingHistogram;
	oGFX::AllocatedBuffer LuminanceBuffer;
	oGFX::AllocatedBuffer LuminanceMonitor;
	void* monitorData{ nullptr };

	std::vector<DescriptorAllocator> descAllocs;
	DescriptorLayoutCache DescLayoutCache;

	FramebufferCache fbCache;

	GfxSamplerManager samplerManager;

	// Store the indirect draw commands containing index offsets and instance count per object

	//Scene objects
	std::mutex g_mut_globalModels;
	std::vector<gfxModel> g_globalModels;
	std::vector<SubMesh> g_globalSubmesh;

	std::mutex g_mut_workQueue;
	std::vector<std::function<void()>> g_workQueue;

	size_t frameCounter = 0;
	uint32_t currentFrame = 0;
	uint32_t getFrame() const;
	uint32_t getPreviousFrame() const;

	uint64_t uboDynamicAlignment{};
	static constexpr uint32_t numCameras = 2;
	uint32_t numAllocatedCameras{};

	struct RenderTarget
	{
		bool inUse = false;
		vkutils::Texture2D texture;
		vkutils::Texture2D depth;
		ImTextureID imguiTex{};
	};
	std::array<RenderTarget, 4>renderTargets;

	bool resizeSwapchain = false;
	bool m_prepared = false;
	bool m_reloadShaders = false;
	bool m_restartIMGUI = false;

	TaskManager g_taskManager;
	std::mutex g_mut_taskMap;
	std::unordered_map<std::thread::id, uint32_t> g_taskManagerMapping;
	uint32_t mappedThreadCnt{};
	uint32_t RegisterThreadMapping();

	// These variables area only to speedup development time by passing adjustable values from the C++ side to the shader.
	// Bind this to every single shader possible.
	// Remove this upon shipping the final product.
	struct ShaderDebugValues
	{
		glm::vec4 vector4_values0{};
		glm::vec4 vector4_values1{};
		glm::vec4 vector4_values2{};
		glm::vec4 vector4_values3{};
		glm::vec4 vector4_values4{};
		glm::vec4 vector4_values5{};
		glm::vec4 vector4_values6{};
		glm::vec4 vector4_values7{};
		glm::vec4 vector4_values8{};
		glm::vec4 vector4_values9{};
	}m_ShaderDebugValues;

public:
	
	bool m_DebugDrawDepthTest{ true };

	// TODO: remove
	struct EntityDetails
	{
		std::string name;
		glm::vec3 position{};
		glm::vec3 scale{1.0f};
		float rot{};
		glm::vec3 rotVec{0.0f,1.0f,0.0f};

		uint32_t modelID{}; // Index for the mesh
		uint32_t entityID{}; // Unique ID for this entity instance
		
		// Very ghetto... To move out to proper material system...
		// Actually 16 bits is enough...
		uint32_t bindlessGlobalTextureIndex_Albedo{ 0xFFFFFFFF };
		uint32_t bindlessGlobalTextureIndex_Normal{ 0xFFFFFFFF };
		uint32_t bindlessGlobalTextureIndex_Roughness{ 0xFFFFFFFF };
		uint32_t bindlessGlobalTextureIndex_Metallic{ 0xFFFFFFFF };

		oGFX::Sphere sphere;
		oGFX::AABB aabb;

		template <typename T>
		float GetBVHeuristic();

		template <>
		float GetBVHeuristic<oGFX::Sphere>()
		{
			return glm::pi<float>()* sphere.radius* sphere.radius;
		}

		template <>
		float GetBVHeuristic<oGFX::AABB>()
		{
			const auto width  = aabb.halfExt[0];
			const auto height = aabb.halfExt[1];
			const auto depth  = aabb.halfExt[2];
			return 8.0f * ( (width*height)*(width*depth)*(height*depth) );
		}
	};

	static ImTextureID CreateImguiBinding(VkSampler s, vkutils::Texture*);
	ImTextureID GetImguiID(uint32_t textureID);

	static VkPipelineShaderStageCreateInfo LoadShader(VulkanDevice& device, const std::string& fileName, VkShaderStageFlagBits stage);
	private:
		uint32_t CreateTextureImage(const oGFX::FileImageData& imageInfo);		
		uint32_t CreateTextureImageImmediate(const oGFX::FileImageData& imageInfo);		
		uint32_t CreateTextureImage(const std::string& fileName);
		
		uint32_t UpdateBindlessGlobalTexture(uint32_t textureID);		

		bool shadowsRendered{ false };

		void InitDefaultPrimatives();
		std::unique_ptr<ModelFileResource>def_cube;
		std::unique_ptr<ModelFileResource>def_sprite;
		std::unique_ptr<ModelFileResource>def_plane;
		std::unique_ptr<ModelFileResource>def_sphere;
		std::unique_ptr<oGFX::Font>def_font;

};



// Helper function to set Viewport & Scissor to the default window full extents.
void SetDefaultViewportAndScissor(VkCommandBuffer commandBuffer, VkViewport* vp = nullptr, VkRect2D* sc = nullptr);
// Helper function to draw a Full Screen Quad, without binding vertex and index buffers.
void DrawFullScreenQuad(VkCommandBuffer commandBuffer);

// Helper function just in case MultiDrawIndirect is not supported... (seriously wtf it is 2022...)
void DrawIndexedIndirect(
	VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride);
