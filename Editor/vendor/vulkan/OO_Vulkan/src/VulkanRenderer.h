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
#include "GpuVector.h"
#include "gpuCommon.h"
#include "DescriptorBuilder.h"
#include "DescriptorAllocator.h"
#include "DescriptorLayoutCache.h"
#include "FramebufferCache.h"
#include "Geometry.h"

#include "Camera.h"

#include "imgui/imgui.h"

#include "GfxSampler.h"

#include "GraphicsWorld.h"
#include "GraphicsBatch.h"

#include <vector>
#include <array>
#include <set>
#include <string>

struct Window;

int Win32SurfaceCreator(ImGuiViewport* vp, ImU64 device, const void* allocator, ImU64* outSurface);

// Moving all the Descriptor Set Layout out of the VulkanRenderer class abomination...
struct SetLayoutDB // Think of a better name? Very short and sweet for easy typing productivity?
{
    // For GPU Scene
    inline static VkDescriptorSetLayout gpuscene;
    // For UBO with the corresponding swap chain image
    inline static VkDescriptorSetLayout FrameUniform;
    // For unbounded array of texture descriptors, used in bindless approach
    inline static VkDescriptorSetLayout bindless;
	// For lighting
	inline static VkDescriptorSetLayout DeferredLightingComposition;

	inline static VkDescriptorSetLayout lights;
	// 
	inline static VkDescriptorSetLayout ForwardDecal;
};

// Moving all the Descriptor Set Layout out of the VulkanRenderer class abomination...
struct PSOLayoutDB
{
	inline static VkPipelineLayout defaultPSOLayout;
	inline static VkPipelineLayout deferredLightingCompositionPSOLayout;
	inline static VkPipelineLayout forwardDecalPSOLayout;
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
		glm::vec4 renderTimer{ 0.0f, 0.0f, 0.0f, 0.0f };

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

}

class VulkanRenderer
{
public:
	static VulkanRenderer* s_vulkanRenderer;

	static constexpr int MAX_FRAME_DRAWS = 2;
	static constexpr int MAX_OBJECTS = 2048;
	static constexpr VkFormat G_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;

	static int ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface);

#define OBJECT_INSTANCE_COUNT 128

	 PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName{ nullptr };

	~VulkanRenderer();

	static VulkanRenderer* get();

	void Init(const oGFX::SetupInfo& setupSpecs, Window& window);

	void CreateInstance(const oGFX::SetupInfo& setupSpecs);
	void CreateDebugCallback();
	void DestroyDebugMessenger();
	void CreateSurface(const oGFX::SetupInfo& setupSpecs, Window& window);
	void AcquirePhysicalDevice(const oGFX::SetupInfo& setupSpecs);
	void CreateLogicalDevice(const oGFX::SetupInfo& setupSpecs);
	void SetupSwapchain();
	void CreateDefaultRenderpass();
	void CreateDefaultDescriptorSetLayout();

	void BlitFramebuffer(VkCommandBuffer cmd, vkutils::Texture2D& src,VkImageLayout srcFinal, vkutils::Texture2D& dst,VkImageLayout dstFinal);

	void CreateDefaultPSOLayouts();
	//void CreateDepthBufferImage();
	void CreateFramebuffers(); 
	void CreateCommandBuffers();

	ImTextureID myImg;

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
	VkDescriptorSet descriptorSet_DeferredComposition;
	// For unbounded array of texture descriptors, used in bindless approach
	VkDescriptorSet descriptorSet_bindless;
	// For GPU Scene
	VkDescriptorSet descriptorSet_gpuscene;

	VkDescriptorSet descriptorSet_lights;

	VkDescriptorSet descriptorSet_bones;

	VkDescriptorSet descriptorSet_objInfos;
	// For UBO with the corresponding swap chain image
	std::vector<VkDescriptorSet> descriptorSets_uniform;

	void ResizeDeferredFB();

	void SetWorld(GraphicsWorld* world);
	void InitWorld(GraphicsWorld* world);
	void DestroyWorld(GraphicsWorld* world);
	GraphicsWorld* currWorld{ nullptr };
	uint32_t renderIteration{ 0};
	uint32_t renderTargetInUseID{ 0 };
	float renderClock{ 0.0f };

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
	void DrawGUI();
	void DestroyImGUI();
	void RestartImgui();
	void ImguiSoftDestroy();

	void InitializeRenderBuffers();
	void DestroyRenderBuffers();
	void GenerateCPUIndirectDrawCommands();
	void UploadInstanceData();
	uint32_t objectCount{};
	// Contains the instanced data
	vkutils::Buffer instanceBuffer;

	bool PrepareFrame();
	void BeginDraw();
	void RenderFrame();
	void Present();

	void UpdateUniformBuffers();

	// Immediate command sending helper
	VkCommandBuffer beginSingleTimeCommands();
	// Immediate command sending helper
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	uint32_t CreateTexture(uint32_t width, uint32_t height, unsigned char* imgData);
	uint32_t CreateTexture(const std::string& fileName);
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

	IndexedVertexBuffer g_GlobalMeshBuffers;

	GpuVector<oGFX::DebugVertex> g_DebugDrawVertexBufferGPU;
	GpuVector<uint32_t> g_DebugDrawIndexBufferGPU;
	std::vector<oGFX::DebugVertex> g_DebugDrawVertexBufferCPU;
	std::vector<uint32_t> g_DebugDrawIndexBufferCPU;

	ModelFileResource* LoadModelFromFile(const std::string& file);
	ModelFileResource* LoadMeshFromBuffers(std::vector<oGFX::Vertex>& vertex, std::vector<uint32_t>& indices, gfxModel* model);
	void LoadSubmesh(gfxModel& mdl, SubMesh& submesh, aiMesh* aimesh, ModelFileResource* modelFile);
	void LoadBoneInformation(ModelFileResource& fileData, oGFX::Skeleton& skeleton, aiMesh& aimesh, std::vector<oGFX::BoneWeight>& boneWeights, uint32_t& vCnt);
	void BuildSkeletonRecursive(ModelFileResource& fileData, aiNode* ainode, oGFX::BoneNode* node, glm::mat4 parentXform = glm::mat4(1.0f), std::string prefix = std::string("\t"));
	const oGFX::Skeleton* GetSkeleton(uint32_t modelID);
	oGFX::CPUSkeletonInstance* CreateSkeletonInstance(uint32_t modelID);

	bool ResizeSwapchain();

	Window* windowPtr{ nullptr };

	//textures
	std::vector<vkutils::Texture2D> g_Textures;
	std::vector<ImTextureID> g_imguiIDs;

	uint32_t whiteTextureID = static_cast<uint32_t>(-1);
	uint32_t blackTextureID = static_cast<uint32_t>(-1);
	uint32_t normalTextureID = static_cast<uint32_t>(-1);
	uint32_t pinkTextureID = static_cast<uint32_t>(-1);

	// - Synchronisation
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	// - Pipeline
	VkRenderPass renderPass_default{};
	VkRenderPass renderPass_default2{};

	vkutils::Buffer indirectCommandsBuffer{};
	GpuVector<oGFX::IndirectCommand> shadowCasterCommandsBuffer{};
	uint32_t indirectDrawCount{};

	GpuVector<oGFX::BoneWeight> skinningVertexBuffer{};
	GpuVector<SpotLightInstance> globalLightBuffer{};

	// - Descriptors

	VkDescriptorPool descriptorPool{};
	VkDescriptorPool samplerDescriptorPool{};

	//std::vector<VkDescriptorSet> samplerDescriptorSets;
	uint32_t bindlessGlobalTexturesNextIndex = 0;

	// SSBO
	std::vector<glm::mat4> boneMatrices{};
	GpuVector<glm::mat4> gpuBoneMatrixBuffer{};

	// SSBO
	std::vector<GPUTransform> gpuTransform{};
	GpuVector<GPUTransform> gpuTransformBuffer;

	// SSBO
	std::vector<GPUObjectInformation> objectInformation;
	GpuVector<GPUObjectInformation> objectInformationBuffer{};
	
	// SSBO
	std::vector<VkBuffer> vpUniformBuffer{};
	std::vector<VkDeviceMemory> vpUniformBufferMemory{};

	std::vector<DescriptorAllocator> descAllocs;
	DescriptorLayoutCache DescLayoutCache;

	FramebufferCache fbCache;

	GfxSamplerManager samplerManager;

	std::vector<VkCommandBuffer> commandBuffers;

	// Store the indirect draw commands containing index offsets and instance count per object

	//Scene objects
	std::vector<gfxModel> g_globalModels;

	uint32_t currentFrame = 0;

	uint64_t uboDynamicAlignment;
	static constexpr uint32_t numCameras = 2;
	uint32_t numAllocatedCameras;

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

		Sphere sphere;
		AABB aabb;

		template <typename T>
		float GetBVHeuristic();

		template <>
		float GetBVHeuristic<Sphere>()
		{
			return glm::pi<float>()* sphere.radius* sphere.radius;
		}

		template <>
		float GetBVHeuristic<AABB>()
		{
			const auto width  = aabb.halfExt[0];
			const auto height = aabb.halfExt[1];
			const auto depth  = aabb.halfExt[2];
			return 8.0f * ( (width*height)*(width*depth)*(height*depth) );
		}
	};

	static ImTextureID CreateImguiBinding(VkSampler s, VkImageView v, VkImageLayout l);
	ImTextureID GetImguiID(uint32_t textureID);

	static VkPipelineShaderStageCreateInfo LoadShader(VulkanDevice& device, const std::string& fileName, VkShaderStageFlagBits stage);
	private:
		uint32_t CreateTextureImage(const oGFX::FileImageData& imageInfo);		
		uint32_t CreateTextureImage(const std::string& fileName);
		uint32_t AddBindlessGlobalTexture(vkutils::Texture2D texture);		

		

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
