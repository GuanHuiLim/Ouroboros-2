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
#include "VulkanFramebufferAttachment.h"
#include "GpuVector.h"
#include "gpuCommon.h"
#include "DescriptorBuilder.h"
#include "DescriptorAllocator.h"
#include "DescriptorLayoutCache.h"
#include "Geometry.h"

#include "Camera.h"

#include "imgui/imgui.h"

#include "GfxSampler.h"

#include "GraphicsWorld.h"

#include <vector>
#include <array>
#include <string>

struct Window;

int Win32SurfaceCreator(ImGuiViewport* vp, ImU64 device, const void* allocator, ImU64* outSurface);


class VulkanRenderer
{
public:

	static constexpr int MAX_FRAME_DRAWS = 2;
	static constexpr int MAX_OBJECTS = 2048;
	static constexpr VkFormat G_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;

#define OBJECT_INSTANCE_COUNT 128

	inline static PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName{ nullptr };

	~VulkanRenderer();

	void Init(const oGFX::SetupInfo& setupSpecs, Window& window);

	void CreateInstance(const oGFX::SetupInfo& setupSpecs);
	void CreateSurface(Window& window);
	void AcquirePhysicalDevice();
	void CreateLogicalDevice();
	void SetupSwapchain();
	void CreateRenderpass();
	void CreateDescriptorSetLayout();

	void CreatePushConstantRange();
	void CreateGraphicsPipeline();
	//void CreateDepthBufferImage();
	void CreateFramebuffers(); 
	void CreateCommandBuffers();

	ImTextureID myImg;
	VulkanFramebufferAttachment offscreenFB;
	VulkanFramebufferAttachment offscreenDepth;
	VkFramebuffer offscreenFramebuffer;
	VkRenderPass offscreenPass;
	void CreateOffscreenPass();
	void CreateOffscreenFB();
	void ResizeOffscreenFB();

	//---------- Device ----------

	inline static VulkanInstance m_instance{};
	inline static VulkanDevice m_device{};
	inline static VulkanSwapchain m_swapchain{};
	inline static std::vector<VkFramebuffer> swapChainFramebuffers;
	inline static uint32_t swapchainIdx{ 0 };

	//---------- DescriptorSetLayout & DescriptorSet ----------

	// For Deferred Lighting onwards
	inline static VkDescriptorSetLayout descriptorSetLayout_DeferredComposition;
	inline static VkDescriptorSet descriptorSet_DeferredComposition;

	// For unbounded array of texture descriptors, used in bindless approach
	inline static VkDescriptorSetLayout descriptorSetLayout_bindless;
	inline static VkDescriptorSet descriptorSet_bindless;

	// For GPU Scene
    inline static VkDescriptorSetLayout descriptorSetLayout_gpuscene;
    inline static VkDescriptorSet descriptorSet_gpuscene;

	// For UBO with the corresponding swap chain image
	inline static VkDescriptorSetLayout descriptorSetLayout_uniform;
	inline static std::vector<VkDescriptorSet> descriptorSets_uniform;

	void ResizeDeferredFB();

	void SetWorld(GraphicsWorld* world);
	inline static GraphicsWorld* currWorld{ nullptr };

	std::array<OmniLightInstance, 6> m_HardcodedOmniLights;

	struct LightUBO
	{
		OmniLightInstance lights[6];
		glm::vec4 viewPos;
	};
	inline static LightUBO lightUBO{};
	float timer{ 0.0f };

	inline static bool deferredRendering = true;

	inline static vk::Buffer lightsBuffer;
	void CreateLightingBuffers(); 
	void UpdateLights(float delta);
	void UploadLights();

	void CreateSynchronisation();
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets_GPUScene();

	struct ImGUIStructures
	{
		VkDescriptorPool descriptorPools{};
		VkRenderPass renderPass{};
		std::vector<VkFramebuffer> buffers;
	}m_imguiConfig{};
	 
	void InitImGUI();
	void ResizeGUIBuffers();
	void DrawGUI();
	void DestroyImGUI();

	//---------- Debug Draw Interface ----------

	void AddDebugBox(const AABB& aabb, const oGFX::Color& col, size_t loc = -1);
	void AddDebugSphere(const Sphere& sphere, const oGFX::Color& col,size_t loc = -1);
	void AddDebugTriangle(const Triangle& tri, const oGFX::Color& col,size_t loc = -1);

	void InitializeRenderBuffers();
	void DestroyRenderBuffers();
	void UpdateIndirectDrawCommands();
	void UploadInstanceData();
	uint32_t objectCount{};
	// Contains the instanced data
	inline static vk::Buffer instanceBuffer;

	bool PrepareFrame();
	void Draw();
	void RenderFrame();
	void Present();

	void SimplePass();

	void UpdateUniformBuffers();

	// Immediate command sending helper
	VkCommandBuffer beginSingleTimeCommands();
	// Immediate command sending helper
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	uint32_t CreateTexture(uint32_t width, uint32_t height,unsigned char* imgData);
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
	void UpdateDebugBuffers();

	void UpdateTreeBuffers();

	struct VertexBufferObject
	{
		GpuVector<oGFX::Vertex> VtxBuffer;
		GpuVector<uint32_t> IdxBuffer;
		size_t VtxOffset{};
		size_t IdxOffset{};
	};

	inline static VertexBufferObject g_MeshBuffers;

	inline static VertexBufferObject g_AABBMeshBuffers;
	std::vector<oGFX::Vertex> g_AABBMeshes;
	inline static VertexBufferObject g_SphereMeshBuffers;
	std::vector<oGFX::Vertex> g_SphereMeshes;


	inline static GpuVector<oGFX::Vertex> g_debugDrawVertBuffer{ &VulkanRenderer::m_device };
	inline static GpuVector<uint32_t> g_debugDrawIndxBuffer{ &VulkanRenderer::m_device };
	std::vector<oGFX::Vertex> g_debugDrawVerts;
	std::vector<uint32_t> g_debugDrawIndices;

	//TEMP
	struct DebugDraw
	{
		GpuVector<oGFX::Vertex> vbo{ &VulkanRenderer::m_device };
		GpuVector<uint32_t> ibo{ &VulkanRenderer::m_device };
		std::vector<oGFX::Vertex> vertex;
		std::vector<uint32_t> indices;
		bool dirty = true;
	};

	
	static constexpr size_t g_btmUp_AABB =0;
	static constexpr size_t g_topDwn_AABB =1;
	static constexpr size_t g_btmUp_Sphere=2;
	static constexpr size_t g_topDwn_Sphere=3;
	static constexpr size_t g_octTree_tris=4;
	static constexpr size_t g_octTree_box=5;
	static constexpr size_t g_BSP_tris=6;

	static constexpr size_t debugDrawBufferCnt = 7;

	inline static bool g_b_drawDebug[debugDrawBufferCnt];
	inline static DebugDraw g_DebugDraws[debugDrawBufferCnt];
	void InitTreeDebugDraws();
	void ShutdownTreeDebug();


	Model* LoadMeshFromFile(const std::string& file);
	Model* LoadMeshFromBuffers(std::vector<oGFX::Vertex>& vertex,std::vector<uint32_t>& indices, gfxModel* model);
	void SetMeshTextures(uint32_t modelID,uint32_t alb, uint32_t norm, uint32_t occlu, uint32_t rough);

	bool ResizeSwapchain();

	inline static Window* windowPtr{nullptr};

	//textures
	std::vector<vk::Texture2D> g_Textures;

	// - Synchronisation
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	// - Pipeline
	VkPipeline graphicsPSO{};
	VkPipeline wireframePSO{};
	inline static VkRenderPass renderPass_default{};

	inline static vk::Buffer indirectCommandsBuffer{};
	inline static VkPipeline indirectPipeline{};
	inline static VkPipelineLayout indirectPipeLayout{};
	inline static uint32_t indirectDrawCount{};

	inline static vk::Buffer boneMatrixBuffer{};
	inline static vk::Buffer skinningVertexBuffer{};
	inline static vk::Buffer globalLightBuffer{};

	// - Descriptors
	

	struct PushConstData
	{
		glm::mat4 xform{};
		glm::vec3 light{};
	};
	inline static VkPushConstantRange pushConstantRange{};

	VkDescriptorPool descriptorPool{};
	VkDescriptorPool samplerDescriptorPool{};
	
	//std::vector<VkDescriptorSet> samplerDescriptorSets;
	uint32_t bindlessGlobalTexturesNextIndex = 0;

	// SSBO
	std::vector<GPUTransform> gpuTransform;
	GpuVector<GPUTransform> gpuTransformBuffer{&m_device};

	std::vector<GPUTransform> debugTransform;
	GpuVector<GPUTransform> debugTransformBuffer{&m_device};
	
	// SSBO
	std::vector<VkBuffer> vpUniformBuffer;
	std::vector<VkDeviceMemory> vpUniformBufferMemory;

	inline static DescriptorAllocator DescAlloc;
	inline static DescriptorLayoutCache DescLayoutCache;

	GfxSamplerManager samplerManager;

	inline static std::vector<VkCommandBuffer> commandBuffers;

	// Store the indirect draw commands containing index offsets and instance count per object
	inline static std::vector<VkDrawIndexedIndirectCommand> m_DrawIndirectCommandsCPU;
	std::vector<VkDrawIndexedIndirectCommand> indirectDebugCommandsCPU;

	//Scene objects
	inline static std::vector<gfxModel> models;

	uint32_t currentFrame = 0;

	uint64_t uboDynamicAlignment;
	uint32_t numCameras;

	struct FrameContextUBO
	{
		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 viewProjection{ 1.0f };
		glm::vec4 cameraPosition{ 1.0f };
		glm::vec4 renderTimer{ 0.0f, 0.0f, 0.0f, 0.0f };
	} m_FrameContextUBO;

	bool resizeSwapchain = false;

	Camera camera;

public:
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
	inline static std::vector<EntityDetails> entities;

	static VkPipelineShaderStageCreateInfo LoadShader(VulkanDevice& device, const std::string& fileName, VkShaderStageFlagBits stage);
	private:
		uint32_t CreateTextureImage(const std::string& fileName);
		uint32_t CreateTextureImage(const oGFX::FileImageData& imageInfo);
		uint32_t UpdateBindlessGlobalTexture(vk::Texture2D texture);		

};

// Helper function to set Viewport & Scissor to the default window full extents.
void SetDefaultViewportAndScissor(VkCommandBuffer commandBuffer);
// Helper function to draw a Full Screen Quad, without binding vertex and index buffers.
void DrawFullScreenQuad(VkCommandBuffer commandBuffer);

// Helper function just in case MultiDrawIndirect is not supported... (seriously wtf it is 2022...)
void DrawIndexedIndirect(
	VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride);
