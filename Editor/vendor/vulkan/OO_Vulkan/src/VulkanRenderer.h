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
#include "FramebufferCache.h"
#include "Geometry.h"

#include "Camera.h"

#include "imgui/imgui.h"

#include "GfxSampler.h"

#include "GraphicsWorld.h"

#include <vector>
#include <array>
#include <set>
#include <string>

struct Window;

int Win32SurfaceCreator(ImGuiViewport* vp, ImU64 device, const void* allocator, ImU64* outSurface);

// Moving all the Descriptor Set Layout out of the VulkanRenderer class abomination...
struct LayoutDB // Think of a better name? Very short and sweet for easy typing productivity?
{
    // For GPU Scene
    inline static VkDescriptorSetLayout gpuscene;
    // For UBO with the corresponding swap chain image
    inline static VkDescriptorSetLayout uniform;
    // For unbounded array of texture descriptors, used in bindless approach
    inline static VkDescriptorSetLayout bindless;
	// For lighting
	inline static VkDescriptorSetLayout DeferredComposition;
	// 
	inline static VkDescriptorSetLayout ForwardDecal;
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
		glm::vec4 cameraPosition{ 1.0f };
		glm::vec4 renderTimer{ 0.0f, 0.0f, 0.0f, 0.0f };
	};

    struct LightUBO
    {
        OmniLightInstance lights[6];
        glm::vec4 viewPos;
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
	void CreateSurface(const oGFX::SetupInfo& setupSpecs, Window& window);
	void AcquirePhysicalDevice(const oGFX::SetupInfo& setupSpecs);
	void CreateLogicalDevice(const oGFX::SetupInfo& setupSpecs);
	void SetupSwapchain();
	void CreateRenderpass();
	void CreateDescriptorSetLayout();

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

    bool m_imguiInitialized = false;
	bool m_initialized = false;

	//---------- Device ----------

    VulkanInstance m_instance{};
    VulkanDevice m_device{};
	VulkanSwapchain m_swapchain{};
	std::vector<VkFramebuffer> swapChainFramebuffers;
	uint32_t swapchainIdx{ 0 };

	//---------- DescriptorSet ----------

	// For Deferred Lighting onwards
	VkDescriptorSet descriptorSet_DeferredComposition;
	// For unbounded array of texture descriptors, used in bindless approach
	VkDescriptorSet descriptorSet_bindless;
	// For GPU Scene
	VkDescriptorSet descriptorSet_gpuscene;
	// For UBO with the corresponding swap chain image
    std::vector<VkDescriptorSet> descriptorSets_uniform;

	void ResizeDeferredFB();

	void SetWorld(GraphicsWorld* world);
	GraphicsWorld* currWorld{ nullptr };
	
	bool deferredRendering = true;

    vkutils::Buffer lightsBuffer;
	void CreateLightingBuffers(); 
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
	void DebugGUIcalls();
	void DrawGUI();
	void DestroyImGUI();

	//---------- Debug Draw Interface ----------

	void AddDebugBox(const AABB& aabb, const oGFX::Color& col, size_t loc = -1);
	void AddDebugSphere(const Sphere& sphere, const oGFX::Color& col,size_t loc = -1);
	void AddDebugTriangle(const Triangle& tri, const oGFX::Color& col,size_t loc = -1);

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
    void UpdateDebugBuffers();

    struct VertexBufferObject
    {
        GpuVector<oGFX::Vertex> VtxBuffer;
        GpuVector<uint32_t> IdxBuffer;
        uint32_t VtxOffset{};
        uint32_t IdxOffset{};
    };

    VertexBufferObject g_GlobalMeshBuffers;

    VertexBufferObject g_AABBMeshBuffers;
    std::vector<oGFX::Vertex> g_AABBMeshes;
    VertexBufferObject g_SphereMeshBuffers;
    std::vector<oGFX::Vertex> g_SphereMeshes;

    GpuVector<oGFX::Vertex> g_debugDrawVertBuffer;
    GpuVector<uint32_t> g_debugDrawIndxBuffer;
    std::vector<oGFX::Vertex> g_debugDrawVerts;
    std::vector<uint32_t> g_debugDrawIndices;

	//TEMP
	struct DebugDraw
	{
		GpuVector<oGFX::Vertex> vbo;
		GpuVector<uint32_t> ibo;
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

	 bool g_b_drawDebug[debugDrawBufferCnt];
	 DebugDraw g_DebugDraws[debugDrawBufferCnt];
	void InitTreeDebugDraws();
	void ShutdownTreeDebug();


	Model* LoadModelFromFile(const std::string& file);
	Model* LoadMeshFromBuffers(std::vector<oGFX::Vertex>& vertex,std::vector<uint32_t>& indices, gfxModel* model);
	void SetMeshTextures(uint32_t modelID,uint32_t alb, uint32_t norm, uint32_t occlu, uint32_t rough);

	bool ResizeSwapchain();

	 Window* windowPtr{nullptr};

	//textures
	std::vector<vkutils::Texture2D> g_Textures;
	std::vector<ImTextureID> g_imguiIDs;

	// - Synchronisation
	 std::vector<VkSemaphore> imageAvailable;
	 std::vector<VkSemaphore> renderFinished;
	 std::vector<VkFence> drawFences;
	
	// - Pipeline
	VkPipeline graphicsPSO{};
	VkPipeline wireframePSO{};
	 VkRenderPass renderPass_default{};
	 VkRenderPass renderPass_default2{};

	 vkutils::Buffer indirectCommandsBuffer{};
	 VkPipeline indirectPSO{};
	 VkPipelineLayout indirectPSOLayout{};
	 uint32_t indirectDrawCount{};

	 vkutils::Buffer boneMatrixBuffer{};
	 vkutils::Buffer skinningVertexBuffer{};
	 vkutils::Buffer globalLightBuffer{};

	// - Descriptors
	
	VkDescriptorPool descriptorPool{};
	VkDescriptorPool samplerDescriptorPool{};
	
	//std::vector<VkDescriptorSet> samplerDescriptorSets;
	uint32_t bindlessGlobalTexturesNextIndex = 0;

	// SSBO
	 std::vector<GPUTransform> gpuTransform{};
	GpuVector<GPUTransform> gpuTransformBuffer;

	 std::vector<GPUTransform> debugTransform;
	GpuVector<GPUTransform> debugTransformBuffer;
	
	// SSBO
	 std::vector<VkBuffer> vpUniformBuffer{};
	 std::vector<VkDeviceMemory> vpUniformBufferMemory{};

	 DescriptorAllocator DescAlloc;
	 DescriptorLayoutCache DescLayoutCache;

	 FramebufferCache fbCache;

	GfxSamplerManager samplerManager;

	 std::vector<VkCommandBuffer> commandBuffers;

	// Store the indirect draw commands containing index offsets and instance count per object

	//Scene objects
	 std::vector<gfxModel> models;

	uint32_t currentFrame = 0;

	uint64_t uboDynamicAlignment;
	uint32_t numCameras;

    

	bool resizeSwapchain = false;
	bool m_prepared = false;

	Camera camera;

public:
	
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
		uint32_t UpdateBindlessGlobalTexture(vkutils::Texture2D texture);		

		

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
