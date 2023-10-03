/************************************************************************************//*!
\file           VulkanRenderer.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief               Defines the full vulkan renderer class. 
The entire class encapsulates the vulkan renderer and acts as an interface for external engines

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "VulkanRenderer.h"

#include "MathCommon.h"

#include <vulkan/vulkan.h>

#pragma warning( push )
#pragma warning( disable : 26451 ) // vendor overflow
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning( pop )

#include "VulkanUtils.h"
#include "Window.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>
static ImDrawListSharedData s_imguiSharedData;

#include "../shaders/shared_structs.h"

#include "GfxRenderpass.h"


extern GfxRenderpass* g_BloomPass;
extern GfxRenderpass* g_DebugDrawRenderpass;
extern GfxRenderpass* g_ImguiRenderpass;
extern GfxRenderpass* g_ForwardParticlePass;
extern GfxRenderpass* g_ForwardUIPass;
extern GfxRenderpass* g_GBufferRenderPass;
extern GfxRenderpass* g_LightingPass;
extern GfxRenderpass* g_LightingHistogram;
extern GfxRenderpass* g_ShadowPass;
extern GfxRenderpass* g_SSAORenderPass;
extern GfxRenderpass* g_SkyRenderPass;
extern GfxRenderpass* g_ZPrePass;

#if defined (ENABLE_DECAL_IMPLEMENTATION)
	#include "renderpass/ForwardDecalRenderpass.h"
#endif

#include "DefaultMeshCreator.h"

#include "GraphicsBatch.h"
#include "FramebufferBuilder.h"
#include "DelayedDeleter.h"

#include "IcoSphereCreator.h"
#include "BoudingVolume.h"

#include "Profiling.h"
#include "DebugDraw.h"

#include <vector>
#include <set>
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <random>
#include <filesystem>
#include <sstream>

// ordering important
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_MULTIPLE_MASTERS_H


#pragma warning( push )
#pragma warning( disable : 26451 )

#define MSDFGEN_CORE_ONLY
#include "msdfgen.h"
#include "msdfgen-ext.h"
#include "core/ShapeDistanceFinder.h"
#define MSDF_ATLAS_PUBLIC 
#define MSDF_ATLAS_NO_ARTERY_FONT 
#include "msdf-atlas-gen.h"

#pragma warning( pop )

VulkanRenderer* VulkanRenderer::s_vulkanRenderer{ nullptr };

// vulkan debug callback
#pragma optimize("", off)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	
	// Ignore all performance related warnings for now..

	constexpr int VALIDATION_MSG_Shader_OutputNotConsumed = 0x609a13b;
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT && pCallbackData->messageIdNumber != VALIDATION_MSG_Shader_OutputNotConsumed) //&& !(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
	{
		
		int x;
		std::cerr << pCallbackData->pMessage << "\n" << std::endl;
		//assert(false); temp comment out
			x=5; // for breakpoint
	}

	return VK_FALSE;

}
#pragma optimize("", on)

int VulkanRenderer::ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{
	auto* hdl = viewport->PlatformHandle;
	(void)vk_allocator;

	// Create temporary object to hold the data
	Window temp;
	temp.rawHandle = hdl;
		
		
	auto result = VulkanRenderer::get()->m_instance.CreateSurface(temp, *(VkSurfaceKHR*)out_vk_surface);
	
	//unbind to prevent destrauction
	temp.rawHandle = nullptr;

	if (result == oGFX::ERROR_VAL)
	{			
		return oGFX::ERROR_VAL;
	}

	return oGFX::SUCCESS_VAL;
		
}

VulkanRenderer::~VulkanRenderer()
{ 
	//wait until no actions being run on device before destorying
	vkDeviceWaitIdle(m_device.logicalDevice);

	std::fstream s("stats.txt", std::ios::out);
	if (s)
	{
		s << "buffer : " << accumulatedBytes << std::endl;
		s << "texture : " << totalTextureSizeLoaded << std::endl;
	}
	s.close();

	if (g_cubeMap.image.image != VK_NULL_HANDLE) {
		g_cubeMap.destroy();
	}
	if (g_radianceMap.image.image != VK_NULL_HANDLE) {
		g_radianceMap.destroy();
	}
	if (g_prefilterMap.image.image != VK_NULL_HANDLE) {
		g_prefilterMap.destroy();
	}
	if (g_brdfLUT.image.image != VK_NULL_HANDLE) {
		g_brdfLUT.destroy();
	}

	for (size_t i = 0; i < renderTargets.size(); i++)
	{
		if (renderTargets[i].texture.image.image)
		{
			renderTargets[i].texture.destroy();
		}
		if (renderTargets[i].depth.image.image)
		{
			renderTargets[i].depth.destroy();
		}
	}

	RenderPassDatabase::Shutdown();

#if VULKAN_MESSENGER
	DestroyDebugMessenger();
#endif // VULKAN_MESSENGER

	fbCache.Cleanup();

	DestroyRenderBuffers();

	samplerManager.Shutdown();

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{

		gpuTransformBuffer[i].destroy();
	}

	g_GlobalMeshBuffers.IdxBuffer.destroy();
	g_GlobalMeshBuffers.VtxBuffer.destroy();

	if (m_imguiInitialized)
	{
		DestroyImGUI();
	}

	DescLayoutCache.Cleanup();

	for (size_t i = 0; i < descAllocs.size(); i++)
	{
		descAllocs[i].Cleanup();
	}

	for (size_t i = 0; i < g_globalModels.size(); i++)
	{
		g_globalModels[i].destroy(m_device.logicalDevice);
	}	
	for (size_t i = 0; i < g_Textures.size(); i++)
	{
		g_Textures[i].destroy();
	}
	DelayedDeleter::get()->Shutdown();

	// global sampler pool
	vkDestroyDescriptorPool(m_device.logicalDevice, samplerDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_device.logicalDevice, SetLayoutDB::bindless, nullptr);

	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(m_device.logicalDevice, framebuffer, nullptr);
	}

	vkDestroyDescriptorPool(m_device.logicalDevice, descriptorPool, nullptr);
	for (size_t i = 0; i < vpUniformBuffer.size(); i++)
	{
		vmaDestroyBuffer(m_device.m_allocator, vpUniformBuffer[i].buffer, vpUniformBuffer[i].alloc);
	}

	for (size_t i = 0; i < drawFences.size(); i++)
	{
		vkDestroyFence(m_device.logicalDevice, drawFences[i], nullptr);
		vkDestroySemaphore(m_device.logicalDevice, renderSemaphore[i], nullptr);
		vkDestroySemaphore(m_device.logicalDevice, presentSemaphore[i], nullptr);
	}
	vkDestroySemaphore(m_device.logicalDevice, frameCountSemaphore, nullptr);

	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::defaultPSOLayout, nullptr);
	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::fullscreenBlitPSOLayout, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, pso_utilFullscreenBlit, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, pso_utilAMDSPD, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, pso_radiance, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, pso_prefilter, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, pso_brdfLUT, nullptr);
	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::AMDSPDPSOLayout, nullptr);
	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::RadiancePSOLayout, nullptr);
	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::prefilterPSOLayout, nullptr);
	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::BRDFLUTPSOLayout, nullptr);

	renderPass_default.destroy();
	renderPass_default_noDepth.destroy();
	renderPass_HDR.destroy();
	renderPass_HDR_noDepth.destroy();
	
	
	PROFILE_GPU_SHUTDOWN();
}

VulkanRenderer* VulkanRenderer::get()
{
	if (s_vulkanRenderer == nullptr)
	{
		s_vulkanRenderer = new VulkanRenderer();
	}
	return s_vulkanRenderer;
}

bool VulkanRenderer::Init(const oGFX::SetupInfo& setupSpecs, Window& window)
{
		
	g_globalModels.reserve(MAX_OBJECTS);
	std::cout << "create instance\n";
	CreateInstance(setupSpecs);
	std::cout << "done instance\n";

#if VULKAN_MESSENGER
	CreateDebugCallback();
#endif // VULKAN_MESSENGER

	CreateSurface(setupSpecs,window);
	// set surface for imgui
	Window::SurfaceFormat = (uint64_t)window.SurfaceFormat;

	AcquirePhysicalDevice(setupSpecs);
	CreateLogicalDevice(setupSpecs);

	InitVMA(setupSpecs);

	//if (m_device.debugMarker)
	//{
	// TODO MAKE SURE THIS IS SUPPORTED
	pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(m_device.logicalDevice, "vkDebugMarkerSetObjectNameEXT");
	pfnDebugMarkerRegionBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(m_device.logicalDevice, "vkCmdDebugMarkerBeginEXT");
	pfnDebugMarkerRegionEnd= (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(m_device.logicalDevice, "vkCmdDebugMarkerEndEXT");
	//}
	//
	SetupSwapchain();

	InitializeRenderBuffers();

	// Initialize all sampler objects
	samplerManager.Init();

	CreateDefaultRenderpass();
	CreateUniformBuffers();
	CreateDefaultDescriptorSetLayout();

	fbCache.Init(m_device.logicalDevice);
	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
	gpuTransformBuffer[i].Init(&m_device,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	}
	//gpuTransformBuffer.reserve(MAX_OBJECTS);


	CreateDescriptorSets_GPUScene();
	CreateDescriptorSets_Lights();

	CreateDefaultPSOLayouts();
	CreateDefaultPSO();

	if (setupSpecs.useOwnImgui)
	{
		InitImGUI();
	}

	CreateLightingBuffers();


	// Calls "Init()" on all registered render passes. Order is not guarunteed.
	auto rpd = RenderPassDatabase::Get();
	GfxRenderpass* ptr;
	rpd->RegisterRenderPass(g_ShadowPass);
	rpd->RegisterRenderPass(g_GBufferRenderPass);
	rpd->RegisterRenderPass(g_ZPrePass);
	rpd->RegisterRenderPass(g_SkyRenderPass);
	rpd->RegisterRenderPass(g_DebugDrawRenderpass);
	rpd->RegisterRenderPass(g_ImguiRenderpass);
	rpd->RegisterRenderPass(g_LightingPass);
	rpd->RegisterRenderPass(g_LightingHistogram);
	rpd->RegisterRenderPass(g_SSAORenderPass);
	rpd->RegisterRenderPass(g_ForwardParticlePass);
	rpd->RegisterRenderPass(g_ForwardUIPass);
	rpd->RegisterRenderPass(g_BloomPass);
#if defined (ENABLE_DECAL_IMPLEMENTATION)
	ptr = new ForwardDecalRenderpass;
	rpd->RegisterRenderPass(ptr);
#endif

	CreateFramebuffers();

	//CreateCommandBuffers();
	
	CreateDescriptorPool();

	g_Textures.reserve(2048);
	g_globalModels.reserve(2048);
	g_imguiIDs.reserve(2048);

	uint32_t whiteTexture = 0xFFFFFFFF; // ABGR
	uint32_t blackTexture = 0xFF000000; // ABGR
	uint32_t normalTexture = 0xFFFF8080; // ABGR
	uint32_t pinkTexture = 0xFFA040A0; // ABGR

	whiteTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&whiteTexture));
	blackTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&blackTexture));
	normalTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&normalTexture));
	pinkTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&pinkTexture));
		

	RenderPassDatabase::InitAllRegisteredPasses();

	auto& shadowTexture = attachments.shadow_depth;
	shadowTexture.updateDescriptor();

	CreateSynchronisation();

	InitDebugBuffers();
		
	InitDefaultPrimatives();

	std::array<VkQueue, 1> cmdQueues{m_device.graphicsQueue};
	std::array<uint32_t, 1> cmdFamily{(uint32_t)m_device.queueIndices.graphicsFamily};
	std::array<VkPhysicalDevice, 1> physDevs{ m_device.physicalDevice};
	std::array<VkDevice, 1> logicDevs{ m_device.logicalDevice};

	PROFILE_INIT_VULKAN(logicDevs.data(), physDevs.data(), cmdQueues.data(), cmdFamily.data(), 1, nullptr);
	
	// by now we should have crashed if not ok
	//std::cerr << "VulkanRenderer::Init failed: " << e.what() << std::endl;
	//__debugbreak();

	return oGFX::SUCCESS_VAL;
	
}

void VulkanRenderer::ReloadShaders()
{
	vkDeviceWaitIdle(m_device.logicalDevice);
	
	CreateDefaultPSO();
	
	RenderPassDatabase::ReloadAllShaders();
}

void VulkanRenderer::CreateInstance(const oGFX::SetupInfo& setupSpecs)
{
		m_instance.Init(setupSpecs);
}
class SDL_Window;
void VulkanRenderer::CreateSurface(const oGFX::SetupInfo& setupSpecs, Window& window)
{
    windowPtr = &window;
	if (window.m_type == Window::WindowType::SDL2)
	{
		assert(setupSpecs.SurfaceFunctionPointer); // Surface pointer doesnt work	
		std::function<bool()> fn = setupSpecs.SurfaceFunctionPointer;
		auto result = fn();
	}
	else
	{
		m_instance.CreateSurface(window,m_instance.surface);
	}
}

void VulkanRenderer::AcquirePhysicalDevice(const oGFX::SetupInfo& setupSpecs)
{
    m_device.InitPhysicalDevice(setupSpecs,m_instance);
}

void VulkanRenderer::CreateLogicalDevice(const oGFX::SetupInfo& setupSpecs)
{
    m_device.InitLogicalDevice(setupSpecs,m_instance);
}

void VulkanRenderer::InitVMA(const oGFX::SetupInfo& setupSpecs)
{
	m_device.InitAllocator(setupSpecs, m_instance);
}

void VulkanRenderer::SetupSwapchain()
{
	m_swapchain.Init(m_instance,m_device);
}

void VulkanRenderer::CreateDefaultRenderpass()
{
	if (renderPass_default.pass)
	{
		return;
	}

	// ATTACHMENTS
	// Colour attachment of render pass
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = m_swapchain.swapChainImageFormat;  //format to use for attachment
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//number of samples to use for multisampling
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;//descripts what to do with attachment before rendering
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//describes what to do with attachment after rendering
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //describes what do with with stencil before rendering
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //describes what do with with stencil before rendering

	//frame buffer data will be stored as image, but images can be given different data layouts
	//to give optimal use for certain operations
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //image data layout before render pass starts
	//colourAttachment.finalLayout = VK_IMAGE_LAYOUT_ENT_SRC_KHR; //image data layout aftet render pass ( to change to)
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //image data layout aftet render pass ( to change to)

	
	// todo editor??
	//colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //image data layout aftet render pass ( to change to)

	// Depth attachment of render pass
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = G_DEPTH_FORMAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// REFERENCES 
	//Attachment reference uses an atttachment index that refers to index i nthe attachment list passed to renderPassCreataeInfo
	VkAttachmentReference  colourAttachmentReference = {};
	colourAttachmentReference.attachment = 0;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment reference
	VkAttachmentReference depthAttachmentReference{};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//information about a particular subpass the render pass is using
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; //pipeline type subpass is to be bound to
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colourAttachmentReference;
	subpass.pDepthStencilAttachment = &depthAttachmentReference;

	// Need to determine when layout transitions occur using subpass dependancies
	std::array<VkSubpassDependency, 2> subpassDependancies;

	//conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transiotion msut happen after...
	subpassDependancies[0].srcSubpass = VK_SUBPASS_EXTERNAL; //subpass index (VK_SUBPASS_EXTERNAL = special vallue meaning outside of renderpass)
	subpassDependancies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Pipeline stage
	subpassDependancies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; //Stage acces mas (memory access)
																	  // but must happen before...
	subpassDependancies[0].dstSubpass = 0;
	subpassDependancies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependancies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependancies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	//conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transiotion msut happen after...
	subpassDependancies[1].srcSubpass = 0;
	subpassDependancies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependancies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	// but must happen before...
	subpassDependancies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependancies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependancies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependancies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	std::array<VkAttachmentDescription, 2> renderpassAttachments = { colourAttachment,depthAttachment };

	//create info for render pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderpassAttachments.size());
	renderPassCreateInfo.pAttachments = renderpassAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependancies.size());
	renderPassCreateInfo.pDependencies = subpassDependancies.data();

	renderPass_default.name = "defaultRenderPass";
	renderPass_default.Init(m_device, renderPassCreateInfo);

	subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
	renderPassCreateInfo.attachmentCount = 1; // colour only
	renderPassCreateInfo.dependencyCount = 0; // colour only
	renderPass_default_noDepth.name = "defaultRenderPass_noDepth";
	renderPass_default_noDepth.Init(m_device, renderPassCreateInfo);


	renderpassAttachments[0].format = G_HDR_FORMAT_ALPHA;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderpassAttachments.size());
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependancies.size());
	renderPass_HDR.name = "defaulRenderpassHDR";
	renderPass_HDR.Init(m_device, renderPassCreateInfo);


	subpass.pDepthStencilAttachment = VK_NULL_HANDLE;
	renderPassCreateInfo.attachmentCount = 1; // colour only
	renderPassCreateInfo.dependencyCount = 0; // colour only
	renderPass_HDR_noDepth.name = "defaultRenderPassHDR_noDepth";
	renderPass_HDR_noDepth.Init(m_device, renderPassCreateInfo);
	

	//renderPassCreateInfo.attachmentCount = 1; // only use colour attachment;
	//renderPassCreateInfo.dependencyCount = 0; // colour only
	//
	//renderPass_blit.name = "renderPass_blit";
	//renderPass_blit.Init(m_device, renderPassCreateInfo);
}

void VulkanRenderer::CreateDefaultDescriptorSetLayout()
{
	descAllocs.resize(m_swapchain.swapChainImages.size());
	for (size_t i = 0; i < descAllocs.size(); i++)
	{
		descAllocs[i].Init(m_device.logicalDevice);
	}

	DescLayoutCache.Init(m_device.logicalDevice);

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_device.physicalDevice, &props);
	size_t minUboAlignment = props.limits.minUniformBufferOffsetAlignment;
	//auto dynamicAlignment = sizeof(glm::mat4);
	uboDynamicAlignment = sizeof(CB::FrameContextUBO);
	if (minUboAlignment > 0)
	{
		uboDynamicAlignment = (uboDynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}

	VkDeviceSize vpBufferSize = uboDynamicAlignment * numCameras;

	descriptorSets_uniform.resize(m_swapchain.swapChainImages.size());
	// UBO for each swapchain images
	for (size_t i = 0; i < m_swapchain.swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo vpBufferInfo{};
		vpBufferInfo.buffer = vpUniformBuffer[i].buffer;	// buffer to get data from
		vpBufferInfo.offset = 0;					// position of start of data
		vpBufferInfo.range = sizeof(CB::FrameContextUBO);// size of data

		DescriptorBuilder::Begin()
			.BindBuffer(0, &vpBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT| VK_SHADER_STAGE_COMPUTE_BIT)
			.Build(descriptorSets_uniform[i], SetLayoutDB::FrameUniform);
	}
	
	//UNIFORM VALUES DESCRIPTOR SET LAYOUT
	// UboViewProejction binding info
	//VkDescriptorSetLayoutBinding vpLayoutBinding = 
	//	oGFX::vk::inits::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 0);
	//
	//std::vector<VkDescriptorSetLayoutBinding> layoutBindings = { vpLayoutBinding/*, modelLayoutBinding*/ };
	//
	//// Create Descriptor Set Layout with given bindings
	//VkDescriptorSetLayoutCreateInfo layoutCreateInfo = 
	//	oGFX::vk::inits::descriptorSetLayoutCreateInfo(layoutBindings.data(),static_cast<uint32_t>(layoutBindings.size()));		
	//
	//																				// Create descriptor set layout
	//VkResult result = vkCreateDescriptorSetLayout(m_device.logicalDevice, &layoutCreateInfo, nullptr, &descriptorSetLayout);
	//if (result != VK_SUCCESS)
	//{
	//	throw std::runtime_error("Failed to create a descriptor set layout!");
	//}

	// CREATE TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
	// Texture binding info
	VkDescriptorSetLayoutBinding samplerLayoutBinding =
		oGFX::vkutils::inits::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, MAX_OBJECTS);

	VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT 
		| VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 
		| VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	VkDescriptorSetLayoutBindingFlagsCreateInfo flaginfo{};
	flaginfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	flaginfo.pBindingFlags = &flags;
	flaginfo.bindingCount = 1;

	// create a descriptor set layout with given bindings for texture
	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo =
		oGFX::vkutils::inits::descriptorSetLayoutCreateInfo(&samplerLayoutBinding, 1);
	textureLayoutCreateInfo.pNext = &flaginfo;
	textureLayoutCreateInfo.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	VkResult result = vkCreateDescriptorSetLayout(m_device.logicalDevice, &textureLayoutCreateInfo, nullptr, &SetLayoutDB::bindless);
	VK_NAME(m_device.logicalDevice, "samplerSetLayout", SetLayoutDB::bindless);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Failed to create a descriptor set layout!" << std::endl;
		__debugbreak();
	}
}

void VulkanRenderer::FullscreenBlit(VkCommandBuffer inCmd, vkutils::Texture& src, VkImageLayout srcFinal, vkutils::Texture& dst, VkImageLayout dstFinal) 
{	
	const VkCommandBuffer cmdlist = inCmd;
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("Blit");

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = { 0.0f,0.0f,0.0f,0.0f };

	glm::uvec2 renderSize = glm::vec2{ dst.width,dst.height };

	VkDescriptorImageInfo texdesc = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_SSAOEdgeClamp(),
		src.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	rhi::CommandList cmd{ cmdlist ,"Fullscreen Blit"};

	cmd.BindAttachment(0, &dst);


	cmd.SetDefaultViewportAndScissor();
	std::array<VkViewport, 1>viewports{ VkViewport{0,renderSize.y * 1.0f,renderSize.x * 1.0f,renderSize.y * -1.0f} };
	cmd.SetViewport(0, static_cast<uint32_t>(viewports.size()), viewports.data());
	VkRect2D scissor{ {}, {renderSize.x,renderSize.y} };
	cmd.SetScissor(scissor);

	VkDescriptorImageInfo sampler = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		VK_NULL_HANDLE,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	cmd.BindPSO(pso_utilFullscreenBlit, PSOLayoutDB::fullscreenBlitPSOLayout);

	// create descriptor for this pass
	cmd.DescriptorSetBegin(0)
		.BindSampler(0, GfxSamplerManager::GetDefaultSampler())
		.BindImage(1, &src, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);


	SSAOPC pc{};
	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(SSAOPC);

	cmd.SetPushConstant(PSOLayoutDB::fullscreenBlitPSOLayout, range, &pc);

	uint32_t dynamicOffset = static_cast<uint32_t>(renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO),
		m_device.properties.limits.minUniformBufferOffsetAlignment));

	cmd.DrawFullScreenQuad();
}

void VulkanRenderer::BlitFramebuffer(VkCommandBuffer cmd, vkutils::Texture& src,VkImageLayout srcFinal, vkutils::Texture& dst,VkImageLayout dstFinal)
{
	bool supportsBlit = true;

	VkFormatProperties formatProps;

	// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
	vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice, src.format, &formatProps);
	if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
		//std::cerr << "Device does not support blitting from optimal tiled images, using copy instead of blit!" << std::endl;
		supportsBlit = false;
	}

	// Check if the device supports blitting to linear images
	vkGetPhysicalDeviceFormatProperties(m_device.physicalDevice, dst.format, &formatProps);
	if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
		//std::cerr << "Device does not support blitting to linear tiled images, using copy instead of blit!" << std::endl;
		supportsBlit = false;
	}

	// Source for the copy is the last rendered swapchain image

	// Transition destination image to transfer destination layout
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmd,
		dst.image.image,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		dst.currentLayout,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmd,
		src.image.image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		src.currentLayout, // DO PROPER RESOURCE TRACKING
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
	if (supportsBlit)
	{
		// Define the region to blit (we will blit the whole swapchain image)
		VkOffset3D srcBlitSize;
		srcBlitSize.x = src.width;
		srcBlitSize.y = src.height;
		srcBlitSize.z = 1;

		VkOffset3D dstBlitSize;
		dstBlitSize.x = dst.width;
		dstBlitSize.y = dst.height;
		dstBlitSize.z = 1;
		VkImageBlit imageBlitRegion{};
		imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlitRegion.srcSubresource.layerCount = 1;
		imageBlitRegion.srcOffsets[1] = srcBlitSize;
		imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlitRegion.dstSubresource.layerCount = 1;
		imageBlitRegion.dstOffsets[1] = dstBlitSize;

		// Issue the blit command
		vkCmdBlitImage(
			cmd,
			src.image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dst.image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageBlitRegion,
			VK_FILTER_NEAREST);
	}
	else
	{
		// Otherwise use image copy (requires us to manually flip components)
		VkImageCopy imageCopyRegion{};
		imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.srcSubresource.layerCount = 1;
		imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.dstSubresource.layerCount = 1;
		imageCopyRegion.extent.width = dst.width;
		imageCopyRegion.extent.height = dst.height;
		imageCopyRegion.extent.depth = 1;

		// Issue the copy command
		vkCmdCopyImage(
			cmd,
			src.image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dst.image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageCopyRegion);
	}

	// Transition destination image to general layout, which is the required layout for mapping the image memory later on
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmd,
		dst.image.image,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		dstFinal,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	// Transition back the swap chain image after the blit is done
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmd,
		src.image.image,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		srcFinal,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	dst.currentLayout = dstFinal;
	src.currentLayout = srcFinal;
}

void VulkanRenderer::CreateDefaultPSOLayouts()
{	
	std::array<VkDescriptorSetLayout,4> descriptorSetLayouts = 
	{
		SetLayoutDB::gpuscene, // (set = 0)
		SetLayoutDB::FrameUniform,  // (set = 1)
		SetLayoutDB::bindless,  // (set = 2)
		SetLayoutDB::lights, // (set = 3)
	};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = oGFX::vkutils::inits::pipelineLayoutCreateInfo(descriptorSetLayouts);
	
	VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &PSOLayoutDB::defaultPSOLayout));
	VK_NAME(m_device.logicalDevice, "defaultPSOLayout", PSOLayoutDB::defaultPSOLayout);
	
	//create dummy for desciptorlayout
	VkDescriptorImageInfo basicSampler = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		0,
		VK_IMAGE_LAYOUT_UNDEFINED);
	DescriptorBuilder::Begin()
		.BindImage(0, nullptr, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BindImage(1, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL_GRAPHICS)
		.BuildLayout(SetLayoutDB::util_fullscreenBlit);

	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &SetLayoutDB::util_fullscreenBlit;
	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &PSOLayoutDB::fullscreenBlitPSOLayout));
	VK_NAME(m_device.logicalDevice, "fullscreenPSOLayout", PSOLayoutDB::fullscreenBlitPSOLayout);
	
	
	DescriptorBuilder::Begin()
		.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3000, gpuTransformBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2002, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT,13)
		.BindImage(2001, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(2000, objectInformationBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_AMDSPD);

	// create compute here
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &SetLayoutDB::compute_AMDSPD;
	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &PSOLayoutDB::AMDSPDPSOLayout));
	VK_NAME(m_device.logicalDevice, "AMDSPD_PSOLayout", PSOLayoutDB::AMDSPDPSOLayout);

	DescriptorBuilder::Begin()
		.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_Radiance);

	// create compute here
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &SetLayoutDB::compute_Radiance;
	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &PSOLayoutDB::RadiancePSOLayout));
	VK_NAME(m_device.logicalDevice, "Radiance_PSOLayout", PSOLayoutDB::RadiancePSOLayout);

	DescriptorBuilder::Begin()
		.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(1, nullptr, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_prefilter);

	// create compute here
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &SetLayoutDB::compute_prefilter;
	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &PSOLayoutDB::prefilterPSOLayout));
	VK_NAME(m_device.logicalDevice, "prefilterPSOLayout", PSOLayoutDB::prefilterPSOLayout);


	DescriptorBuilder::Begin()
		.BindImage(2, nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BuildLayout(SetLayoutDB::compute_brdfLUT);

	// create compute here
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &SetLayoutDB::compute_brdfLUT;
	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &PSOLayoutDB::BRDFLUTPSOLayout));
	VK_NAME(m_device.logicalDevice, "BRDFLUTPSOLayout", PSOLayoutDB::BRDFLUTPSOLayout);
}

void VulkanRenderer::CreateDefaultPSO()
{
	
	const char* shaderVS = "Shaders/bin/genericFullscreen.vert.spv";
	const char* shaderPS = "Shaders/bin/Blit.frag.spv";
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages
	{
		LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT , VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, this->G_DEPTH_COMPARISON);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::fullscreenBlitPSOLayout, renderPass_default_noDepth.pass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	// Empty vertex input state, vertices are generated by the vertex shader
	VkPipelineVertexInputStateCreateInfo emptyInputState = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo();
	pipelineCI.pVertexInputState = &emptyInputState;
	//pipelineCI.renderPass = renderPass_default_noDepth.pass;
	pipelineCI.renderPass = VK_NULL_HANDLE;

	VkFormat format = m_swapchain.swapChainImageFormat;
	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.viewMask = {};
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats =&format;
	renderingInfo.depthAttachmentFormat = G_DEPTH_FORMAT;
	renderingInfo.stencilAttachmentFormat = G_DEPTH_FORMAT;

	pipelineCI.pNext = &renderingInfo;
	
	pipelineCI.layout = PSOLayoutDB::fullscreenBlitPSOLayout;
	colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	
	if (pso_utilFullscreenBlit != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_utilFullscreenBlit, nullptr);
	}
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_utilFullscreenBlit));
	VK_NAME(m_device.logicalDevice, "pso_blit", pso_utilFullscreenBlit);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr); // destroy vert
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr); // destroy fragment
	
	
	if (pso_utilAMDSPD != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_utilAMDSPD, nullptr); 
	}
	const char* computeShader = "Shaders/bin/ffx_spd_downsample_pass.glsl.spv";
	VkComputePipelineCreateInfo computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::AMDSPDPSOLayout);
	computeCI.stage = LoadShader(m_device, computeShader, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_utilAMDSPD));
	VK_NAME(m_device.logicalDevice, "pso_AMDSPD", pso_utilAMDSPD);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr);

	if (pso_radiance != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_radiance, nullptr); 
	}
	const char* radianceShader = "Shaders/bin/irradiance.comp.spv";
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::RadiancePSOLayout);
	computeCI.stage = LoadShader(m_device, radianceShader, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_radiance));
	VK_NAME(m_device.logicalDevice, "pso_Radiance", pso_radiance);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr);

	if (pso_prefilter != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_prefilter, nullptr);
	}
	const char* prefilterShader = "Shaders/bin/envPrefilter.comp.spv";
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::prefilterPSOLayout);
	computeCI.stage = LoadShader(m_device, prefilterShader, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_prefilter));
	VK_NAME(m_device.logicalDevice, "pso_prefilter", pso_prefilter);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr);

	if (pso_brdfLUT != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(m_device.logicalDevice, pso_brdfLUT, nullptr);
	}
	const char* lutShader = "Shaders/bin/brdfLUT.comp.spv";
	computeCI = oGFX::vkutils::inits::computeCreateInfo(PSOLayoutDB::BRDFLUTPSOLayout);
	computeCI.stage = LoadShader(m_device, lutShader, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHK(vkCreateComputePipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &computeCI, nullptr, &pso_brdfLUT));
	VK_NAME(m_device.logicalDevice, "pso_brdfLUT", pso_brdfLUT);
	vkDestroyShaderModule(m_device.logicalDevice, computeCI.stage.module, nullptr);
	
	
}

void VulkanRenderer::CreateDebugCallback()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance.instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		VK_CHK(func(m_instance.instance, &createInfo, nullptr, &m_debugMessenger));
	}
}

void VulkanRenderer::CreateFramebuffers()
{
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(m_device.logicalDevice, swapChainFramebuffers[i], nullptr);
	}

	//resize framebuffer count to equal swapchain image count
	swapChainFramebuffers.resize(m_swapchain.swapChainImages.size());

	//create a frame buffer for each swapchain image
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			m_swapchain.swapChainImages[i].view,
			m_swapchain.depthAttachment.view
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass_default.pass; //render pass layout the frame buffer will be used with
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data(); //list of attachments (1:1 with render pass)
		framebufferCreateInfo.width = m_swapchain.swapChainExtent.width;
		framebufferCreateInfo.height = m_swapchain.swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(m_device.logicalDevice, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]);
		VK_NAME(m_device.logicalDevice, "swapchainFramebuffers", swapChainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Failed to create a Framebuffer!" << std::endl;
			__debugbreak();
		}
	}
}

void VulkanRenderer::DestroyDebugMessenger()
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instance.instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(m_instance.instance, m_debugMessenger, nullptr);
	}
}

void VulkanRenderer::CreateCommandBuffers()
{
	// resize command buffers count to have one for each frame buffer
	//commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// VK_COMMAND_BUFFER_LEVEL_PRIMARY : buffer you submit directly to queue, cant be called  by other buffers
															//VK_COMMAND_BUFFER_LEVEL_SECONDARY :  buffer cant be called directly, can be called from other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = 1;

	//for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	//{
	//	cbAllocInfo.commandPool = m_device.commandPools[i];
	//	//allocate command buffers and place handles in array of buffers
	//	VkResult result = vkAllocateCommandBuffers(m_device.logicalDevice, &cbAllocInfo, &commandBuffers[i]);
	//	if (result != VK_SUCCESS)
	//	{
	//		std::cerr << "Failed to allocate Command Buffers!" << std::endl;
	//		__debugbreak();
	//	}
	//}
	
}

VkCommandBuffer VulkanRenderer::GetCommandBuffer()
{
	constexpr bool beginBuffer = true;
	VkCommandBuffer result = m_device.commandPoolManagers[getFrame()].GetNextCommandBuffer(beginBuffer);
	VK_NAME(m_device.logicalDevice, "DEFAULTCMD", result);
	return result;
}

void VulkanRenderer::SubmitSingleCommandAndWait(VkCommandBuffer cmd)
{
	m_device.commandPoolManagers[getFrame()].SubmitCommandBufferAndWait(m_device.graphicsQueue, cmd);	
}

void VulkanRenderer::SubmitSingleCommand(VkCommandBuffer cmd)
{
	m_device.commandPoolManagers[getFrame()].SubmitCommandBuffer(m_device.graphicsQueue, cmd);
}

void VulkanRenderer::SetWorld(GraphicsWorld* world)
{
	auto lam = [this,w = world]() {
		currWorld = w;
	};
	std::scoped_lock l{g_mut_workQueue};
	g_workQueue.emplace_back(lam);
}

void VulkanRenderer::InitWorld(GraphicsWorld* world)
{
	assert(world && "dont pass nullptr");
	auto lam = [this,w = world]() {
		for (uint32_t x = 0; x < w->numCameras; ++x)
		{
			auto& wrdID = w->targetIDs[x];
			if (wrdID == -1)
			{
				// allocate render target
				bool found = false;
				for (size_t i = 0; i < renderTargets.size(); i++)
				{

					if (renderTargets[i].inUse == false)
					{
						numAllocatedCameras++;
						renderTargets[i].inUse = true;
						wrdID = static_cast<int32_t>(i);
						found = true;
						break;
					}
				}
				assert(found && "Could not find enough rendertargets");
				// initialization
				auto& image = renderTargets[wrdID].texture;
				if (image.image.image == VK_NULL_HANDLE)
				{
					image.name = "GW_"+std::to_string(wrdID)+":COL";
					image.forFrameBuffer(&m_device, G_NON_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
						m_swapchain.swapChainExtent.width,m_swapchain.swapChainExtent.height);
					fbCache.RegisterFramebuffer(image);
					auto cmd = GetCommandBuffer();
					vkutils::SetImageInitialState(cmd, image);
					SubmitSingleCommandAndWait(cmd);
				}
				if (image.image.image && renderTargets[wrdID].imguiTex == 0)
				{
					renderTargets[wrdID].imguiTex = CreateImguiBinding(samplerManager.GetDefaultSampler(), &image);				
				}
				auto& depth =  renderTargets[wrdID].depth;
				if (depth.image.image == VK_NULL_HANDLE)
				{
					depth.name = "GW_"+std::to_string(wrdID)+":DEPTH";
					depth.forFrameBuffer(&m_device, G_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
						m_swapchain.swapChainExtent.width,m_swapchain.swapChainExtent.height);
					fbCache.RegisterFramebuffer(depth);		

					auto cmd = GetCommandBuffer();
					vkutils::SetImageInitialState(cmd, depth);
					SubmitSingleCommandAndWait(cmd);

					//world->imguiID[0] = CreateImguiBinding(samplerManager.GetDefaultSampler(), depth.view, depth.imageLayout);
				}

				//assignment 
				w->imguiID [x] = renderTargets[wrdID].imguiTex;
			}		
		}	
	};
	world->initialized = true;
	std::scoped_lock l{g_mut_workQueue};
	g_workQueue.emplace_back(lam);
}

void VulkanRenderer::DestroyWorld(GraphicsWorld* world)
{
	assert(world && "dont pass nullptr");
	assert(world->initialized && "World should exist dont destroy non-init world");
	

	auto lam = [this,w = world]() {
		for (uint32_t x = 0; x < w->numCameras; ++x)
		{
			auto& wrdID = w->targetIDs[x];
			renderTargets[wrdID].inUse = false;
			wrdID = -1;
			numAllocatedCameras--;
		}	
		w->initialized = false;
	};
	std::scoped_lock l{g_mut_workQueue};
	g_workQueue.emplace_back(lam);
}

int32_t VulkanRenderer::GetPixelValue(uint32_t fbID, glm::vec2 uv)
{
	//return 0;

	uv = glm::clamp(uv, { 0.0,0.0 }, { 1.0,1.0 });

	// Bad but only editor uses this
	auto& physicalDevice = m_device.physicalDevice;
	auto& device = m_device.logicalDevice;
	vkQueueWaitIdle(m_device.graphicsQueue);
	vkDeviceWaitIdle(device);

	
	auto& target = attachments.gbuffer[GBufferAttachmentIndex::ENTITY_ID];
	if (target.currentLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		return -1;

	VkCommandBuffer copyCmd = beginSingleTimeCommands();
	VK_NAME(device, "COPY_DST_EDITOR_ID_CMD_LIST", copyCmd);

	// Source for the copy is the last rendered swapchain image
	VkImage srcImage = target.image.image;

	// Create the linear tiled destination image to copy to and to read the memory from
	VkImageCreateInfo imageCreateCI(oGFX::vkutils::inits::imageCreateInfo());
	imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
	// Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
	imageCreateCI.format = VK_FORMAT_R32_SINT;
	imageCreateCI.extent.width = target.width;
	imageCreateCI.extent.height = target.height;
	imageCreateCI.extent.depth = 1;
	imageCreateCI.arrayLayers = 1;
	imageCreateCI.mipLevels = 1;
	imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
	imageCreateCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	// Create the image

	oGFX::AllocatedImage dstImage{};
	VmaAllocationCreateInfo allocCI{};
	allocCI.usage = VMA_MEMORY_USAGE_AUTO;
	allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	vmaCreateImage(m_device.m_allocator, &imageCreateCI, &allocCI, &dstImage.image, &dstImage.allocation, &dstImage.allocationInfo);
	vkutils::Texture temptex;
	temptex.width = target.width;
	temptex.height = target.height;
	temptex.format = target.format;
	temptex.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	temptex.image = dstImage;

	// Do the actual blit from the swapchain image to our host visible destination image
	BlitFramebuffer(copyCmd, target, target.currentLayout, temptex, VK_IMAGE_LAYOUT_GENERAL);

	vkEndCommandBuffer(copyCmd);

	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd; 

	vkQueueSubmit(m_device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device.graphicsQueue);
	vkFreeCommandBuffers(m_device.logicalDevice, m_device.commandPoolManagers[getFrame()].m_commandpool, 1, &copyCmd);


	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout subResourceLayout;
	vkGetImageSubresourceLayout(device, dstImage.image, &subResource, &subResourceLayout);

	const char* mappedData = nullptr;
	auto result = vmaMapMemory(m_device.m_allocator, dstImage.allocation, (void**)&mappedData);
	if (result != VK_SUCCESS)
	{
		assert(false);
	}
	
	mappedData += subResourceLayout.offset;
	glm::uvec2 pixels = glm::uvec2{ target.width * uv.x,target.height * (uv.y) };
	pixels = glm::clamp(pixels, { 0,0 }, { target.width-1,target.height-1 });
	uint32_t indx = (pixels.x + pixels.y * target.width);
	int32_t value = ((int32_t*)mappedData)[indx];
	//uint32_t value = ((uint32_t*)data)[pixels.x * (pixels.y * subResourceLayout.rowPitch)];

	// Clean up resources
	vmaUnmapMemory(m_device.m_allocator, dstImage.allocation);
	vmaDestroyImage(m_device.m_allocator, dstImage.image, dstImage.allocation);

	return value;

}

void VulkanRenderer::CreateLightingBuffers()
{
	//oGFX::CreateBuffer(m_device.physicalDevice, m_device.logicalDevice, sizeof(CB::LightUBO), 
	//	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	//	&lightsBuffer.buffer, &lightsBuffer.memory);
	//lightsBuffer.size = sizeof(CB::LightUBO);
	//lightsBuffer.device = m_device.logicalDevice;
	//lightsBuffer.descriptor.buffer = lightsBuffer.buffer;
	//lightsBuffer.descriptor.offset = 0;
	//lightsBuffer.descriptor.range = sizeof(CB::LightUBO);
	//
	//VK_CHK(lightsBuffer.map());
}

void VulkanRenderer::UploadLights()
{
	if (currWorld == nullptr)
		return;

	assert(currWorld->initialized && "World not initialized - did you call VulkanRenderer::InitWorld?");

	PROFILE_SCOPED();

	//CB::LightUBO lightUBO{};
	//
	//// Current view position
	//lightUBO.viewPos = glm::vec4(camera.m_position, 0.0f);
	//
	//// Temporary reroute
	//auto& allLights = currWorld->GetAllOmniLightInstances();
	//
	//// Gather lights to be uploaded.
	//// TODO: Frustum culling for light bounding volume...
	//int numLights = glm::clamp((int)allLights.size(), 0, 6);
	//for (int i = 0; i < numLights; ++i)
	//{
	//	lightUBO.lights[i] = allLights[i];
	//}
	//
	//// Only lights that are inside/intersecting the camera frustum should be uploaded.
	//memcpy(lightsBuffer.mapped, &lightUBO, sizeof(CB::LightUBO));

	const auto& spotLights = batches.GetLocalLights();
	m_numShadowcastLights = batches.m_numShadowcastLights;
	auto cmd = GetCommandBuffer();
	globalLightBuffer[getFrame()].writeToCmd(spotLights.size(), spotLights.data(), cmd, m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);

}

void VulkanRenderer::UploadBones()
{

}

void VulkanRenderer::CreateSynchronisation()
{
	presentSemaphore.resize(MAX_FRAME_DRAWS);
	renderSemaphore.resize(MAX_FRAME_DRAWS);

	drawFences.resize(MAX_FRAME_DRAWS);
	//Semaphore creation information
	VkSemaphoreCreateInfo semaphorecreateInfo = {};
	semaphorecreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	//fence creating information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreTypeCreateInfo timelineCreateInfo{};
	timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	timelineCreateInfo.pNext = NULL;
	timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	timelineCreateInfo.initialValue = 0;

	VkSemaphoreCreateInfo sci{};
	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	sci.pNext = &timelineCreateInfo;
	sci.flags = 0;
	VK_CHK(vkCreateSemaphore(m_device.logicalDevice, &sci, nullptr, &frameCountSemaphore));

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(m_device.logicalDevice, &semaphorecreateInfo, nullptr, &presentSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device.logicalDevice, &semaphorecreateInfo, nullptr, &renderSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(m_device.logicalDevice, &fenceCreateInfo, nullptr,&drawFences[i]) != VK_SUCCESS)
		{
			std::cerr << "Failed to create a Semaphore and/or Fence!" << std::endl;
			__debugbreak();
		}
		VK_NAME(m_device.logicalDevice, "presentSemaphore", presentSemaphore[i]);
		VK_NAME(m_device.logicalDevice, "renderSemaphore", renderSemaphore[i]);
		VK_NAME(m_device.logicalDevice, "drawFences", drawFences[i]);
	}
}

void VulkanRenderer::CreateUniformBuffers()
{	
	// ViewProjection buffer size
	//auto dynamicAlignment = sizeof(glm::mat4);
	uboDynamicAlignment = oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO),m_device.properties.limits.minUniformBufferOffsetAlignment);
	
	VkDeviceSize vpBufferSize = numCameras*uboDynamicAlignment;

	//// LightData bufffer size
	//VkDeviceSize modelBufferSize = modelUniformAlignment * MAX_OBJECTS;

	// One uniform buffer for each image (and by extension, command buffer)
	vpUniformBuffer.resize(m_swapchain.swapChainImages.size());
	//modelDUniformBuffer.resize(swapChainImages.size());
	//modelDUniformBufferMemory.resize(swapChainImages.size());

	//create uniform buffers
	for (size_t i = 0; i < m_swapchain.swapChainImages.size(); i++)
	{
		oGFX::CreateBuffer(m_device.m_allocator, vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
			, vpUniformBuffer[i]);
		/*createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelDUniformBuffer[i], &modelDUniformBufferMemory[i]);*/
	}
}

void VulkanRenderer::CreateDescriptorPool()
{
	// CREATE UNIFORM DESCRIPTOR POOL
	// Type of descriptors + how many DESCRIPTORS, not DESCRIPTOR_SETS (combined makes the pool size)

	// ViewProjection pool
	VkDescriptorPoolSize vpPoolsize = oGFX::vkutils::inits::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(vpUniformBuffer.size()));
	VkDescriptorPoolSize attachmentPool = oGFX::vkutils::inits::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);

	//list of pool sizes
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolsize,attachmentPool /*, modelPoolSize*/ };

	//data to create the descriptor pool
	VkDescriptorPoolCreateInfo poolCreateInfo = oGFX::vkutils::inits::descriptorPoolCreateInfo(descriptorPoolSizes,static_cast<uint32_t>(m_swapchain.swapChainImages.size()+1));
	//create descriptor pool
	VkResult result = vkCreateDescriptorPool(m_device.logicalDevice, &poolCreateInfo, nullptr, &descriptorPool);
	VK_NAME(m_device.logicalDevice, "descriptorPool", descriptorPool);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Failed to create a descriptor pool!" << std::endl;
		__debugbreak();
	}

	// Create Sampler Descriptor pool
	// Texture sampler pool
	VkDescriptorPoolSize samplerPoolSize = oGFX::vkutils::inits::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_OBJECTS);// or MAX_OBJECTS?
	std::vector<VkDescriptorPoolSize> samplerpoolSizes = { samplerPoolSize };

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = oGFX::vkutils::inits::descriptorPoolCreateInfo(samplerpoolSizes,1); // or MAX_OBJECTS?
	samplerPoolCreateInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	result = vkCreateDescriptorPool(m_device.logicalDevice, &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool);
	VK_NAME(m_device.logicalDevice, "samplerDescriptorPool", samplerDescriptorPool);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Failed to create a descriptor pool!" << std::endl;
		__debugbreak();
	}

	// Variable descriptor
	VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variableDescriptorCountAllocInfo = {};

	uint32_t variableDescCounts[] = { MAX_OBJECTS };
	variableDescriptorCountAllocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
	variableDescriptorCountAllocInfo.descriptorSetCount = 1;
	variableDescriptorCountAllocInfo.pDescriptorCounts = variableDescCounts;

	//Descriptor set allocation info
	VkDescriptorSetAllocateInfo setAllocInfo = oGFX::vkutils::inits::descriptorSetAllocateInfo(samplerDescriptorPool,&SetLayoutDB::bindless,1);
	setAllocInfo.pNext = &variableDescriptorCountAllocInfo;

	//Allocate our descriptor sets
	result = vkAllocateDescriptorSets(m_device.logicalDevice, &setAllocInfo, &descriptorSet_bindless);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Failed to allocate texture descriptor sets!" << std::endl;
		__debugbreak();
	}
}

void VulkanRenderer::CreateDescriptorSets_GPUScene()
{
	VkDescriptorBufferInfo info{};
	info.buffer = gpuTransformBuffer[getFrame()].getBuffer();
	info.offset = 0;
	info.range = VK_WHOLE_SIZE;

	VkDescriptorImageInfo basicSampler = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetDefaultSampler(),
		0,
		VK_IMAGE_LAYOUT_UNDEFINED);

	DescriptorBuilder::Begin()
		.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.BindBuffer(3, gpuTransformBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.BindBuffer(4, gpuBoneMatrixBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.BindBuffer(5, objectInformationBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.BindBuffer(6, gpuSkinningBoneWeightsBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(descriptorSet_gpuscene,SetLayoutDB::gpuscene);
}

void VulkanRenderer::CreateDescriptorSets_Lights()
{
	VkDescriptorBufferInfo info{};
	info.buffer = globalLightBuffer[getFrame()].getBuffer();
	info.offset = 0;
	info.range = VK_WHOLE_SIZE;

	for (size_t i = 0; i < m_swapchain.swapChainImages.size(); i++)
	{
		DescriptorBuilder::Begin()
			.BindBuffer(4, &info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.Build(descriptorSet_lights,SetLayoutDB::lights);
	}
	
}

void VulkanRenderer::InitImGUI()
{
	if (m_imguiInitialized) return;	

	VkAttachmentDescription attachment = {};
	attachment.format = m_swapchain.swapChainImageFormat;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // Draw GUI on what exitst
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//attachment.initialLayout = m_swapchain.swapChainImages[swapchainIdx].currentLayout;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// TODO: make sure we set the previous renderpass to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL 
	// since this will be the final pass (before presentation) instead
#if OO_END_PRODUCT
	attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //image data layout aftet render pass ( to change to)
#else
#endif // OO_END_PRODUCT
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // final layout for presentation.

	VkAttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // create dependancy outside current renderpass
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // make sure pixels have been fully rendered before performing this pass
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // same thing
	dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;


	if (vkCreateRenderPass(m_device.logicalDevice, &info, nullptr, &m_imguiConfig.renderPass) != VK_SUCCESS) 
	{
		std::cerr << "Could not create Dear ImGui's render pass" << std::endl;
		__debugbreak();
	}
	VK_NAME(m_device.logicalDevice, "imguiConfig_renderpass", m_imguiConfig.renderPass);

	std::vector<VkDescriptorPoolSize> pool_sizes
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo dpci = oGFX::vkutils::inits::descriptorPoolCreateInfo(pool_sizes,1000);
	dpci.flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	vkCreateDescriptorPool(m_device.logicalDevice, &dpci, nullptr, &m_imguiConfig.descriptorPools);
	VK_NAME(m_device.logicalDevice, "imguiConfig_descriptorPools", m_imguiConfig.descriptorPools);

	unsigned char* pixels = nullptr;
	int width, height;
	ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	std::vector<VkBufferImageCopy> mips;
	VkDeviceSize buffSz = width * height * 4;
	VkBufferImageCopy imageRegion{};
	imageRegion.bufferOffset = 0;											// Offset into data
	imageRegion.bufferRowLength = 0;										// Row length of data to calculate data spacing
	imageRegion.bufferImageHeight = 0;										// Image height to calculate data spacing
	imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Which aspect of image to copy
	imageRegion.imageSubresource.mipLevel = 0;								// Mipmap Level to copy
	imageRegion.imageSubresource.baseArrayLayer = 0;						// Starting array layer (if array)
	imageRegion.imageSubresource.layerCount = 1;							// Number of layers to copy starting at baseArray layer
	imageRegion.imageOffset = { 0,0,0 };									//	Offset into image (as opposed to raw data in buffer offset)
	imageRegion.imageExtent = { (uint32_t)width,(uint32_t)height, 1 };		//  Size of region to copy as XYZ values
	mips.push_back(imageRegion);
	
	g_imguiFont.fromBuffer(pixels, buffSz, VK_FORMAT_R8G8B8A8_UNORM, width, height, mips, &m_device, m_device.graphicsQueue);

	m_imguiInitialized = true;
	PerformImguiRestart();

	memcpy(&s_imguiSharedData, ImGui::GetDrawListSharedData(), sizeof(ImDrawListSharedData));



	struct BackendData {
		ImGui_ImplVulkan_InitInfo   VulkanInitInfo;
		VkRenderPass                RenderPass;
		VkDeviceSize                BufferMemoryAlignment;
		VkPipelineCreateFlags       PipelineCreateFlags;
		VkDescriptorSetLayout       DescriptorSetLayout;
		VkPipelineLayout            PipelineLayout;
		VkPipeline                  Pipeline;
		uint32_t                    Subpass;
		VkShaderModule              ShaderModuleVert;
		VkShaderModule              ShaderModuleFrag;

		// Font data
		VkSampler                   FontSampler;
		VkDeviceMemory              FontMemory;
		VkImage                     FontImage;
		VkImageView                 FontView;
		VkDescriptorSet				FontDescriptorSet;
	};
	BackendData* bd = (BackendData*)(ImGui::GetIO().BackendRendererUserData);
	g_imguiToTexture[bd->FontDescriptorSet] = &g_imguiFont;


	// Create frame buffers for every swap chain image
	// We need to do this because ImGUI only cares about the colour attachment.
	std::array<VkImageView, 2> fbattachments{};
	VkFramebufferCreateInfo _ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	_ci.renderPass      = m_imguiConfig.renderPass;
	_ci.width           = m_swapchain.swapChainExtent.width;
	_ci.height          =  m_swapchain.swapChainExtent.height;
	_ci.layers          = 1;
	_ci.attachmentCount = 1;
	_ci.pAttachments    = fbattachments.data();

	// Each of the three swapchain images gets an associated frame
	// buffer, all sharing one depth buffer.
	m_imguiConfig.buffers.resize(m_swapchain.swapChainImages.size());
	for(uint32_t i = 0; i < m_swapchain.swapChainImages.size(); i++) 
	{
		// TODO make sure all images resize for imgui
		fbattachments[0] = m_swapchain.swapChainImages[i].view;         // A color attachment from the swap chain
													//fbattachments[1] = m_depthImage.imageView;  // A depth attachment
		VK_CHK(vkCreateFramebuffer(m_device.logicalDevice, &_ci, nullptr, &m_imguiConfig.buffers[i])); 
		VK_NAME(m_device.logicalDevice, "imguiconfig_Framebuffer", m_imguiConfig.buffers[i]);
	}

}

void VulkanRenderer::ResizeGUIBuffers()
{
	for(uint32_t i = 0; i < m_imguiConfig.buffers.size(); i++) 
	{      
		vkDestroyFramebuffer(m_device.logicalDevice, m_imguiConfig.buffers[i], nullptr);
	}
	// Create frame buffers for every swap chain image
	// We need to do this because ImGUI only cares about the colour attachment.
	std::array<VkImageView, 2> fbattachments{};
	VkFramebufferCreateInfo _ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
	_ci.renderPass      = m_imguiConfig.renderPass;
	_ci.width           = m_swapchain.swapChainExtent.width;
	_ci.height          =  m_swapchain.swapChainExtent.height;
	_ci.layers          = 1;
	_ci.attachmentCount = 1;
	_ci.pAttachments    = fbattachments.data();
	m_imguiConfig.buffers.resize(m_swapchain.swapChainImages.size());

	for(uint32_t i = 0; i < m_swapchain.swapChainImages.size(); i++) 
	{
		// TODO make sure all images resize for imgui
		fbattachments[0] = m_swapchain.swapChainImages[i].view;         // A color attachment from the swap chain
																			 //fbattachments[1] = m_depthImage.imageView;  // A depth attachment
		VK_CHK(vkCreateFramebuffer(m_device.logicalDevice, &_ci, nullptr, &m_imguiConfig.buffers[i]));
		VK_NAME(m_device.logicalDevice, "imguiconfig_buffers", m_imguiConfig.buffers[i]);
	}
}

void VulkanRenderer::DebugGUIcalls()
{
	if(ImGui::Begin("img"))
	{
		if (currWorld) {
		const char* views[]  = { "Lookat", "FirstPerson" };
		ImGui::ListBox("Camera View", reinterpret_cast<int*>(&currWorld->cameras.front().m_CameraMovementType), views, 2);
		auto sz = ImGui::GetContentRegionAvail();
		ImGui::Image(myImg, { sz.x,sz.y });
		}
	}
	ImGui::End();
	
	if(ImGui::Begin("Deferred Rendering GBuffer"))
	{
		ImGui::Checkbox("Enable Deferred Rendering", &deferredRendering);
		if (deferredRendering)
		{
			const auto sz = ImGui::GetContentRegionAvail();
			auto deferredImg = Attachments_imguiBinding::deferredImg;
	
	
			const float renderWidth = float(windowPtr->m_width);
			const float renderHeight = float(windowPtr->m_height);
			const float aspectRatio = renderHeight / renderWidth;
			const ImVec2 imageSize = { sz.x, sz.x * aspectRatio };
	
			//auto gbuff = GBufferRenderPass::Get();
			//ImGui::BulletText("World Position");
			//ImGui::Image(gbuff->deferredImg[POSITION], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("World Normal");
			ImGui::Image(deferredImg[NORMAL], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("Albedo");
			ImGui::Image(deferredImg[ALBEDO], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("Material");
			ImGui::Image(deferredImg[MATERIAL], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("Depth (TODO)");
			//ImGui::Image(gbuff->deferredImg[3], { sz.x,sz.y/4 });
			ImGui::Image(Attachments_imguiBinding::shadowImg ,imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		}
	}
	ImGui::End();
}

void VulkanRenderer::DrawGUI()
{

	return;

	PROFILE_SCOPED();
	std::scoped_lock l{m_imguiShutdownGuard};
	if (m_imguiInitialized == false) return;

	VkRenderPassBeginInfo GUIpassInfo = {};
	GUIpassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	GUIpassInfo.renderPass  = m_imguiConfig.renderPass;
	GUIpassInfo.framebuffer = m_imguiConfig.buffers[swapchainIdx];
	GUIpassInfo.renderArea = { {0, 0}, {m_swapchain.swapChainExtent}};

    const VkCommandBuffer cmdlist = GetCommandBuffer();
	VK_NAME(m_device.logicalDevice, "IM_GUI_CMD", cmdlist);
	

	// temp hardcode until its in its own renderer
	vkutils::TransitionImage(cmdlist, m_swapchain.swapChainImages[swapchainIdx], m_swapchain.swapChainImages[swapchainIdx].referenceLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	for (size_t i = 0; i < renderTargets.size(); i++)
	{
		if (renderTargets[i].texture.image.image != VK_NULL_HANDLE)
		{
			vkutils::TransitionImage(cmdlist, renderTargets[i].texture, renderTargets[i].texture.referenceLayout, VK_IMAGE_LAYOUT_GENERAL);
		}
	}
	vkCmdBeginRenderPass(cmdlist, &GUIpassInfo, VK_SUBPASS_CONTENTS_INLINE);
	if (m_imguiDrawData.Valid) 
	{
		ImGui_ImplVulkan_RenderDrawData(&m_imguiDrawData, cmdlist);
	}
	vkCmdEndRenderPass(cmdlist);

	// Draw call done, invalidate old list

	for (size_t i = 0; i < renderTargets.size(); i++)
	{
		if (renderTargets[i].texture.image.image != VK_NULL_HANDLE)
		{
			vkutils::TransitionImage(cmdlist, renderTargets[i].texture, VK_IMAGE_LAYOUT_GENERAL, renderTargets[i].texture.referenceLayout);
		}
	}
	
	

	// set by final renderpass
	m_swapchain.swapChainImages[swapchainIdx].currentLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//std::cout << currentFrame << " DrawGui " << std::to_string(swapchainIdx) <<" " 
	//	<< oGFX::vkutils::tools::VkImageLayoutString(m_swapchain.swapChainImages[swapchainIdx].currentLayout) << std::endl;
}

void VulkanRenderer::ImguiSoftDestroy()
{
	vkDeviceWaitIdle(m_device.logicalDevice);
	ImGui_ImplVulkan_Shutdown();
	if (windowPtr->m_type == Window::WindowType::WINDOWS32)
	{
		ImGui_ImplWin32_Shutdown();
	}
}


void VulkanRenderer::SubmitImguiDrawList(ImDrawData* drawData)
{
	OO_ASSERT(drawData);

	// watch performance if bad change everything to vectors
	ImDrawData newimguiDrawData;
	std::vector<ImDrawList*> newimguiDrawList;

	newimguiDrawData.TotalVtxCount = drawData->TotalVtxCount;
	newimguiDrawData.TotalIdxCount = drawData->TotalIdxCount;
	newimguiDrawData.DisplayPos = drawData->DisplayPos;
	newimguiDrawData.DisplaySize = drawData->DisplaySize;
	newimguiDrawData.CmdListsCount = drawData->CmdListsCount;
	newimguiDrawData.FramebufferScale = ImVec2(1.0f, 1.0f);
	newimguiDrawData.OwnerViewport = drawData->OwnerViewport;

	newimguiDrawList.reserve(drawData->CmdListsCount);
	for (size_t i = 0; i < drawData->CmdListsCount; i++)
	{
		ImDrawList* myDrawList = drawData->CmdLists[i]->CloneOutput();
		myDrawList->_Data = &s_imguiSharedData;
		newimguiDrawList.emplace_back(myDrawList);
	}

	auto lam = [this
		,inDraw = std::move(newimguiDrawData)
		,inLists = std::move(newimguiDrawList)]() mutable // move vector all the way in
		{		
		// should have been handled by renderer
		InvalidateDrawLists();
		m_imguiDrawList = std::move(inLists);
		m_imguiDrawData.Valid = true;
		m_imguiDrawData.CmdLists = m_imguiDrawList.data();

		m_imguiDrawData.TotalVtxCount = inDraw.TotalVtxCount;
		m_imguiDrawData.TotalIdxCount = inDraw.TotalIdxCount;
		m_imguiDrawData.DisplayPos = inDraw.DisplayPos;
		m_imguiDrawData.DisplaySize = inDraw.DisplaySize;
		m_imguiDrawData.CmdListsCount = inDraw.CmdListsCount;
		m_imguiDrawData.FramebufferScale = ImVec2(1.0f, 1.0f);
		m_imguiDrawData.OwnerViewport = inDraw.OwnerViewport;
	};
	std::scoped_lock l{ g_mut_workQueue };
	g_workQueue.push_back(lam);
	
}

void VulkanRenderer::InvalidateDrawLists()
{
	for (size_t i = 0; i < m_imguiDrawList.size(); i++)
	{
		IM_DELETE(m_imguiDrawList[i]);
	}
	m_imguiDrawList.clear();
}

void VulkanRenderer::DestroyImGUI()
{
	if (m_imguiInitialized == false) return;
	std::scoped_lock l{ m_imguiShutdownGuard };

	vkDeviceWaitIdle(m_device.logicalDevice);

	s_imguiSharedData.Font = nullptr;
	g_imguiFont.destroy();

	for (size_t i = 0; i < m_imguiConfig.buffers.size(); i++)
	{
		vkDestroyFramebuffer(m_device.logicalDevice, m_imguiConfig.buffers[i], nullptr);
	}
	vkDestroyRenderPass(m_device.logicalDevice, m_imguiConfig.renderPass, nullptr);
	vkDestroyDescriptorPool(m_device.logicalDevice, m_imguiConfig.descriptorPools, nullptr);
	
	ImguiSoftDestroy();

	m_imguiInitialized = false;
}

void checkresult(VkResult checkresult)
{
	if (checkresult != VK_SUCCESS)
	{
		std::cout << oGFX::vkutils::tools::VkResultString(checkresult) << std::endl;
	}

}

void VulkanRenderer::RestartImgui()
{
	m_restartIMGUI = true;
}

void VulkanRenderer::PerformImguiRestart()
{
	if (windowPtr->m_type == Window::WindowType::WINDOWS32)
	{
		ImGui_ImplWin32_Init(windowPtr->GetRawHandle());
		//setup surface creator
		ImGui::GetPlatformIO().Platform_CreateVkSurface = ImGui_ImplWin32_CreateVkSurface;
	}
	else
	{

		if (ImGui::GetIO().BackendPlatformUserData == NULL)
		{
			std::cout << "Vulkan Imgui Error: you should handle the initialization of imgui::window_init before here"<< std::endl;
			assert(true);
		}		
	}
	ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
	//pio.Platform_CreateVkSurface = Win32SurfaceCreator;

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = m_instance.instance;
	init_info.PhysicalDevice = m_device.physicalDevice;
	init_info.Device = m_device.logicalDevice;
	init_info.QueueFamily = m_device.queueIndices.graphicsFamily;
	init_info.Queue = m_device.graphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = m_imguiConfig.descriptorPools;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = m_swapchain.minImageCount+1;
	init_info.ImageCount = static_cast<uint32_t>(m_swapchain.swapChainImages.size());
	init_info.CheckVkResultFn = VK_NULL_HANDLE; // can be used to handle the error checking
	init_info.CheckVkResultFn = checkresult; // can be used to handle the error checking

	ImGui_ImplVulkan_Init(&init_info, m_imguiConfig.renderPass);

	// This uploads the ImGUI font package to the GPU
	VkCommandBuffer command_buffer = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

	endSingleTimeCommands(command_buffer); 
	m_imguiInitialized = true;
}


void VulkanRenderer::InitializeRenderBuffers()
{
	// In this function, all global rendering related buffers should be initialized, ONCE.

	// Note: Moved here from VulkanRenderer::UpdateIndirectCommands
	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{

		indirectCommandsBuffer[i].Init(&m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );
		VK_NAME(m_device.logicalDevice, "Indirect Command Buffer", indirectCommandsBuffer[i].getBuffer()); 

		shadowCasterCommandsBuffer[i].Init(&m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT );
		shadowCasterCommandsBuffer[i].reserve(MAX_OBJECTS,m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);
		VK_NAME(m_device.logicalDevice, "Shadow Command Buffer", shadowCasterCommandsBuffer[i].getBuffer());

		// Note: Moved here from VulkanRenderer::UpdateInstanceData
		instanceBuffer[i].Init(&m_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT );
		VK_NAME(m_device.logicalDevice, "Instance Buffer", instanceBuffer[i].getBuffer());

		objectInformationBuffer[i].Init(&m_device,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		//objectInformationBuffer.reserve(MAX_OBJECTS);  
		VK_NAME(m_device.logicalDevice, "Object inforBuffer", objectInformationBuffer[i].getBuffer());

		constexpr uint32_t MAX_LIGHTS = 512;
		// TODO: Currently this is only for OmniLightInstance.
		// You should also support various light types such as spot lights, etc...

		globalLightBuffer[i].Init(&m_device,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		//globalLightBuffer.reserve(MAX_LIGHTS);
		VK_NAME(m_device.logicalDevice, "Light Buffer", globalLightBuffer[i].getBuffer());

		constexpr uint32_t MAX_GLOBAL_BONES = 2048;
		constexpr uint32_t MAX_SKINNING_VERTEX_BUFFER_SIZE = 4 * 1024 * 1024; // 4MB

		gpuBoneMatrixBuffer[i].Init(&m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		//gpuBoneMatrixBuffer.reserve(MAX_GLOBAL_BONES * sizeof(glm::mat4x4));
		VK_NAME(m_device.logicalDevice, "Bone Matrix Buffer", gpuBoneMatrixBuffer[i].getBuffer());

		

		g_particleCommandsBuffer[i].Init(&m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
		//g_particleCommandsBuffer.reserve(1024); // commands are generally per emitter. shouldnt have so many..

		g_UIVertexBufferGPU[i].Init(&m_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		g_UIIndexBufferGPU[i].Init(&m_device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}

	g_GlobalMeshBuffers.IdxBuffer.Init(&m_device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	g_GlobalMeshBuffers.VtxBuffer.Init(&m_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	//g_GlobalMeshBuffers.IdxBuffer.reserve(8 * 1000 * 1000);
	//g_GlobalMeshBuffers.VtxBuffer.reserve(1 * 1000 * 1000);

	gpuSkinningBoneWeightsBuffer.Init(&m_device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	//skinningVertexBuffer.reserve(MAX_SKINNING_VERTEX_BUFFER_SIZE);  
	VK_NAME(m_device.logicalDevice, "Skinning Weights Buffer", gpuSkinningBoneWeightsBuffer.getBuffer());
	

	for (size_t i = 0; i < g_particleDatas.size(); i++)
	{
		g_particleDatas[i].Init(&m_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		//g_particleDatas[i].reserve(100000*10); // 10 max particle systems
	}
	

	oGFX::CreateBuffer(m_device.m_allocator, sizeof(CB::AMDSPD_ATOMIC), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT| VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, SPDatomicBuffer);
	oGFX::CreateBuffer(m_device.m_allocator, sizeof(CB::AMDSPD_UBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, SPDconstantBuffer);
	
	const size_t STARTING_VERTEX_CNT = 5000;
	size_t vertex_size = STARTING_VERTEX_CNT * sizeof(ImDrawVert);
	size_t index_size = STARTING_VERTEX_CNT * sizeof(ImDrawIdx);
	size_t imguiCBsize = sizeof(glm::mat4);

	imguiVertexBuffer.resize(MAX_FRAME_DRAWS);
	imguiIndexBuffer.resize(MAX_FRAME_DRAWS);
	imguiConstantBuffer.resize(MAX_FRAME_DRAWS);

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		oGFX::CreateBuffer(m_device.m_allocator, vertex_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, imguiVertexBuffer[i]);
		oGFX::CreateBuffer(m_device.m_allocator, index_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, imguiIndexBuffer[i]);
		oGFX::CreateBuffer(m_device.m_allocator, imguiCBsize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, imguiConstantBuffer[i]);
	}


	// TODO: Move other global GPU buffer initialization here...
}

void VulkanRenderer::DestroyRenderBuffers()
{
	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		indirectCommandsBuffer[i].destroy();
		shadowCasterCommandsBuffer[i].destroy();
		instanceBuffer[i].destroy();
		objectInformationBuffer[i].destroy();
		globalLightBuffer[i].destroy();
		gpuBoneMatrixBuffer[i].destroy();
		g_UIVertexBufferGPU[i].destroy();
		g_UIIndexBufferGPU[i].destroy();
		g_particleCommandsBuffer[i].destroy();
	}

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		// dont unmap for create mapped bit
		// vmaUnmapMemory(m_device.m_allocator, imguiVertexBuffer[i].alloc);
		// vmaUnmapMemory(m_device.m_allocator, imguiIndexBuffer[i].alloc);

		vmaDestroyBuffer(m_device.m_allocator, imguiVertexBuffer[i].buffer, imguiVertexBuffer[i].alloc);
		vmaDestroyBuffer(m_device.m_allocator, imguiIndexBuffer[i].buffer, imguiIndexBuffer[i].alloc);
		vmaDestroyBuffer(m_device.m_allocator, imguiConstantBuffer[i].buffer, imguiConstantBuffer[i].alloc);
	}

	
	gpuSkinningBoneWeightsBuffer.destroy();

	for (size_t i = 0; i < g_particleDatas.size(); i++)
	{
		g_particleDatas[i].destroy();
	}

	vmaDestroyBuffer(m_device.m_allocator, SPDconstantBuffer.buffer, SPDconstantBuffer.alloc);
	vmaDestroyBuffer(m_device.m_allocator, SPDatomicBuffer.buffer, SPDatomicBuffer.alloc);

}

void VulkanRenderer::GenerateCPUIndirectDrawCommands()
{
	PROFILE_SCOPED();

	if (currWorld == nullptr)
	{
		return;
	}

	// All object commands
	{
		auto& allObjectsCommands = batches.GetBatch(GraphicsBatch::ALL_OBJECTS);

		objectCount = 0;
		for (auto& indirectCmd : allObjectsCommands)
		{
			objectCount += indirectCmd.instanceCount ?  indirectCmd.instanceCount:1;
		}


		if (objectCount == 0)
			return;


		// Better to catch this on the software side early than the Vulkan validation layer
		// TODO: Fix this gracefully
		if (allObjectsCommands.size() > MAX_OBJECTS)
		{
			MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of indirect command buffer.", L"BAD ERROR");
		}

		auto cmd = GetCommandBuffer();
		indirectCommandsBuffer[getFrame()].writeToCmd(allObjectsCommands.size(), allObjectsCommands.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);

	}

	// shadow commands
	{
		auto& shadowObjects = batches.GetBatch(GraphicsBatch::SHADOW_CAST);
		if (shadowObjects.size() > MAX_OBJECTS)
		{
			MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of indirect command buffer.", L"BAD ERROR");
		}
		shadowCasterCommandsBuffer[getFrame()].clear();
		auto cmd = GetCommandBuffer();
		shadowCasterCommandsBuffer[getFrame()].writeToCmd(shadowObjects.size(), (void*)shadowObjects.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);
	}

	{
		auto& particleCommands = batches.GetParticlesBatch();
		auto& particleData = batches.GetParticlesData();

		g_particleCommandsBuffer[getFrame()].clear();
		g_particleDatas[getFrame()].clear();

		auto cmd = GetCommandBuffer();
		g_particleCommandsBuffer[getFrame()].writeToCmd(particleCommands.size(), particleCommands.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);		
		g_particleDatas[getFrame()].writeToCmd(particleData.size(), particleData.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);
		
	}

}

void VulkanRenderer::UploadInstanceData()
{
	PROFILE_SCOPED();
	//if (instanceBuffer.size != 0) return;
	
	using namespace std::chrono;
	static uint64_t curr = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	static std::default_random_engine rndEngine(static_cast<uint32_t>(curr));
	static std::uniform_real_distribution<float> uniformDist(0.0f, 1.0f);

	constexpr float radius = 10.0f;
	constexpr float offset = 10.0f;

	// update the transform positions
	gpuTransform.clear();
	gpuTransform.reserve(MAX_OBJECTS);

	objectInformation.clear();
	objectInformation.reserve(MAX_OBJECTS);

	boneMatrices.clear();
	boneMatrices.reserve(MAX_OBJECTS); // TODO:: change to better max value

	// TODO: Must the entire buffer be uploaded every frame?

	uint32_t indexCounter = 0;
	std::vector<oGFX::InstanceData> instanceDataBuff;
	instanceDataBuff.reserve(objectCount);
	if (currWorld)
	{
		uint32_t matCnt = 0;
		for (auto& ent : currWorld->GetAllObjectInstances())
		{
			
			auto& mdl = g_globalModels[ent.modelID];
			for (size_t i = 0; i < mdl.m_subMeshes.size(); i++)
			{
				if (ent.submesh[i] == true)
				{
					oGFX::InstanceData instData;
					//size_t sz = instanceData.size();
					//for (size_t x = 0; x < g_globalModels[ent.modelID].meshCount; x++)
					{
						// This is per entity. Should be per material.
						constexpr uint32_t invalidIndex = 0xFFFFFFFF;

						uint32_t albedo = ent.bindlessGlobalTextureIndex_Albedo;
						uint32_t normal = ent.bindlessGlobalTextureIndex_Normal;
						uint32_t roughness = ent.bindlessGlobalTextureIndex_Roughness;
						uint32_t metallic = ent.bindlessGlobalTextureIndex_Metallic;
						uint32_t emissive = ent.bindlessGlobalTextureIndex_Emissive;
						const uint8_t perInstanceData = ent.instanceData;

						if (albedo == invalidIndex || g_Textures[ent.bindlessGlobalTextureIndex_Albedo].isValid == false)
							albedo = whiteTextureID; // TODO: Dont hardcode this bindless texture index
						if (normal == invalidIndex || g_Textures[ent.bindlessGlobalTextureIndex_Normal].isValid == false)
							normal = blackTextureID; // TODO: Dont hardcode this bindless texture index
						if (roughness == invalidIndex || g_Textures[ent.bindlessGlobalTextureIndex_Roughness].isValid == false)
							roughness = whiteTextureID; // TODO: Dont hardcode this bindless texture index
						if (metallic == invalidIndex || g_Textures[ent.bindlessGlobalTextureIndex_Metallic].isValid == false)
							metallic = blackTextureID; // TODO: Dont hardcode this bindless texture index
						if (emissive == invalidIndex || g_Textures[ent.bindlessGlobalTextureIndex_Emissive].isValid == false)
							emissive = blackTextureID; // TODO: Dont hardcode this bindless texture index

						// Important: Make sure this index packing matches the unpacking in the shader
						const uint32_t albedo_normal = albedo << 16 | (normal & 0xFFFF);
						const uint32_t roughness_metallic = roughness << 16 | (metallic & 0xFFFF);
						const uint32_t instanceID = uint32_t(indexCounter); // the instance id should point to the entity
						auto res = ent.flags & ObjectInstanceFlags::SKINNED; 
						auto isSkin = (res== ObjectInstanceFlags::SKINNED);
						const uint32_t emissive_skinned = emissive << 16 | (uint32_t)perInstanceData | isSkin << 8; //matCnt;

																						 // Putting these ranges here for easy reference:
																						 // 9-bit:  [0 to 511]
																						 // 10-bit: [0 to 1023]
																						 // 11-bit: [0 to 2047]
																						 // 12-bit: [0 to 4095]
																						 // 13-bit: [0 to 8191]
																						 // 14-bit: [0 to 16383]
																						 // 15-bit: [0 to 32767]
																						 // 16-bit: [0 to 65535]

																						 // TODO: This is the solution for now.
																						 // In the future, we can just use an index for all the materials (indirection) to fetch from another buffer.
						instData.instanceAttributes = uvec4(instanceID, emissive_skinned, albedo_normal, roughness_metallic);
						
						instanceDataBuff.emplace_back(instData);
					}
				}				

			} // end m_subMeshes loop

			//for (size_t i = 0; i < mdl.m_subMeshes.size(); i++)
			{
				// creates a single transform reference for each entity in the scene
				size_t x = gpuTransform.size();
				mat4 xform = ent.localToWorld;
				GPUTransform gpt;
				gpt.row0 = vec4(xform[0][0], xform[1][0], xform[2][0], xform[3][0]);
				gpt.row1 = vec4(xform[0][1], xform[1][1], xform[2][1], xform[3][1]);
				gpt.row2 = vec4(xform[0][2], xform[1][2], xform[2][2], xform[3][2]);
				mat4 inverseXform = glm::inverse(xform);
				gpt.invRow0 = vec4(inverseXform[0][0], inverseXform[1][0], inverseXform[2][0], inverseXform[3][0]);
				gpt.invRow1 = vec4(inverseXform[0][1], inverseXform[1][1], inverseXform[2][1], inverseXform[3][1]);
				gpt.invRow2 = vec4(inverseXform[0][2], inverseXform[1][2], inverseXform[2][2], inverseXform[3][2]);
				gpuTransform.emplace_back(gpt);
			}
			// skined mesh
			GPUObjectInformation oi;
			oi.entityID = ent.entityID;
			oi.materialIdx = 7; // tem,p
			oi.emissiveColour = ent.emissiveColour;
			if ((ent.flags & ObjectInstanceFlags::SKINNED) == ObjectInstanceFlags::SKINNED)
			{
				auto& mdl = g_globalModels[ent.modelID];

				if (ent.bones.empty())
				{
					ent.bones.resize(mdl.skeleton->inverseBindPose.size());
					for (auto& b:ent.bones )
					{
						b = mat4(1.0f);
					}
				}
				oi.boneWeightsOffset = mdl.skinningWeightsOffset;
				oi.boneStartIdx = static_cast<uint32_t>(boneMatrices.size());
				//oi.boneCnt = static_cast<uint32_t>(ent.bones.size());

				for (size_t i = 0; i < ent.bones.size(); i++)
				{
					boneMatrices.push_back(ent.bones[i]);
				}
			}

			objectInformation.push_back(oi);

			++indexCounter;
			++matCnt;
		}// end of entity instance loop
	}
	

	if (instanceDataBuff.empty())
	{
		return;
	}
	auto cmd = GetCommandBuffer();
	gpuTransformBuffer[getFrame()].writeToCmd(gpuTransform.size(), gpuTransform.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);
	gpuBoneMatrixBuffer[getFrame()].writeToCmd(boneMatrices.size(), boneMatrices.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);

	objectInformationBuffer[getFrame()].writeToCmd(objectInformation.size(), objectInformation.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);

    // Better to catch this on the software side early than the Vulkan validation layer
	// TODO: Fix this gracefully
    if (instanceDataBuff.size() > MAX_OBJECTS)
    {
		MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of instance buffer.", L"BAD ERROR");
    }

	instanceBuffer[getFrame()].writeToCmd(instanceDataBuff.size(), instanceDataBuff.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);

}

void VulkanRenderer::UploadUIData()
{
	const auto& verts = batches.GetUIVertices();

	std::vector<uint32_t> idx;
	const auto numQuads = verts.size()/4;
	uint32_t currVert = 0;
	// hardcode indices
	for (size_t i = 0; i < numQuads; i++)
	{
		idx.emplace_back(currVert + 0);
		idx.emplace_back(currVert + 2);
		idx.emplace_back(currVert + 1);
		idx.emplace_back(currVert + 2);
		idx.emplace_back(currVert + 0);
		idx.emplace_back(currVert + 3);

		currVert += 4;
	}

	auto cmd = GetCommandBuffer();
	g_UIVertexBufferGPU[getFrame()].writeToCmd(verts.size(), verts.data(),cmd,m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);
	g_UIIndexBufferGPU[getFrame()].writeToCmd(idx.size(), idx.data(),cmd,m_device.graphicsQueue,m_device.commandPoolManagers[getFrame()].m_commandpool);


}

bool VulkanRenderer::PrepareFrame()
{
	{
		std::scoped_lock s{ g_mut_workQueue };
		for (size_t i = 0; i < g_workQueue.size(); i++)
		{
			g_workQueue[i]();
		}
		g_workQueue.clear();
	}

	if (resizeSwapchain || windowPtr->m_width == 0 ||windowPtr->m_height == 0)
	{
		m_prepared = ResizeSwapchain();
		if (m_prepared == false)
			return false;
		resizeSwapchain = false;
	}

	if (m_restartIMGUI == true)
	{
		PerformImguiRestart();
	}

	if (m_reloadShaders == true) {
		ReloadShaders();
		m_reloadShaders = false;
	}

	this->BeginDraw(); // TODO: Clean this up...
	
	return true;
}

void VulkanRenderer::BeginDraw()
{
	PROFILE_SCOPED();

	//vkWaitForFences(m_device.logicalDevice, 1, &drawFences[getFrame()], VK_TRUE, UINT64_MAX);
	uint64_t res{};
	VK_CHK(vkGetSemaphoreCounterValue(m_device.logicalDevice, frameCountSemaphore, &res));
	//printf("[FRAME COUNTER %5llu]\n", res);	

	//wait for given fence to signal from last draw before continuing
	{
		PROFILE_SCOPED("Wait Swapchain Fence");
		VK_CHK(vkWaitForFences(m_device.logicalDevice, 1, &drawFences[getFrame()], VK_TRUE, std::numeric_limits<uint64_t>::max()));
		//mainually reset fences
		VK_CHK(vkResetFences(m_device.logicalDevice, 1, &drawFences[getFrame()]));
	}

	{
		PROFILE_SCOPED("Begin Command Buffer");

		m_device.commandPoolManagers[getFrame()].ResetPool();
		//Information about how to begin each command buffer
		VkCommandBufferBeginInfo bufferBeginInfo = oGFX::vkutils::inits::commandBufferBeginInfo();
		//start recording commanders to command buffer!
		//VkResult result = vkBeginCommandBuffer(cmd, &bufferBeginInfo);
		//if (result != VK_SUCCESS)
		//{
		//	std::cerr << "Failed to start recording a Command Buffer!" << std::endl;
		//	__debugbreak();
		//}
	}

	{
		{
			PROFILE_SCOPED("vkAcquireNextImageKHR");

			//1. get the next available image to draw to and set something to signal when we're finished with the image ( a semaphore )
			// -- GET NEXT IMAGE
			//get  index of next image to be drawn to , and signal semaphore when ready to be drawn to
			VkResult res = vkAcquireNextImageKHR(m_device.logicalDevice, m_swapchain.swapchain, std::numeric_limits<uint64_t>::max(),
			    presentSemaphore[getFrame()], VK_NULL_HANDLE, &swapchainIdx);
			if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR /*|| WINDOW_RESIZED*/)
			{
				resizeSwapchain = true;
				m_prepared = false;
			}
		}
		
		//std::cout << currentFrame << " Setting " << std::to_string(swapchainIdx) <<" " << oGFX::vkutils::tools::VkImageLayoutString(m_swapchain.swapChainImages[swapchainIdx].currentLayout) << std::endl;

		DelayedDeleter::get()->Update();

		descAllocs[getFrame()].ResetPools();

		shadowsRendered = false;

		if (currWorld)
		{
			batches.Init(currWorld, this, MAX_OBJECTS);
			currWorld->BeginFrame();
			batches.GenerateBatches();
		}

		{
			PROFILE_SCOPED("Transfer data");
			{
				
			}
			auto cmd = GetCommandBuffer();
			if (g_GlobalMeshBuffers.IdxBuffer.m_mustUpdate) g_GlobalMeshBuffers.IdxBuffer.flushToGPU(cmd, m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);
			if (g_GlobalMeshBuffers.VtxBuffer.m_mustUpdate) g_GlobalMeshBuffers.VtxBuffer.flushToGPU(cmd, m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);
			if (gpuSkinningBoneWeightsBuffer.m_mustUpdate) gpuSkinningBoneWeightsBuffer.flushToGPU(cmd, m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);
			
			UpdateUniformBuffers();
			UploadInstanceData();
			UploadUIData();
			UploadLights();

			GenerateCPUIndirectDrawCommands();
	
			VkDescriptorImageInfo basicSampler = oGFX::vkutils::inits::descriptorImageInfo(
				GfxSamplerManager::GetDefaultSampler(),
				0,
				VK_IMAGE_LAYOUT_UNDEFINED);

			DescriptorBuilder::Begin()
				.BindImage(0, &basicSampler, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.BindBuffer(3, gpuTransformBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
				.BindBuffer(4, gpuBoneMatrixBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
				.BindBuffer(5, objectInformationBuffer[getFrame()].GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
				.BindBuffer(6, gpuSkinningBoneWeightsBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
				.Build(descriptorSet_gpuscene,SetLayoutDB::gpuscene);
	
			auto uniformMinAlignment = m_device.properties.limits.minUniformBufferOffsetAlignment;
			auto paddedAlignment = oGFX::vkutils::tools::UniformBufferPaddedSize(2*sizeof(CB::FrameContextUBO), uniformMinAlignment);
			
			VkDescriptorBufferInfo vpBufferInfo{};
			vpBufferInfo.buffer = vpUniformBuffer[getFrame()].buffer;	// buffer to get data from
			vpBufferInfo.offset = 0;				// position of start of data
			vpBufferInfo.range = sizeof(CB::FrameContextUBO);		// size of data
			DescriptorBuilder::Begin()
				.BindBuffer(0, &vpBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT)
				.Build(descriptorSets_uniform[getFrame()], SetLayoutDB::FrameUniform);
	
	
		}
		
	}

	
}

void VulkanRenderer::RenderFrame()
{
	PROFILE_SCOPED();


	bool shouldRunDebugDraw = UploadDebugDrawBuffers();
    {
		// Command list has already started inside VulkanRenderer::Draw
        PROFILE_GPU_CONTEXT(GetCommandBuffer());

        //this->SimplePass(); // Unsued
		// Manually schedule the order of the render pass execution. (single threaded)
		if(currWorld)
		{
			if (getFrame())
			{
				std::string str("CommandListEven " + std::to_string(getFrame()));
				PROFILE_GPU_EVENT( "CommandListEven");
				RenderFunc(shouldRunDebugDraw);
			}
			else
			{
				std::string str("CommandListOdd " + std::to_string(getFrame()));
				PROFILE_GPU_EVENT( "CommandListOdd");
				RenderFunc(shouldRunDebugDraw);
				//std::this_thread::sleep_for(std::chrono::milliseconds(16));
			}		
			
		}
		{
			// RenderPassDatabase::GetRenderPass<DebugDrawRenderpass>()->dodebugRendering = shouldRunDebugDraw;
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_ImguiRenderpass->Draw(cmd);
		}
    }
}

void VulkanRenderer::RenderFunc(bool shouldRunDebugDraw)
{
	renderIteration = 0;
	for (size_t i = 0; i < currWorld->numCameras; i++)
	{		
		if (currWorld->shouldRenderCamera[i] == false)
		{
			++renderIteration;
			continue;
		}

		renderTargetInUseID = currWorld->targetIDs[i];
		VkMemoryBarrier memoryBarrier{};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;	

		vkCmdPipelineBarrier(GetCommandBuffer(),
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // srcStageMask
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // dstStageMask
			VK_DEPENDENCY_BY_REGION_BIT,		  // dependancy flag
			0,                                    // memoryBarrierCount
			nullptr,                       // pMemoryBarriers
			0, NULL, 0, NULL
		);
		
		if (shadowsRendered == false) // only render shadowpass once per frame...  // this is for multi-viewport
		{
			//generally works until we need to perform better frustrum culling....
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_ShadowPass->Draw(cmd);
			shadowsRendered = true;
		}

		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_ZPrePass->Draw(cmd);
		}
		
		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_GBufferRenderPass->Draw(cmd);
		}	
		
		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			VK_NAME(m_device.logicalDevice, "SSAO_CMD", cmd);
			g_SSAORenderPass->Draw(cmd);
		}

		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_LightingPass->Draw(cmd);
		}

		if(g_cubeMap.image.image != VK_NULL_HANDLE)
		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_SkyRenderPass->Draw(cmd);
		}

		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_LightingHistogram->Draw(cmd);
			VkBufferCopy region{};
			region.size = sizeof(LuminenceData);
			vkCmdCopyBuffer(cmd, LuminanceBuffer.buffer, LuminanceMonitor.buffer, 1, &region);
			vmaFlushAllocation(m_device.m_allocator, LuminanceMonitor.alloc, 0, sizeof(LuminenceData));
		}		


		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_BloomPass->Draw(cmd);
		}

		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_ForwardParticlePass->Draw(cmd);
		}

		{
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_ForwardUIPass->Draw(cmd);
		}
#if defined		(ENABLE_DECAL_IMPLEMENTATION)
		RenderPassDatabase::GetRenderPass<ForwardDecalRenderpass>()->Draw();
#endif				
		if (shouldRunDebugDraw) // for now need to run regardless because of transition.. TODO: FIX IT ONE DAY
		{
			// RenderPassDatabase::GetRenderPass<DebugDrawRenderpass>()->dodebugRendering = shouldRunDebugDraw;
			const VkCommandBuffer cmd = GetCommandBuffer();
			g_DebugDrawRenderpass->Draw(cmd);
		}		

		++renderIteration; // next viewport
	}

	auto& dst = m_swapchain.swapChainImages[swapchainIdx];
	//std::cout << currentFrame << " Func " << std::to_string(swapchainIdx) <<" " << oGFX::vkutils::tools::VkImageLayoutString(dst.currentLayout) << std::endl;
	if (currWorld->numCameras > 1)
	{
		// TODO: Very bad pls fix
		auto thisID = currWorld->targetIDs[1];
		auto& texture = renderTargets[thisID].texture;		

		auto nextID = currWorld->targetIDs[0];
		auto& nextTexture = renderTargets[nextID].texture;
		FullscreenBlit(GetCommandBuffer(), nextTexture, nextTexture.referenceLayout, dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	}
	else
	{
		auto thisID = currWorld->targetIDs[0];
		auto& texture = renderTargets[thisID].texture;
		FullscreenBlit(GetCommandBuffer(), texture, texture.referenceLayout, dst, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
	// only blit main framebuffer

	//if (shouldRunDebugDraw) // for now need to run regardless because of transition.. TODO: FIX IT ONE DAY
	
}

void VulkanRenderer::Present()
{

	PROFILE_SCOPED();

	//	std::cout << currentFrame << " Present " << std::to_string(swapchainIdx) 
	//		<<" " << oGFX::vkutils::tools::VkImageLayoutString(m_swapchain.swapChainImages[swapchainIdx].currentLayout) << std::endl;
	if (m_swapchain.swapChainImages[swapchainIdx].currentLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		std::cout << currentFrame << " Transition to present.." << std::endl;
		vkutils::TransitionImage(GetCommandBuffer(), m_swapchain.swapChainImages[swapchainIdx], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
#if OO_END_PRODUCT
#else

#endif // OO_END_PRODUCT
	
	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[swapchainImageIndex]);
	//stop recording to command buffer
	//VkResult result = vkEndCommandBuffer(GetCommandBuffer());
	VkResult result{};
	//if (result != VK_SUCCESS)
	//{
	//	std::cerr << "Failed to stop recording a Command Buffer!" << std::endl;
	//	__debugbreak();
	//}

	


	//2. Submit command buffer to queue for execution, make sure it waits for image to be signalled as available before drawing
	//		and signals when it has finished rendering
	// --SUBMIT COMMAND BUFFER TO RENDER
	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1; //number of semaphores to wait on
	submitInfo.pWaitSemaphores = &presentSemaphore[getFrame()]; //list of semaphores to wait on
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	std::vector <VkSemaphore> frameSemaphores = { renderSemaphore[getFrame()],
	};

	auto cmd = GetCommandBuffer();
	submitInfo.pWaitDstStageMask = waitStages; //stages to check semapheres at
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;	// command buffer to submit
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(frameSemaphores.size());						// number of semaphores to signal
	submitInfo.pSignalSemaphores = frameSemaphores.data();				// semphores to signal when command buffer finished

																				//submit command buffer to queue
	{
		PROFILE_SCOPED("SubmitMainQueue");
		m_device.commandPoolManagers[getFrame()].SubmitAll(m_device.graphicsQueue, submitInfo, drawFences[getFrame()]);
		//result = vkQueueSubmit(m_device.graphicsQueue, 1, &submitInfo, drawFences[getFrame()]);
		if (result != VK_SUCCESS)
		{
			std::cerr << "Failed to submit command buffer to queue! " << oGFX::vkutils::tools::VkResultString(result) << std::endl;
			__debugbreak();
		}
	}

	auto present = std::min(1u, getFrame() - 1u);
	//3. present image t oscreen when it has signalled finished rendering
	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderSemaphore[getFrame()];	//semaphores to wait on
	presentInfo.swapchainCount = 1;					//number of swapchains to present to
	presentInfo.pSwapchains = &m_swapchain.swapchain;			//swapchains to present images to
	presentInfo.pImageIndices = &swapchainIdx;		//index of images in swapchains to present
	//std::cout << "swapchainidx " << getFrame() << "\t currentFrame " << currentFrame << std::endl;
															//present image
	PROFILE_GPU_PRESENT(m_swapchain.swapchain);
	
	{
		PROFILE_SCOPED("QueuePresent")
		result = vkQueuePresentKHR(m_device.graphicsQueue, &presentInfo);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR /*|| WINDOW_RESIZED*/)
		{
			resizeSwapchain = true;
			m_prepared = false;
			return;
		}
		else if(result != VK_SUCCESS && result!= VK_SUBOPTIMAL_KHR)
		{
			std::cout << oGFX::vkutils::tools::VkResultString(result) << "\nFailed to present image!" << std::endl;
		}
	}

	uint64_t  signalCounter = currentFrame + 1;
	VkTimelineSemaphoreSubmitInfo computeTimelineInfo{};
	computeTimelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	computeTimelineInfo.pNext = nullptr;
	computeTimelineInfo.waitSemaphoreValueCount = 0;
	computeTimelineInfo.pWaitSemaphoreValues = nullptr;
	computeTimelineInfo.signalSemaphoreValueCount = 1;
	computeTimelineInfo.pSignalSemaphoreValues= &signalCounter;

	VkSubmitInfo qsi{};
	qsi.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	qsi.pNext = &computeTimelineInfo;
	qsi.pWaitDstStageMask = waitStages; //stages to check semapheres at
	qsi.commandBufferCount = 0;
	qsi.pCommandBuffers = nullptr;	// command buffer to submit
	qsi.signalSemaphoreCount = 1;						// number of semaphores to signal
	qsi.pSignalSemaphores = &frameCountSemaphore;
	vkQueueSubmit(m_device.graphicsQueue, 1, &qsi, nullptr);
	//get next frame (use % MAX_FRAME_DRAWS to keep value below max frames)
	//currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
	++currentFrame;
}

void ffxSpdSetup(uint32_t*    dispatchThreadGroupCountXY,
                         uint32_t*    workGroupOffset,
                         uint32_t*    numWorkGroupsAndMips,
					     uint32_t*     rectInfo,
                         int32_t mips)
{
    // determines the offset of the first tile to downsample based on
    // left (rectInfo[0]) and top (rectInfo[1]) of the subregion.
    workGroupOffset[0] = rectInfo[0] / 64;
    workGroupOffset[1] = rectInfo[1] / 64;

    uint32_t endIndexX = (rectInfo[0] + rectInfo[2] - 1) / 64;  // rectInfo[0] = left, rectInfo[2] = width
	uint32_t endIndexY = (rectInfo[1] + rectInfo[3] - 1) / 64;  // rectInfo[1] = top, rectInfo[3] = height

    // we only need to dispatch as many thread groups as tiles we need to downsample
    // number of tiles per slice depends on the subregion to downsample
    dispatchThreadGroupCountXY[0] = endIndexX + 1 - workGroupOffset[0];
    dispatchThreadGroupCountXY[1] = endIndexY + 1 - workGroupOffset[1];

    // number of thread groups per slice
    numWorkGroupsAndMips[0] = (dispatchThreadGroupCountXY[0]) * (dispatchThreadGroupCountXY[1]);

    if (mips >= 0)
    {
        numWorkGroupsAndMips[1] = uint32_t(mips);
    }
    else
    {
        // calculate based on rect width and height
		uint32_t resolution    = std::max(rectInfo[2], rectInfo[3]);
        numWorkGroupsAndMips[1] = uint32_t((std::min(std::floor(std::log2(float(resolution))), float(12))));
    }
}

void VulkanRenderer::GenerateMipmaps(vkutils::Texture& texture)
{
	auto oldLayout = texture.currentLayout;
	
	constexpr size_t maxNumMips = 13;
	auto texMips = std::floor(std::log2(std::max(texture.width, texture.height))) + 1;

	if (texMips < 2) return;

	vkutils::Texture generatedTexture; // writing into a new texture
	generatedTexture = texture;
	generatedTexture.image.image = VK_NULL_HANDLE;
	generatedTexture.image.allocation = VK_NULL_HANDLE;
	generatedTexture.view = VK_NULL_HANDLE;
	generatedTexture.name += " xd";
	generatedTexture.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	generatedTexture.AllocateImageMemory(&m_device, generatedTexture.usage, (uint32_t)texMips);
	generatedTexture.CreateImageView();

	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext = NULL;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY; // for shader
	viewCreateInfo.format = generatedTexture.format;
	viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = texture.layerCount;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	
	viewCreateInfo.image = generatedTexture.image.image;
	std::array < VkImageView, maxNumMips> mipViews;
	for (size_t i = 0; i < texMips; i++)
	{
		viewCreateInfo.subresourceRange.baseMipLevel = (uint32_t)i;
		vkCreateImageView(m_device.logicalDevice,&viewCreateInfo,nullptr, &mipViews[i]);
	}

	std::array<VkDescriptorImageInfo, maxNumMips> samplers{};
	for (size_t i = 0; i < samplers.size(); i++)
	{
		samplers[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplers[i].imageView = mipViews[0];
		samplers[i].sampler = samplerManager.GetSampler_EdgeClamp();
	}
	for (size_t i = 0; i < texMips; i++)
	{		
		samplers[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplers[i].imageView = mipViews[i];
		samplers[i].sampler = samplerManager.GetSampler_EdgeClamp();
	}

	VkDescriptorBufferInfo cb{};
	cb.buffer = SPDconstantBuffer.buffer;
	cb.offset = 0;
	cb.range = VK_WHOLE_SIZE;

	VkDescriptorBufferInfo atomic{};
	atomic.buffer = SPDatomicBuffer.buffer;
	atomic.offset = 0;
	atomic.range = VK_WHOLE_SIZE;

	VkDescriptorImageInfo dii{};
	dii.imageLayout = texture.currentLayout;
	dii.imageView = texture.view;
	dii.sampler = samplerManager.GetSampler_SSAOEdgeClamp();
	

	std::array<VkDescriptorSet, 1> dstsets;
	DescriptorBuilder::Begin()
		.BindImage(0, &dii, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(3000, &cb, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindImage(2002, samplers.data(), VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 13)
		.BindImage(2001, &samplers[6], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
		.BindBuffer(2000, &atomic, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.Build(dstsets[0], SetLayoutDB::compute_AMDSPD);

	
	auto cmd = beginSingleTimeCommands();

	{
		rhi::CommandList cmdlist{ cmd, "Mipmap generation" };
		vkutils::ComputeImageBarrier(cmd, generatedTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkutils::ComputeImageBarrier(cmd, texture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		VkImageCopy region{};
		region.srcSubresource = VkImageSubresourceLayers{VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
		region.srcSubresource.layerCount = texture.layerCount;
		region.srcOffset = {};
		region.dstSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT,0,0,1 };
		region.dstSubresource.layerCount = generatedTexture.layerCount;
		region.dstOffset={};
		region.extent = { texture.width,texture.height,1 };
		vkCmdCopyImage(cmd, texture.image.image, texture.currentLayout
			, generatedTexture.image.image, generatedTexture.currentLayout,
			1, &region);

		vkutils::ComputeImageBarrier(cmd, generatedTexture, VK_IMAGE_LAYOUT_GENERAL);
		vkutils::ComputeImageBarrier(cmd, texture, VK_IMAGE_LAYOUT_GENERAL);

		//clear buffer
		vkCmdFillBuffer(cmd, atomic.buffer, atomic.offset, VK_WHOLE_SIZE, 0);
		VkBufferMemoryBarrier bmb{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
		bmb.buffer = atomic.buffer;
		bmb.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		bmb.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		bmb.offset = 0;
		bmb.size = VK_WHOLE_SIZE;
		bmb.srcQueueFamilyIndex = m_device.queueIndices.graphicsFamily;
		vkCmdPipelineBarrier(
			cmd,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			1, &bmb,
			0, nullptr);
		cmdlist.BindPSO(pso_utilAMDSPD, PSOLayoutDB::AMDSPDPSOLayout,VK_PIPELINE_BIND_POINT_COMPUTE);
		cmdlist.BindDescriptorSet(PSOLayoutDB::AMDSPDPSOLayout, 0, dstsets, VK_PIPELINE_BIND_POINT_COMPUTE, 0);


		CB::AMDSPD_UBO spdConstants{};
		// Get SPD info for run
		uint32_t dispatchThreadGroupCountXY[2];
		uint32_t numWorkGroupsAndMips[2];
		uint32_t rectInfo[4] = { 0, 0, generatedTexture.width, generatedTexture.height }; // left, top, width, height
		ffxSpdSetup(dispatchThreadGroupCountXY, spdConstants.workGroupOffset, numWorkGroupsAndMips, rectInfo, -1);

		// Complete setting up the constant buffer data
		spdConstants.mips = numWorkGroupsAndMips[1];
		spdConstants.numWorkGroups = numWorkGroupsAndMips[0];
		spdConstants.invInputSize[0] = 1.f / texture.width;
		spdConstants.invInputSize[1] = 1.f / texture.height;

		// This value is the image region dimension that each thread group of the FSR shader operates on
		uint32_t dispatchX = dispatchThreadGroupCountXY[0];
		uint32_t dispatchY = dispatchThreadGroupCountXY[1];
		uint32_t dispatchZ = generatedTexture.layerCount; // tex.depth
		//vmaAllocateMemory
		void* data{};
		vmaMapMemory(m_device.m_allocator, SPDconstantBuffer.alloc, &data);
		memcpy(data, &spdConstants, sizeof(CB::AMDSPD_UBO));
		vmaUnmapMemory(m_device.m_allocator, SPDconstantBuffer.alloc);

		cmdlist.Dispatch(dispatchX, dispatchY, dispatchZ);
		//vkCmdDispatch(cmd, dispatchX, dispatchY, dispatchZ);

		//vkutils::ComputeImageBarrier(cmd, texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, texture.mipLevels);
		// shader read(2-A) -> general(2-A)
		vkutils::ComputeImageBarrier(cmd, texture, oldLayout);
		vkutils::ComputeImageBarrier(cmd, generatedTexture, oldLayout);
		// shader read(A) -> orig(A)  
	}

	//m_device.commandPoolManagers[getFrame()].SubmitCommandBuffer(m_device.graphicsQueue, cmd);
	endSingleTimeCommands(cmd);
	vkQueueWaitIdle(m_device.graphicsQueue);
	vkFreeCommandBuffers(m_device.logicalDevice,m_device.commandPoolManagers[getFrame()].m_commandpool, 1, &cmd);

	generatedTexture.updateDescriptor();

	std::swap(texture, generatedTexture);
	generatedTexture.destroy();

	for (size_t i = 0; i < texMips; i++)
	{
		vkDestroyImageView(m_device.logicalDevice, mipViews[i], nullptr);
	}
	//DelayedDeleter::get()->DeleteAfterFrames([buffer = std::move(scratchBuffer)]() mutable {buffer.destroy(); });
}

void VulkanRenderer::GenerateRadianceMap(VkCommandBuffer cmdlist, vkutils::CubeTexture& cubemap)
{

	g_radianceMap = cubemap;
	g_radianceMap.image = {};
	g_radianceMap.view = VK_NULL_HANDLE;
	g_radianceMap.name = "radianceMap";
	g_radianceMap.format = G_HDR_FORMAT_ALPHA;
	g_radianceMap.width = 32;
	g_radianceMap.height = 32;

	g_radianceMap.AllocateImageMemory(&m_device, g_radianceMap.usage);
	g_radianceMap.CreateImageView();
	vkutils::ComputeImageBarrier(cmdlist, g_radianceMap, VK_IMAGE_LAYOUT_UNDEFINED, g_radianceMap.referenceLayout);
	
	rhi::CommandList cmd{cmdlist, "RadianceMapGeneration"};

	glm::vec4 values{};
	const float MAX_LIGHT_CONTRIBUTION = 100.0f;
	values.x = MAX_LIGHT_CONTRIBUTION;

	VkPushConstantRange pcr{};
	pcr.size = sizeof(glm::vec4);
	pcr.stageFlags = VK_SHADER_STAGE_ALL;

	cmd.SetPushConstant(PSOLayoutDB::RadiancePSOLayout, pcr, &values.x);

	// use source to create radiance map for 6 faces using compute
	cmd.BindPSO(pso_radiance, PSOLayoutDB::RadiancePSOLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindSampler(0, GfxSamplerManager::GetSampler_Cube())
		.BindImage(1, &cubemap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
		.BindImage(2, &g_radianceMap, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	const uint32_t CUBE_FACES = 6;
	cmd.Dispatch(g_radianceMap.width / 16 + 1, g_radianceMap.height / 16 + 1, CUBE_FACES);


	// use spd to generate mipmaps

}

void VulkanRenderer::GeneratePrefilterMap(VkCommandBuffer cmdlist, vkutils::CubeTexture& cubemap)
{
	g_prefilterMap = cubemap;
	g_prefilterMap.image = {};
	g_prefilterMap.view = VK_NULL_HANDLE;
	g_prefilterMap.name = "prefilterMap";
	g_prefilterMap.format = G_HDR_FORMAT_ALPHA;
	g_prefilterMap.width = 128;
	g_prefilterMap.height = 128;

	const uint32_t NumRoughnessSampler = 6;

	g_prefilterMap.AllocateImageMemory(&m_device, g_prefilterMap.usage, NumRoughnessSampler);
	g_prefilterMap.CreateImageView();
	vkutils::ComputeImageBarrier(cmdlist, g_prefilterMap, VK_IMAGE_LAYOUT_UNDEFINED, g_prefilterMap.referenceLayout);

	rhi::CommandList cmd{ cmdlist, "PrefilterGeneration"};
	// use source to create radiance map for 6 faces using compute
	cmd.BindPSO(pso_prefilter, PSOLayoutDB::prefilterPSOLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
	//cmd.DescriptorSetBegin(0)
	//	.BindSampler(0, GfxSamplerManager::GetSampler_Cube())
	//	.BindImage(1, &cubemap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
	//	.BindImage(2, &g_prefilterMap, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	VkPushConstantRange range{};
	range.size = sizeof(float);
	range.stageFlags = VK_SHADER_STAGE_ALL;

	for (size_t i = 0; i < g_prefilterMap.mipLevels; i++)
	{
		VkImageView view = g_prefilterMap.GenerateMipView(i);
		float roughness = float(i) / (g_prefilterMap.mipLevels - 1);

		cmd.SetPushConstant(PSOLayoutDB::prefilterPSOLayout, range, &roughness);

		glm::vec2 dims = glm::vec2{ g_prefilterMap.width>>i,g_prefilterMap.height>>i };

		const uint32_t CUBE_FACES = 6;
		cmd.DescriptorSetBegin(0)
			.BindSampler(0, GfxSamplerManager::GetSampler_Cube())
			.BindImage(1, &cubemap, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
			.BindImage(2, &g_prefilterMap, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, view);

		cmd.Dispatch(dims.x/ 16 + 1, dims.y / 16 + 1, CUBE_FACES);
		auto delFun = [imgView = view, dev = m_device.logicalDevice]
			{
				vkDestroyImageView(dev, imgView, nullptr);
			};
		DelayedDeleter::get()->DeleteAfterFrames(delFun);
	}
	



}

void VulkanRenderer::GenerateBRDFLUT(VkCommandBuffer cmdlist, vkutils::Texture2D& texture)
{
	if (g_brdfLUT.image.image != VK_NULL_HANDLE) return; // already generated
	g_brdfLUT.PrepareEmpty(VK_FORMAT_R16G16_SFLOAT, 512, 512, &m_device);
	g_brdfLUT.name = "BRDF_LUT";
	g_brdfLUT.AllocateImageMemory(&m_device,g_brdfLUT.usage);
	g_brdfLUT.CreateImageView();
	vkutils::SetImageInitialState(cmdlist, g_brdfLUT);

	rhi::CommandList cmd{ cmdlist, "BRDF LUT gen",glm::vec4{1.0,0.0,0.0,1.0} };	
	cmd.BindPSO(pso_brdfLUT, PSOLayoutDB::BRDFLUTPSOLayout, VK_PIPELINE_BIND_POINT_COMPUTE);
	cmd.DescriptorSetBegin(0)
		.BindImage(2, &g_brdfLUT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.Dispatch(g_brdfLUT.width / 16 + 1, g_brdfLUT.height / 16 + 1);


}

bool VulkanRenderer::ResizeSwapchain()
{
	while (windowPtr->m_height == 0 || windowPtr->m_width == 0)
	{
		//if (windowPtr->m_type == Window::WINDOWS32)
		//{
		//	Window::PollEvents();
		//}
		//else
		{
			return false;
		}
		if (windowPtr->windowShouldClose) return false;
	}
	m_swapchain.Init(m_instance, m_device);
	CreateDefaultRenderpass();
	//CreateDepthBufferImage();
	CreateFramebuffers();

	fbCache.ResizeSwapchain(m_swapchain.swapChainExtent.width, m_swapchain.swapChainExtent.height);

	ResizeGUIBuffers();

	// update imgui shit
	if (currWorld)
	{
		for (size_t x = 0; x < currWorld->numCameras; x++)
		{
			const auto targetID = currWorld->targetIDs[x];
			auto& image = renderTargets[targetID].texture;
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = samplerManager.GetDefaultSampler();
			desc_image[0].imageView = image.view;
			desc_image[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = (VkDescriptorSet)currWorld->imguiID[x];
			write_desc[0].descriptorCount = 1;
			if (image.referenceLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			}
			else
			{
				write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			}
			if (currWorld->imguiID[x])
			{
				write_desc[0].pImageInfo = desc_image;
				vkUpdateDescriptorSets(m_device.logicalDevice, 1, write_desc, 0, NULL);
			}
		}		
	}

	return true;
}

uint32_t VulkanRenderer::GetDefaultCubeID()
{
	return def_cube->meshResource;
}

uint32_t VulkanRenderer::GetDefaultPlaneID()
{
	return def_plane->meshResource;
}

uint32_t VulkanRenderer::GetDefaultSpriteID()
{
	return def_sprite->meshResource;
}

uint32_t VulkanRenderer::getFrame() const
{
	return currentFrame % MAX_FRAME_DRAWS;
}

ModelFileResource* VulkanRenderer::GetDefaultCube()
{
	return def_cube.get();
}

oGFX::Font* VulkanRenderer::GetDefaultFont()
{
	return def_font.get();
}

oGFX::Font * VulkanRenderer::LoadFont(const std::string & filename)
{

	auto* font = new oGFX::Font;
	oGFX::TexturePacker atlas = CreateFontAtlas(filename, *font);

	//std::stringstream ss;
	//for (auto& car : font->m_characterInfos)
	//{
	//	const auto g = car.second;
	//	ss << "[" << (char)car.first << "] {" << g.textureCoordinates.x << "," << g.textureCoordinates.y << "}"
	//		"] {" << g.textureCoordinates.z << "," << g.textureCoordinates.w << "}\n";
	//}
	//std::cout << ss.str();

	font->m_name = std::filesystem::path(filename).stem().wstring();
	size_t channels = 4;
	//atlas.buffer.resize(atlas.textureSize.x * atlas.textureSize.y * channels);

	bool generateMips = false;
	font->m_atlasID = CreateTexture(atlas.textureSize.x, atlas.textureSize.y, (uint8_t*)atlas.buffer.data(),generateMips);

	return font;
}

ModelFileResource* VulkanRenderer::LoadModelFromFile(const std::string& file)
{
	std::stringstream ss;
	// new model loader
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_REMOVE_EMPTY_BONES, false);
	uint flags = 0;
	flags |= aiProcess_Triangulate;
	flags |= aiProcess_GenSmoothNormals;
	flags |= aiProcess_ImproveCacheLocality;
	flags |= aiProcess_CalcTangentSpace;
	flags |= aiProcess_FindInstances; // this step is slow but it finds duplicate instances in FBX
	flags |= aiProcess_FlipUVs;
	//flags |= aiProcess_LimitBoneWeights; // limmits bones to 4
	const aiScene *scene = importer.ReadFile(file,flags
		//  aiProcess_Triangulate                // Make sure we get triangles rather than nvert polygons
		//| aiProcess_LimitBoneWeights           // 4 weights for skin model max
		//| aiProcess_GenUVCoords                // Convert any type of mapping to uv mapping
		//| aiProcess_TransformUVCoords          // preprocess UV transformations (scaling, translation ...)
		//| aiProcess_FindInstances              // search for instanced meshes and remove them by references to one master
		//| aiProcess_CalcTangentSpace           // calculate tangents and bitangents if possible
		////| aiProcess_JoinIdenticalVertices      // join identical vertices/ optimize indexing
		//| aiProcess_RemoveRedundantMaterials   // remove redundant materials
		//| aiProcess_FindInvalidData            // detect invalid model data, such as invalid normal vectors
		////| aiProcess_PreTransformVertices       // TODO: remove for skinning?
		//| aiProcess_FlipUVs						// TODO: some mesh need
		//| aiProcess_GenNormals					// TODO: some mesh need
	);

	if (!scene)
	{
		return nullptr; // Dont explode...
		//throw std::runtime_error("Failed to load model! (" + file + ")");
	}

	

	ss <<"[Loading] " << file << std::endl;
	//if (scene->mNumAnimations && scene->mAnimations[0]->mNumMorphMeshChannels)
	//{
	//	std::stringstream ss{"Morphs\n"};
	//	for (size_t i = 0; i < scene->mAnimations[0]->mNumMorphMeshChannels; i++)
	//	{
	//		auto& morph = scene->mAnimations[0]->mMorphMeshChannels[i];
	//		for (size_t y = 0; y < morph->mNumKeys; y++)
	//		{
	//			auto& key = morph->mKeys[y];
	//			ss << "T:[" << key.mTime << "]" << std::endl;
	//			for (size_t x = 0; x < key.mNumValuesAndWeights; x++)
	//			{
	//				ss << "\tV:[" << key.mValues[x] << "] W:[" << key.mWeights[x] << "]" << std::endl;
	//			}
	//		}
	//		ss << std::endl;
	//	}
	//	os << ss.str() << std::endl;
	//}

	size_t count{ 0 };
	ss << "Meshes" << scene->mNumMeshes << std::endl;
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		auto& mesh = scene->mMeshes[i];
		ss << "\tMesh" << i << " " << mesh->mName.C_Str() << std::endl;
		ss << "\t\tverts:"  << mesh->mNumVertices << std::endl;
		ss << "\t\tbones:"  << mesh->mNumBones << std::endl;
		/*
		for (size_t anim = 0; anim < mesh->mNumAnimMeshes; anim++)
		{
		std::stringstream ss;
			ss << "Anim mesh_" << anim << ":" << mesh->mName.C_Str() << std::endl;
			auto& animMesh = mesh->mAnimMeshes[anim];
			if (animMesh->HasPositions())
			{
				for (size_t pos = 0; pos < animMesh->mNumVertices; pos++)
				{
					++count;
					auto v = aiVector3D_to_glm(animMesh->mVertices[pos]);
					ss << "\tPos:"<< pos<< "[" << v.x << "," << v.y << "," << v.z <<"]" << std::endl;
				}
			}
		os << ss.str() << std::endl;
		}
		os << "Takes huge amount of data : " << (float)(sizeof(glm::vec3) * count) / (1024) << "Kb" << std::endl;
		*/
		
		//int sum = 0;
		//for (size_t x = 0; x <  scene->mMeshes[i]->mNumBones; x++)
		//{
		//	std::map<uint32_t, float> wts;
		//	os << "\t\t\tweights:"  << scene->mMeshes[i]->mBones[x]->mNumWeights << std::endl;
		//	for (size_t y = 0; y < scene->mMeshes[i]->mBones[x]->mNumWeights; y++)
		//	{
		//		auto& weight = scene->mMeshes[i]->mBones[x]->mWeights[y];
		//		assert(wts.find(weight.mVertexId) == wts.end());
		//		wts[weight.mVertexId] = weight.mWeight;
		//	}
		//	for (auto [v,w] :wts)
		//	{
		//		os << "\t\t\t\t"  <<":["<<v <<"," << w << "]" << std::endl;
		//	}
		//	sum += scene->mMeshes[i]->mBones[x]->mNumWeights;
		//}
		//os << "\t\t\t|sum weights:"  << sum << std::endl;
	}

#if 0
	if (scene->HasAnimations())
	{
		os << "Animated scene\n";
		for (size_t i = 0; i < scene->mNumAnimations; i++)
		{
			os << "Anim name: " << scene->mAnimations[i]->mName.C_Str() << std::endl;
			os << "Anim frames: "<< scene->mAnimations[i]->mDuration << std::endl;
			os << "Anim ticksPerSecond: "<< scene->mAnimations[i]->mTicksPerSecond << std::endl;
			os << "Anim duration: "<< static_cast<float>(scene->mAnimations[i]->mDuration)/scene->mAnimations[i]->mTicksPerSecond << std::endl;
			os << "Anim numChannels: "<< scene->mAnimations[i]->mNumChannels << std::endl;
			os << "Anim numMeshChannels: "<< scene->mAnimations[i]->mNumMeshChannels << std::endl;
			os << "Anim numMeshChannels: "<< scene->mAnimations[i]->mNumMorphMeshChannels << std::endl;
			for (size_t x = 0; x < scene->mAnimations[i]->mNumChannels; x++)
			{
				auto& channel = scene->mAnimations[i]->mChannels[x];
				os << "\tKeys name: " << channel->mNodeName.C_Str() << std::endl;
				for (size_t y = 0; y < channel->mNumPositionKeys; y++)
				{
					os << "\t Key_"<< std::to_string(y)<<" time: " << channel->mPositionKeys[y].mTime << std::endl;
					auto& pos = channel->mPositionKeys[y].mValue;
					os << "\t Key_"<< std::to_string(y)<<" value: " <<pos.x <<", " << pos.y<<", " << pos.z << std::endl;
				}
			}
		}
		os << std::endl;
	}
#endif


	ModelFileResource* modelFile = new ModelFileResource(file);
	//modelFile->fileName = file;

	//auto& mdl = [&]()-> gfxModel& {
	//	std::scoped_lock(g_mut_globalModels);
	//	auto mdlResourceIdx = g_globalModels.size();
	//	modelFile->meshResource = static_cast<uint32_t>(mdlResourceIdx);
	//	return  g_globalModels.emplace_back(gfxModel{});		 
	//}();
	uint32_t indx{};
	{
		std::scoped_lock s(g_mut_globalModels);
		auto mdlResourceIdx = g_globalModels.size();
		modelFile->meshResource = static_cast<uint32_t>(mdlResourceIdx);
		g_globalModels.emplace_back(gfxModel{});
		indx = (uint32_t)mdlResourceIdx;
	}
	auto& mdl = g_globalModels[indx];


	mdl.name = std::filesystem::path(file).stem().string();

	mdl.m_subMeshes.resize(scene->mNumMeshes);
	modelFile->numSubmesh =scene->mNumMeshes;
	mdl.cpuModel = modelFile;

	uint32_t totalBones{ 0 };
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		totalBones += scene->mMeshes[i]->mNumBones;
	}
	bool hasBone = totalBones > 0;

	if (hasBone)
	{
		mdl.skeleton = new oGFX::Skeleton();
		modelFile->skeleton = mdl.skeleton;
	}

	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		auto& aimesh = scene->mMeshes[i];
		LoadSubmesh(mdl, mdl.m_subMeshes[i], aimesh, modelFile);
	}
	
	if (hasBone)
	{
		mdl.skeleton->boneWeights.resize(modelFile->vertices.size());
		uint32_t verticesCnt = 0;
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			auto& aimesh = scene->mMeshes[i];
			LoadBoneInformation(*modelFile,*mdl.skeleton, *aimesh, mdl.skeleton->boneWeights, verticesCnt);
		}
		mdl.skeleton->m_boneNodes = new oGFX::BoneNode();
		mdl.skeleton->m_boneNodes->mName = "RootNode";
		BuildSkeletonRecursive(*modelFile, scene->mRootNode, mdl.skeleton->m_boneNodes);
	}
	
	for (auto& sm : mdl.m_subMeshes)
	{
		mdl.vertexCount += sm.vertexCount;
		mdl.indicesCount += sm.indicesCount;
	}

	//always has one transform, root
	modelFile->ModelSceneLoad(scene, *scene->mRootNode, nullptr, glm::mat4{ 1.0f });
	
	{
		LoadMeshFromBuffers(modelFile->vertices, modelFile->indices, &mdl);
	}

	ss << "\t [Meshes loaded] " << modelFile->sceneMeshCount << std::endl;

	//std::cout << ss.str();
	return modelFile;
}

oGFX::TexturePacker VulkanRenderer::CreateFontAtlas(const std::string& filename, oGFX::Font& font)
{


	oGFX::TexturePacker atlas({512,512});


	using namespace msdfgen;
	using namespace msdf_atlas;

	bool success = false;
	// Initialize instance of FreeType library
	if (msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype()) {
		// Load font file
		if (msdfgen::FontHandle *fontHdl = msdfgen::loadFont(ft, filename.c_str())) {
			// Storage for glyph geometry and their coordinates in the atlas
			std::vector<GlyphGeometry> glyphs;
			// FontGeometry is a helper class that loads a set of glyphs from a single font.
			// It can also be used to get additional font metrics, kerning information, etc.
			FontGeometry fontGeometry(&glyphs);
			// Load a set of character glyphs:
			// The second argument can be ignored unless you mix different font sizes in one atlas.
			// In the last argument, you can specify a charset other than ASCII.
			// To load specific glyph indices, use loadGlyphs instead.

			Charset charSet;
			for (size_t i = 0; i < 255; i++)
			{
				charSet.add(static_cast<msdf_atlas::unicode_t>(i));
			}
			fontGeometry.loadCharset(fontHdl, 1.0, charSet);
			// Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
			const double maxCornerAngle = 3.0;
			for (GlyphGeometry &glyph : glyphs)
				glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);
			// TightAtlasPacker class computes the layout of the atlas.
			TightAtlasPacker packer;
			// Set atlas parameters:
			// setDimensions or setDimensionsConstraint to find the best value
			packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::POWER_OF_TWO_SQUARE);
			// setScale for a fixed size or setMinimumScale to use the largest that fits
			packer.setMinimumScale(24.0);
			// setPixelRange or setUnitRange
			packer.setPixelRange(2.0);
			packer.setMiterLimit(1.0);
			// Compute atlas layout - pack glyphs
			packer.pack(glyphs.data(), static_cast<int>(glyphs.size()));
			// Get final atlas dimensions
			int width = 0, height = 0;
			packer.getDimensions(width, height);
			// The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
			ImmediateAtlasGenerator<
				float, // pixel type of buffer for individual glyphs depends on generator function
				4, // number of atlas color channels
				mtsdfGenerator, // function to generate bitmaps for individual glyphs
				BitmapAtlasStorage<byte, 4> // class that stores the atlas bitmap
											// For example, a custom atlas storage class that stores it in VRAM can be used.
			> generator(width, height);
			// GeneratorAttributes can be modified to change the generator's default settings.
			GeneratorAttributes attributes;
			generator.setAttributes(attributes);
			generator.setThreadCount(4);
			// Generate atlas bitmap
			generator.generate(glyphs.data(), static_cast<int>(glyphs.size()));
			// The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
			// The glyphs array (or fontGeometry) contains positioning data for typesetting text.
			auto bitmap = generator.atlasStorage().operator msdfgen::BitmapConstRef<msdfgen::byte, 4>();
			bitmap.pixels;
			bitmap.width;
			bitmap.height;
			atlas.resizeBuffer({ bitmap.width, bitmap.height });
			auto totalPixels = bitmap.width * bitmap.height * 4;
			auto stack = bitmap.width * 4;

			

			constexpr bool FLIP_Y = true;

			for (GlyphGeometry& glyph : glyphs)
			{

				auto c = glyph.getCodepoint();
				auto& infos = font.m_characterInfos[c];
				infos.Advance.x = static_cast<float>(glyph.getAdvance());
				infos.Advance.y = {};
				int wd{}, ht{};
				glyph.getBoxSize(wd,ht);
				infos.Size.x = (float)wd /bitmap.width;
				infos.Size.y = (float)ht /bitmap.height;
				double l, b, t, r;
				glyph.getQuadAtlasBounds(l, b, r, t);

				double pl, pb, pr, pt;
				glyph.getQuadPlaneBounds(pl, pb, pr, pt);
				auto val1 = pr - pl;
				auto val2 = pt - pb;

				infos.Size.x = static_cast<float>(val1);
				infos.Size.y = static_cast<float>(val2);
				
				infos.Bearing = glm::vec2{ pl,pb};
				//infos.Bearing = glm::ivec2{ 1 };

				infos.textureCoordinates = glm::vec4{
					l/bitmap.width,b/bitmap.height,
					r/bitmap.width,t/bitmap.height,
				};
				if constexpr (FLIP_Y == true)
				{
					infos.textureCoordinates.y = 1.0f - infos.textureCoordinates.y;
					infos.textureCoordinates.w = 1.0f - infos.textureCoordinates.w;
				}
			}

			font.m_characterInfos['\n'].Size = font.m_characterInfos['\n'].Size.y == 0 ? 
				font.m_characterInfos['A'].Size : font.m_characterInfos['\n'].Size;
			font.m_characterInfos[' '].Size = font.m_characterInfos[' '].Size.x == 0 ? 
				font.m_characterInfos['\n'].Size : font.m_characterInfos[' '].Size;
			font.m_characterInfos[' '].Advance = font.m_characterInfos[' '].Advance.x == 0 ? 
				font.m_characterInfos['a'].Advance : font.m_characterInfos[' '].Advance;

			if constexpr (FLIP_Y == true)
			{
				for (size_t i = 0; i <  bitmap.height; i++)
				{
					auto front = i * stack;
					auto back = front + stack;
					std::memcpy((uint8_t*)atlas.buffer.data() + front, bitmap.pixels + (totalPixels - back), stack);
				}
			}

			//success = myProject::submitAtlasBitmapAndLayout(generator.atlasStorage(), glyphs);
			// Cleanup
			msdfgen::destroyFont(fontHdl);
		}
		msdfgen::deinitializeFreetype(ft);
	}

#if 0
	std::vector<uint16_t> unknownChars;
	// normal nordic characters are under 255 
	// while extended norse is up to 503
	//old nordic charaters require up to character 0503 and  42856 and 42857

	//init free type library
	FT_Library m_library; // init lib	

	FT_Error error = FT_Init_FreeType(&m_library);
	if (error)
	{
		std::cout << "ERROR LOAD" << std::endl;
		//LOG_ENGINE_CRITICAL("FontError: Failed to load freetype library! Error {0}", error);
	}

	FT_Face face;
	error = FT_New_Face(m_library, filename.c_str(), 0, &face);
	if (error == FT_Err_Unknown_File_Format)
	{
		std::cout << "ERROR FORMAT" << std::endl;
		//LOG_ENGINE_CRITICAL("Font Error: font format is unsupported! Error {0}", error);
	}
	else if (error)
	{
		std::cout << "ERROR UNABLe READ" << std::endl;
		//LOG_ENGINE_CRITICAL("Font Error: unable to read or open font! Error {0}", error);
	}
	// enable unicode
	FT_Select_Charmap(face, ft_encoding_unicode);

	//FT_Set_Pixel_Sizes(face,0, 72);

	FontHandle* fontFace = adoptFreetypeFont(face);
	if (fontFace)
	{
		for (uint16_t c = ' '; c < 125; c++)
		{		
			Shape shape;
			auto glyph_index = FT_Get_Char_Index( face, c );
			double advance{};
			if (loadGlyph(shape, fontFace, GlyphIndex(glyph_index),&advance))
			{

				oo::Font::Glyph character = {
					0,
					glm::vec4(0.0f,0.0f,1.0f,1.0f),
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					glm::ivec2(face->glyph->advance.x, 0/*face->glyph->advance.y*/)
				};
				shape.getBounds()
					character.Advance.x = advance;
				character.Size.x = 16*4;
				character.Size.y = 16*4;

				//if (character.Size.x && character.Size.y && glyph_index != 0)
				//if(face->glyph->advance.y || face->glyph->advance.x)
				if (shape.validate())
				{
					shape.normalize();
					//                      max. angle
					edgeColoringSimple(shape, 3.0);
					//           image width, height
					constexpr int channels = 3;
					Bitmap<float, channels> msdf(32, 32);
					//                     range, scale, translation	
					generateMSDF(msdf, shape, 4.0, 1.0, Vector2(4.0, 4.0));


					int subpixels = (channels)*msdf.width()*msdf.height();
					// add alpha
					int pixelsWithAlpha = (channels+1)*msdf.width()*msdf.height();
					std::vector<unsigned char> bytePixels(pixelsWithAlpha);
					for (int i = 0; i < subpixels; i+=3)
					{
						// make space for alpha
						bytePixels[i+0+(i/3)] = msdfgen::pixelFloatToByte(msdf[i+0]);
						bytePixels[i+1+(i/3)] = msdfgen::pixelFloatToByte(msdf[i+1]);
						bytePixels[i+2+(i/3)] = msdfgen::pixelFloatToByte(msdf[i+2]);
					}
					glm::ivec2 pixels = atlas.packTexture((uint32_t*)bytePixels.data(), glm::ivec2{ msdf.width(), msdf.height() }, c);
					character.textureCoordinates.x = static_cast<float>(pixels.x); //min X
					character.textureCoordinates.y = static_cast<float>(pixels.y); // max Y
				}
				//else
				{
					//unknownChars.push_back(c);
				}		
				font.m_characterInfos[c] = character;
			}
		}
		// finished with MSDFG font
		msdfgen::destroyFont(fontFace);
	}
	FT_Done_Face(face);
	FT_Done_FreeType(m_library);

	// let each special character be a space
	for (auto c : unknownChars)
	{
		if (c < 32 && !std::isspace(c))
			font.m_characterInfos[c] = font.m_characterInfos[' '];
	}

	// set the size to the bigger of the two if not we end up with ruined sizes for new line
	font.m_characterInfos['\n'].Size = font.m_characterInfos['\n'].Size.y == 0 ? 
		font.m_characterInfos['A'].Size : font.m_characterInfos['\n'].Size;

	//we now have the final buffer in the atlas
	glm::ivec2 texSize = atlas.textureSize; //square texture anyway
											//LOG_ENGINE_ERROR("tex size {0},{1}", texSize.x, texSize.y);
	for (auto& [c, glyph] : font.m_characterInfos)
	{
		glm::vec4 realTexcoord{ 0.0f };
		//calculating real texture coords
		realTexcoord.x = static_cast<float>(glyph.textureCoordinates.x) / texSize.x;
		realTexcoord.y = static_cast<float>(glyph.textureCoordinates.y) / texSize.y;
		realTexcoord.z = static_cast<float>(glyph.textureCoordinates.x + glyph.Size.x) / texSize.x;
		realTexcoord.w = static_cast<float>(glyph.textureCoordinates.y + glyph.Size.y) / texSize.y;
		glyph.textureCoordinates = realTexcoord;
	}

#endif


	return atlas;
}

void VulkanRenderer::LoadSubmesh(gfxModel& mdl,
	SubMesh& submesh,
	aiMesh* aimesh,
	ModelFileResource* modelFile
)
{
	submesh.name = aimesh->mName.C_Str();

	auto& vertices = modelFile->vertices;
	auto& indices = modelFile->indices;

	auto cacheVoffset = vertices.size();
	auto cacheIoffset = indices.size();

	bool once = false;

	vertices.reserve(vertices.size() + aimesh->mNumVertices);
	for (size_t i = 0; i < aimesh->mNumVertices; i++)
	{
		oGFX::Vertex vertex;
		vertex.pos = aiVector3D_to_glm(aimesh->mVertices[i]);
		if (aimesh->HasTextureCoords(0)) // does the mesh contain texture coordinates?
		{
			vertex.tex = glm::vec2{ aimesh->mTextureCoords[0][i].x, aimesh->mTextureCoords[0][i].y };
		}
		if (aimesh->HasNormals())
		{
			vertex.norm = aiVector3D_to_glm(aimesh->mNormals[i]);
		}
		if (aimesh->HasTangentsAndBitangents())
		{
			vertex.tangent = aiVector3D_to_glm(aimesh->mTangents[i]);
		}
		if (aimesh->HasVertexColors(0))
		{
			const auto& color = aimesh->mColors[0][i];
			vertex.col = glm::vec4{ color.r, color.g, color.b, color.a };
		}
		vertices.emplace_back(vertex);


		if (once == false) {
			auto tanlen = glm::dot(vertex.tangent, vertex.tangent);
			auto nlen = glm::dot(vertex.norm, vertex.norm);
			auto bt = glm::cross(vertex.tangent, vertex.norm);
			auto blen = glm::dot(bt, bt);

			// we can reject here if needed
			if (tanlen == 0.0f || nlen == 0.0f || blen == 0.0f) {
				once = true;
				std::string namestring;
				if (aimesh->mName.C_Str()) namestring = aimesh->mName.C_Str();
				printf("Model %s has vertex normal issues v[%llu]\n", namestring.c_str(), i);
				printf("Fixing vertex normals...\n");
				//__debugbreak();
			}
		}
	}

	uint32_t indicesCnt{};
	for (uint32_t i = 0; i < aimesh->mNumFaces; i++)
	{
		const aiFace& face = aimesh->mFaces[i];
		indicesCnt += face.mNumIndices;
		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}


		assert(face.mNumIndices == 3);
		std::array<oGFX::Vertex, 3> vert{
			vertices[face.mIndices[0]],
			vertices[face.mIndices[1]],
			vertices[face.mIndices[2]],
		};

		auto line0= vert[0].pos-vert[1].pos;
		auto line1= vert[2].pos-vert[2].pos;
		auto normal = glm::normalize(glm::cross(line0,line1));
		if (glm::dot(normal, normal) == 0) {
			__debugbreak();
		}

		for (uint32_t j = 0; j < face.mNumIndices; j++)
		{			
			if (glm::dot(vert[j].norm, vert[j].norm) == 0)
			{
				// fix zero normals
				vert[j].norm = normal;
			}
		}
	}

	submesh.vertexCount = aimesh->mNumVertices;
	submesh.baseVertex = static_cast<uint32_t>(cacheVoffset);
	submesh.indicesCount = indicesCnt;
	submesh.baseIndices = static_cast<uint32_t>(cacheIoffset);

	std::vector<glm::vec3> plainVertices;
	plainVertices.resize(aimesh->mNumVertices);
	for (size_t i = 0; i < plainVertices.size(); i++)
	{
		plainVertices[i] = vertices[cacheVoffset+i].pos; 
	}
	oGFX::BV::LarsonSphere(submesh.boundingSphere, plainVertices);
	//submesh.boundingSphere.radius *= 1.5f;
	//std::cout << "Sphere generated :" << submesh.name << " [" 
	//	<< submesh.boundingSphere.center.x << ", "
	//	<< submesh.boundingSphere.center.y << ", "
	//	<< submesh.boundingSphere.center.z << "] r: " << submesh.boundingSphere.radius << "\n";
}

ModelFileResource* VulkanRenderer::LoadMeshFromBuffers(
	std::vector<oGFX::Vertex>& vertex,
	std::vector<uint32_t>& indices,
	gfxModel* model
)
{
	uint32_t index = 0;
	ModelFileResource* m{ nullptr };

	if (model == nullptr)
	{
		std::scoped_lock s{ g_mut_globalModels };
		// this is a file-less object, generate a model for it
		index = static_cast<uint32_t>(g_globalModels.size());
		g_globalModels.emplace_back(gfxModel());
		model = &g_globalModels[index];

		model->indicesCount = static_cast<uint32_t>(indices.size());
		model->vertexCount = static_cast<uint32_t>(vertex.size());

		SubMesh sm;
		sm.baseIndices = static_cast<uint32_t>(0);
		sm.baseVertex = static_cast<uint32_t>(0);
		sm.indicesCount = static_cast<uint32_t>(indices.size());
		sm.vertexCount = static_cast<uint32_t>(vertex.size());

		std::vector<glm::vec3> plainVertices;
		plainVertices.resize(vertex.size());
		for (size_t i = 0; i < plainVertices.size(); i++)
		{
			plainVertices[i] = vertex[i].pos;
		}
		oGFX::BV::RitterSphere(sm.boundingSphere, plainVertices);

		model->m_subMeshes.push_back(sm);

		m = new ModelFileResource();
		Node* n = new Node{};
		m->sceneInfo = n;
		m->vertices = vertex;
		m->indices = indices;
		m->numSubmesh = 1;
		m->meshResource = index;

		model->cpuModel = m;
	}	

	// these offsets are using local offset based on the buffer.
	//std::cout << "Writing to vtx from data " << model->baseVertex
	//	<< " for " << model->vertexCount
	//	<<" total " << model->baseVertex+model->vertexCount
	//	<< " at GPU buffer " << g_GlobalMeshBuffers.VtxOffset
	//	<< std::endl;


	// now we update them to the global offset
	auto cmd = GetCommandBuffer();
	auto lam = [this,model,cmd]() 
	{
		//std::scoped_lock(g_mut_globalMeshBuffers);
		auto& indices = model->cpuModel->indices;
		auto& vertex = model->cpuModel->vertices;
		
		g_GlobalMeshBuffers.IdxBuffer.addWriteCommand(model->indicesCount, indices.data() + model->baseIndices,g_GlobalMeshBuffers.IdxOffset);
		g_GlobalMeshBuffers.VtxBuffer.addWriteCommand(model->vertexCount, vertex.data() + model->baseVertex,g_GlobalMeshBuffers.VtxOffset);

		model->baseIndices = g_GlobalMeshBuffers.IdxOffset;
		model->baseVertex = g_GlobalMeshBuffers.VtxOffset;

		g_GlobalMeshBuffers.IdxOffset += model->indicesCount;
		g_GlobalMeshBuffers.VtxOffset += model->vertexCount;

		if (model->skeleton)
		{
			auto& sk = model->skeleton;
			model->skinningWeightsOffset = this->g_skinningBoneWeights.size();
			this->g_skinningBoneWeights.resize(model->skinningWeightsOffset + sk->boneWeights.size());
			memcpy(this->g_skinningBoneWeights.data() + model->skinningWeightsOffset
				, sk->boneWeights.data()
				, sk->boneWeights.size() * sizeof(BoneWeight));
			
			gpuSkinningBoneWeightsBuffer.addWriteCommand(sk->boneWeights.size(), sk->boneWeights.data(), model->skinningWeightsOffset);
			
		}
	};

	{
		std::scoped_lock s{ g_mut_workQueue };
		g_workQueue.emplace_back(lam);
	}

	return m;
}

void VulkanRenderer::LoadBoneInformation(ModelFileResource& fileData,
	oGFX::Skeleton& skeleton,
	aiMesh& aimesh,
	std::vector<BoneWeight>& boneWeights,
	uint32_t& vCnt
)
{
	uint32_t numBones = 0;
	std::stringstream ss;
	for (size_t i = 0; i < aimesh.mNumBones; i++)
	{
		auto& currBone = aimesh.mBones[i];
		uint32_t boneIndex = 0;
		std::string boneName = currBone->mName.C_Str();


		if (fileData.strToBone.find(boneName) == fileData.strToBone.end())
		{
			// bone doesnt exist, allocate
			boneIndex = numBones++;

			oGFX::BoneInverseBindPoseInfo& invBindPoseInfo = skeleton.inverseBindPose.emplace_back(oGFX::BoneInverseBindPoseInfo{});

			// Map the name of this bone to this index. (map<string,int>)
			fileData.strToBone[boneName] = boneIndex;

			// Setup information
			// TODO: quaternions?
			invBindPoseInfo.transform = aiMat4_to_glm(currBone->mOffsetMatrix);
			invBindPoseInfo.boneIdx = boneIndex;
		}
		else
		{
			// bone already exists!
			boneIndex = fileData.strToBone[boneName];
		}

		// Add the bone weights for the vertices for the current bone
		for (size_t j = 0; j < currBone->mNumWeights; ++j)
		{
			const unsigned vertexID = currBone->mWeights[j].mVertexId + vCnt;
			const float weight = currBone->mWeights[j].mWeight;

			bool success = false;

			auto& vertex = boneWeights[vertexID];
			for (int slot = 0; slot < 4; ++slot)
			{
				if (vertex.boneIdx[slot] == boneIndex && boneIndex != 0) {
					success = true;
					break;
				}

				if (vertex.boneWeights[slot] == 0.0f)
				{
					vertex.boneIdx[slot] = boneIndex;
					vertex.boneWeights[slot] = weight;
					success = true;
					break;
				}
			}
#define NORMALIZE_BONE_WEIGHTS

			// Check if the number of weights is >4, just in case, since we dont support
			if (!success)
			{
				
				
				float sum;
#ifdef NORMALIZE_BONE_WEIGHTS
				uint32_t minBone = boneIndex;
				float minW = weight;
				for (size_t i = 0; i < 4; i++)
				{
					if (vertex.boneWeights[i] < minW)
					{
						std::swap(vertex.boneWeights[i], minW);
						std::swap(vertex.boneIdx[i], minBone);
					}
				}
				sum = 0.0f;
				for (size_t i = 0; i < 4; i++)
				{
					sum += vertex.boneWeights[i];
				}
				for (size_t i = 0; i < 4; i++)
				{
					vertex.boneWeights[i]*= (1.0f/sum);
				}
				sum = 0.0f;
				for (auto&[key,val] :  fileData.strToBone)
				{
					if (val == minBone)
					{
						ss << "Discarded weight: [" << key<<",\t"<< minW << "]" << std::endl;
						break;
					}
				}
				for (size_t i = 0; i < 4; i++)
				{
					sum += vertex.boneWeights[i];
				}
				//std::cout << "Final sum : [" << sum<< "]"<<std::endl;

#else
				//dump bone names
				std::cout << "Dumping bones...\n";
				std::cout << "Bone affected : " << currBone->mName.C_Str() << ",\t" << weight<< std::endl;
				for (size_t i = 0; i < 4; i++)
				{
					for (auto&[key,val] :  fileData.strToBone)
					{
						if (val == vertex.boneIdx[i])
						{
							std::cout << "Bone affected : " << key << ",\t"<<vertex.boneWeights[i] << std::endl;
							break;
						}
					}
				}				
				// Vertex already has 4 bone weights assigned.
				assert(false && "Bone weights >4 is not supported.");
#endif // NORMALIZE_BONE_WEIGHTS
			}
		}

	} // end bone for
	vCnt += aimesh.mNumVertices;
}

void VulkanRenderer::BuildSkeletonRecursive(ModelFileResource& fileData, aiNode* ainode, oGFX::BoneNode* parent, glm::mat4 parentXform,std::string prefix)
{
	std::string node_name{ ainode->mName.data };
	//std::stringstream ss;
	// TODO: quat ?
	glm::mat4x4 node_transform = parentXform * aiMat4_to_glm(ainode->mTransformation);
	oGFX::BoneNode* targetParent = parent;
	std::string cName = node_name.substr(node_name.find_last_of("_") + 1);
	oGFX::BoneNode* node = parent;

	//std::cout << "Loading " << node_name << std::endl;

	// Save the bone index
	bool bIsBoneNode = false;
	auto iter = fileData.strToBone.find(node_name);
	if (iter != fileData.strToBone.end())
	{
		//ss <<prefix<< "Creating bone " << node_name << std::endl;
		prefix += '\t';
		bIsBoneNode = true;
		node = new oGFX::BoneNode;
		node->mbIsBoneNode = true;
		node->mName = node_name;
		node->mpParent = targetParent;
		node->mModelSpaceLocal = node_transform;
		node->mModelSpaceGlobal= node_transform;
		node->m_BoneIndex = iter->second;
		if (targetParent)
		{
			targetParent->mChildren.push_back(node);
		}
		targetParent = node;
	}

	// Leaving this here to check the scale
	aiVector3D pos, scale;
	aiQuaternion qua;
	ainode->mTransformation.Decompose(scale, qua, pos);

	if ((scale.x - scale.y) > 0.0001f || (scale.x - scale.z) > 0.0001f)
	{
		static bool firstTime = true;
		if (firstTime)
		{
			// Non-uniform scale bone detected...
			__debugbreak();
			firstTime = false;
		}
	}

	// Recursion through all children
	for (size_t i = 0; i < ainode->mNumChildren; i++)
	{
		if (bIsBoneNode)
		{
			// we have collapsed the transforms start for new local transform
			BuildSkeletonRecursive(fileData, ainode->mChildren[i], targetParent,glm::mat4(1.0f),prefix);
		}
		else
		{
			BuildSkeletonRecursive(fileData, ainode->mChildren[i], targetParent,node_transform,prefix);
		}
	}
	//std::cout << ss.str();
}

const oGFX::Skeleton* VulkanRenderer::GetSkeleton(uint32_t modelID)
{
	return g_globalModels[modelID].skeleton;
}

oGFX::CPUSkeletonInstance* VulkanRenderer::CreateSkeletonInstance(uint32_t modelID)
{
	return oGFX::CreateCPUSkeleton(g_globalModels[modelID].skeleton);
}


VkCommandBuffer VulkanRenderer::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo= oGFX::vkutils::inits::commandBufferAllocateInfo(m_device.commandPoolManagers[getFrame()].m_commandpool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device.logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	std::cout << __FUNCTION__ << "Begin " << (size_t)commandBuffer << std::endl;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	std::cout << __FUNCTION__ << (size_t)commandBuffer << std::endl;
	vkQueueSubmit(m_device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

}

uint32_t VulkanRenderer::CreateTexture(uint32_t width, uint32_t height, unsigned char* imgData, bool generateMips)
{
	using namespace oGFX;
	FileImageData fileData;
	fileData.w = width;
	fileData.h = height;
	fileData.channels = 4;
	fileData.dataSize = (size_t)fileData.w * (size_t)fileData.h * (size_t)fileData.channels;
	fileData.imgData.resize(fileData.dataSize);

	VkBufferImageCopy copyRegion{};
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.bufferOffset = 0;
	copyRegion.imageExtent.width = fileData.w;
	copyRegion.imageExtent.height = fileData.h;
	copyRegion.imageExtent.depth = 1;
	fileData.mipInformation.push_back(copyRegion);

	memcpy(fileData.imgData.data(), imgData, fileData.dataSize);
	//fileData.imgData = imgData;

	auto ind = CreateTextureImage(fileData);
	
	auto lam = [this, ind]() {
		UpdateBindlessGlobalTexture(ind);
		};
	{
		std::scoped_lock s{ g_mut_workQueue };
		g_workQueue.emplace_back(lam);
	}

	//return location of set with texture
	return ind;

}

uint32_t VulkanRenderer::CreateTexture(const std::string& file)
{
	// Create texture image and get its location in array
	uint32_t textureImageLoc = CreateTextureImage(file);
	//create texture descriptor
	auto lam = [this, textureImageLoc]() {
		UpdateBindlessGlobalTexture(textureImageLoc);
	};
	{
		std::scoped_lock s{ g_mut_workQueue };
		g_workQueue.emplace_back(lam);
	}

	//return location of set with texture
	return textureImageLoc;
}

bool VulkanRenderer::ReloadTexture(uint32_t textureID,const std::string& file)
{
	// Create texture image and get its location in array
	
	// Load data
	oGFX::FileImageData imageData;
	imageData.Create(file);

	VkDeviceSize imageSize = imageData.dataSize;

	auto& texture = g_Textures[textureID];

	//vkDeviceWaitIdle(m_device.logicalDevice);
	UnloadTexture(textureID);
	texture.fromBuffer((void*)imageData.imgData.data(), imageSize, imageData.format, imageData.w, imageData.h, imageData.mipInformation, &m_device, m_device.graphicsQueue);
	
	GenerateMipmaps(texture);
	texture.updateDescriptor();

	UpdateBindlessGlobalTexture(textureID);

	//texture.Update((void*)imageData.imgData.data(), imageSize, imageData.format, imageData.w, imageData.h,imageData.mipInformation, &m_device, m_device.graphicsQueue);
	
	//setup imgui binding
	//g_imguiIDs.push_back(CreateImguiBinding(texture.sampler, texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

	return true;
}

void VulkanRenderer::UnloadTexture(uint32_t textureID)
{
	auto& texture = g_Textures[textureID];
	constexpr bool delayDeletion = true;
	texture.destroy(delayDeletion);
	texture.isValid = false;
}

VulkanRenderer::TextureInfo VulkanRenderer::GetTextureInfo(uint32_t handle)
{
	TextureInfo ti{
		g_Textures[handle].name,
		g_Textures[handle].width,
		g_Textures[handle].height,
		g_Textures[handle].format,
		g_Textures[handle].mipLevels,
	};
	
	return ti;
}

void VulkanRenderer::InitDebugBuffers()
{
	// TODO remove this
	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		g_DebugDrawVertexBufferGPU[i].Init(&m_device,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		g_DebugDrawIndexBufferGPU[i].Init(&m_device,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}
}

bool VulkanRenderer::UploadDebugDrawBuffers()
{
	PROFILE_SCOPED();

	// Seriously...
	if (g_DebugDrawVertexBufferCPU.empty() || g_DebugDrawIndexBufferCPU.empty())
	{
		// As long as the debug draw is not executed, clearing is not necessary.
		//g_DebugDrawVertexBufferGPU.clear();
		//g_DebugDrawIndexBufferGPU.clear();
		return false; // Do not run any debug draw render pass
	}

	g_DebugDrawVertexBufferGPU[getFrame()].reserve(g_DebugDrawVertexBufferCPU.size() ,m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);
	g_DebugDrawIndexBufferGPU[getFrame()].reserve(g_DebugDrawIndexBufferCPU.size(),m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);

	// Copy CPU debug draw buffers to the GPU
	auto cmd = GetCommandBuffer();
	g_DebugDrawVertexBufferGPU[getFrame()].writeToCmd(g_DebugDrawVertexBufferCPU.size() , g_DebugDrawVertexBufferCPU.data(),cmd,m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);
	g_DebugDrawIndexBufferGPU[getFrame()].writeToCmd(g_DebugDrawIndexBufferCPU.size() , g_DebugDrawIndexBufferCPU.data(),cmd,m_device.graphicsQueue, m_device.commandPoolManagers[getFrame()].m_commandpool);

	// Clear the CPU debug draw buffers for this frame
	g_DebugDrawVertexBufferCPU.clear();
	g_DebugDrawIndexBufferCPU.clear();

	// TODO: By default, drawing only lasts 1 frame. To handle with duration.

	return true;
}

void VulkanRenderer::UpdateUniformBuffers()
{		
	PROFILE_SCOPED();

	float height = static_cast<float>(windowPtr->m_height);
	float width = static_cast<float>(windowPtr->m_width);
	float ar = width / height;

	CB::FrameContextUBO frameContextUBO[2]{};
	if (currWorld)
	{
		for (size_t i = 0; i < currWorld->numCameras; i++)
		{
			auto& camera = currWorld->cameras[i];
			
			frameContextUBO[i].projection = camera.matrices.perspective;
			frameContextUBO[i].view = camera.matrices.view;
			frameContextUBO[i].viewProjection = frameContextUBO[i].projection * frameContextUBO[i].view;
			frameContextUBO[i].inverseViewProjection = glm::inverse(frameContextUBO[i].viewProjection);
			frameContextUBO[i].inverseView = glm::inverse(frameContextUBO[i].view);
			frameContextUBO[i].inverseProjection = glm::inverse(frameContextUBO[i].projection);
			frameContextUBO[i].cameraPosition = glm::vec4(camera.m_position,1.0);
			frameContextUBO[i].renderTimer.x = renderClock;
			frameContextUBO[i].renderTimer.y = std::sin(renderClock * glm::pi<float>());
			frameContextUBO[i].renderTimer.z = std::cos(renderClock * glm::pi<float>());
			frameContextUBO[i].renderTimer.w = 0.0f; // unused
		
			// These variables area only to speedup development time by passing adjustable values from the C++ side to the shader.
			// Bind this to every single shader possible.
			// Remove this upon shipping the final product.
			{			
				frameContextUBO[i].vector4_values0 = m_ShaderDebugValues.vector4_values0;
				frameContextUBO[i].vector4_values1 = m_ShaderDebugValues.vector4_values1;
				frameContextUBO[i].vector4_values2 = m_ShaderDebugValues.vector4_values2;
				frameContextUBO[i].vector4_values3 = m_ShaderDebugValues.vector4_values3;
				frameContextUBO[i].vector4_values4 = m_ShaderDebugValues.vector4_values4;
				frameContextUBO[i].vector4_values5 = m_ShaderDebugValues.vector4_values5;
				frameContextUBO[i].vector4_values6 = m_ShaderDebugValues.vector4_values6;
				frameContextUBO[i].vector4_values7 = m_ShaderDebugValues.vector4_values7;
				frameContextUBO[i].vector4_values8 = m_ShaderDebugValues.vector4_values8;
				frameContextUBO[i].vector4_values9 = m_ShaderDebugValues.vector4_values9;
			}
		}
	}

	

	void *data;
	auto alignedRange = oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), m_device.properties.limits.minUniformBufferOffsetAlignment);
	// map whole aligned range
	vmaMapMemory(m_device.m_allocator, vpUniformBuffer[getFrame()].alloc, &data);

	memcpy(data, &frameContextUBO[0], sizeof(CB::FrameContextUBO));
	// Maybe dont need to check align range for this
	memcpy((char*)data+alignedRange, &frameContextUBO[1], sizeof(CB::FrameContextUBO));

	//VkMappedMemoryRange memRng{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
	//memRng.memory = vpUniformBuffer[getFrame()];
	//memRng.offset =  0;
	//memRng.size =  numCameras*alignedRange;
	//VK_CHK(vkFlushMappedMemoryRanges(m_device.logicalDevice, 1, &memRng));

	vmaUnmapMemory(m_device.m_allocator, vpUniformBuffer[getFrame()].alloc);
}

uint32_t VulkanRenderer::CreateTextureImage(const std::string& fileName)
{
	//Load image file
	oGFX::FileImageData imageData;
	imageData.Create(fileName);
	
//#define OVERIDE_TEXTURE_SIZE_ONE
#ifdef OVERIDE_TEXTURE_SIZE_ONE
	imageData.w = 1;
	imageData.h = 1;
	imageData.dataSize = 1 * 1 * 4;
	imageData.mipInformation.front().imageExtent = VkExtent3D{ 1,1,1 };
#endif // OVERIDE_TEXTURE_SIZE_ONE

	//int width{}, height{};
	//VkDeviceSize imageSize;
	//unsigned char *imageData = oGFX::LoadTextureFromFile(fileName, width, height, imageSize);

	auto value = CreateTextureImage(imageData);

	imageData.Free();
	return value;
}

uint32_t VulkanRenderer::CreateCubeMapTexture(const std::string& folderName)
{
	//Load image file
	oGFX::FileImageData imageData;
	imageData.CreateCube(folderName);

	OO_ASSERT(imageData.imgData.size());

	//#define OVERIDE_TEXTURE_SIZE_ONE
#ifdef OVERIDE_TEXTURE_SIZE_ONE
	imageData.w = 1;
	imageData.h = 1;
	imageData.dataSize = 1 * 1 * 4;
	imageData.mipInformation.front().imageExtent = VkExtent3D{ 1,1,1 };
#endif // OVERIDE_TEXTURE_SIZE_ONE

	auto lam = [this, imageInfo = imageData, highVal = imageData.highestColValue]() {

		g_cubeMap.name = imageInfo.name;
		g_cubeMap.highestColValue = highVal;
		g_cubeMap.fromBuffer((void*)imageInfo.imgData.data(), imageInfo.dataSize, imageInfo.format, imageInfo.w, imageInfo.h, imageInfo.mipInformation, &m_device, m_device.graphicsQueue);
		// dont generate for now
		GenerateMipmaps(g_cubeMap);


		VkCommandBuffer cmdlist = GetCommandBuffer();
		GenerateRadianceMap(cmdlist, g_cubeMap);
		GeneratePrefilterMap(cmdlist, g_cubeMap);
		GenerateBRDFLUT(cmdlist, g_brdfLUT);
		SubmitSingleCommandAndWait(cmdlist);

		GenerateMipmaps(g_radianceMap);

		};
	{
		std::scoped_lock s{ g_mut_workQueue };
		g_workQueue.emplace_back(lam);
	}
	
	imageData.Free();
	return 0;
}

uint32_t VulkanRenderer::CreateTextureImage(const oGFX::FileImageData& imageInfo)
{
	VkDeviceSize imageSize = imageInfo.dataSize;

	totalTextureSizeLoaded += imageSize;

	auto indx = [this]{
		// mutex
		std::scoped_lock s(g_mut_Textures);
		auto indx = g_Textures.size();
		g_Textures.push_back(vkutils::Texture2D());
		g_imguiIDs.push_back({});

		uint32_t texSiz = (uint32_t)g_Textures.size();
		uint32_t imguiSiz = (uint32_t)g_imguiIDs.size();

		assert(g_Textures.size() == g_imguiIDs.size());

		return indx;
	}();

	auto lam = [this, indx, imageInfo]() {
		auto& texture = g_Textures[indx];

		texture.name = imageInfo.name;
		texture.fromBuffer((void*)imageInfo.imgData.data(), imageInfo.dataSize, imageInfo.format, imageInfo.w, imageInfo.h,imageInfo.mipInformation, &m_device, m_device.graphicsQueue);

		GenerateMipmaps(texture);
		//setup imgui binding
		g_imguiIDs[indx] = CreateImguiBinding(samplerManager.GetDefaultSampler(), &texture);
		
	};
	{
		std::scoped_lock s{ g_mut_workQueue };
		g_workQueue.emplace_back(lam);
	}

	// Return index of new texture image
	return static_cast<uint32_t>(indx);
}

uint32_t VulkanRenderer::CreateTextureImageImmediate(const oGFX::FileImageData& imageInfo)
{
	VkDeviceSize imageSize = imageInfo.dataSize;

	totalTextureSizeLoaded += imageSize;

	auto indx = [this]{
		// mutex
		std::scoped_lock s(g_mut_Textures);
		auto indx = g_Textures.size();
		g_Textures.push_back(vkutils::Texture2D());
		g_imguiIDs.push_back({});

		uint32_t texSiz = (uint32_t)g_Textures.size();
		uint32_t imguiSiz = (uint32_t)g_imguiIDs.size();

		assert(g_Textures.size() == g_imguiIDs.size());

		return indx;
	}();

	
	auto& texture = g_Textures[indx];

	texture.fromBuffer((void*)imageInfo.imgData.data(), imageInfo.dataSize, imageInfo.format, imageInfo.w, imageInfo.h,imageInfo.mipInformation, &m_device, m_device.graphicsQueue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	texture.name = imageInfo.name;

	//setup imgui binding
	g_imguiIDs[indx] = CreateImguiBinding(samplerManager.GetDefaultSampler(), &texture);
	

	// Return index of new texture image
	return static_cast<uint32_t>(indx);
}


VkPipelineShaderStageCreateInfo VulkanRenderer::LoadShader(VulkanDevice& device,const std::string& fileName, VkShaderStageFlagBits stage)
{
	// SHADER STAGE CREATION INFORMATION
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	//shader stage name 
	shaderStageCreateInfo.stage = stage;

	//build shader modules to link to pipeline
	//read in SPIR-V code of shaders
	auto shaderCode = oGFX::readFile(fileName);
	VkShaderModule shaderModule = oGFX::CreateShaderModule(device,shaderCode);

	//shader module to be used by stage
	shaderStageCreateInfo.module = shaderModule;
	//pointer to the shader starting function
	shaderStageCreateInfo.pName = "main";

	assert(shaderStageCreateInfo.module != VK_NULL_HANDLE);
	return shaderStageCreateInfo;
}

uint32_t VulkanRenderer::UpdateBindlessGlobalTexture(uint32_t textureID)
{

	auto& texture = g_Textures[textureID];
	texture.descriptor.sampler = samplerManager.GetDefaultSampler();
	std::vector<VkWriteDescriptorSet> writeSets
	{
		oGFX::vkutils::inits::writeDescriptorSet(descriptorSet_bindless, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texture.descriptor),
	};

	//auto index = static_cast<uint32_t>(samplerDescriptorSets.size());
	//samplerDescriptorSets.push_back(globalSamplers); // Wtf???
	writeSets[0].dstArrayElement = textureID;

	vkUpdateDescriptorSets(m_device.logicalDevice, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);
	texture.isValid = true;

	return textureID;
}

void VulkanRenderer::InitDefaultPrimatives()
{
	{
		DefaultMesh dm = CreateDefaultCubeMesh();
		def_cube.reset(LoadMeshFromBuffers(dm.m_VertexBuffer, dm.m_IndexBuffer, nullptr));
	}
	{
		DefaultMesh pm = CreateDefaultPlaneXZMesh();
		def_plane.reset(LoadMeshFromBuffers(pm.m_VertexBuffer, pm.m_IndexBuffer, nullptr));
	}
	{
		DefaultMesh sm = CreateDefaultPlaneXYMesh();
		def_sprite.reset(LoadMeshFromBuffers(sm.m_VertexBuffer, sm.m_IndexBuffer, nullptr));
	}
	{
		def_font.reset(LoadFont("defaultAsset/Roboto-Medium.ttf"));
	}
}

ImTextureID VulkanRenderer::GetImguiID(uint32_t textureID)
{
	//std::string s("Curr " + std::to_string(g_imguiIDs.size()) + " Access " + std::to_string(textureID) +"\n");
	//std::cout << s;
	return g_imguiIDs[textureID];
}

ImTextureID VulkanRenderer::CreateImguiBinding(VkSampler s, vkutils::Texture* tex)
{
	auto& vr = *VulkanRenderer::get();
	if (vr.m_imguiInitialized == false)
	{
		return 0;
	}	
	VkDescriptorSet dsc = ImGui_ImplVulkan_AddTexture(s, tex->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	// add to mapping
	std::scoped_lock l{ vr.g_mute_imguiTextureMap };
	ImTextureID id = (ImTextureID)dsc;
	vr.g_imguiToTexture[id] = tex;
	return dsc;
}

int Win32SurfaceCreator(ImGuiViewport* vp, ImU64 device, const void* allocator, ImU64* outSurface)
{
	Window newWindow;
	newWindow.Init();
	
	vp->Size = ImVec2{ (float)newWindow.m_width,(float)newWindow.m_height };
	vp->PlatformHandle = (void*)newWindow.GetRawHandle();
	vp->PlatformHandleRaw = (void*)newWindow.GetRawHandle();
	
	*outSurface = Window::SurfaceFormat;
	//*outSurface = 
	return 1;
}

// Helper function to set Viewport & Scissor to the default window full extents.
void SetDefaultViewportAndScissor(VkCommandBuffer commandBuffer, VkViewport* vp, VkRect2D* sc)
{
	auto& vr = *VulkanRenderer::get();
    auto* windowPtr = vr.windowPtr;
    const float vpHeight = (float)vr.m_swapchain.swapChainExtent.height;
    const float vpWidth = (float)vr.m_swapchain.swapChainExtent.width;
    VkViewport viewport = { 0.0f, vpHeight, vpWidth, -vpHeight, 0.0f, 1.0f };
    VkRect2D scissor = { {0, 0}, {uint32_t(windowPtr->m_width), uint32_t(windowPtr->m_height) } };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	if (vp) *vp = viewport;
	if (sc) *sc = scissor;
}

// Helper function to draw a Full Screen Quad, without binding vertex and index buffers.
void DrawFullScreenQuad(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void DrawIndexedIndirect(
	VkCommandBuffer commandBuffer,
	VkBuffer buffer,
	VkDeviceSize offset,
	uint32_t drawCount,
	uint32_t stride)
{
    if (VulkanRenderer::get()->m_device.enabledFeatures.multiDrawIndirect)
    {
        vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, sizeof(oGFX::IndirectCommand));
    }
    else
    {
        // If MDI not supported, still use IDCB but draw one by one per instance instead of one MDI for all instances (ie count = 1)
        for (uint32_t i = 0; i < drawCount; ++i)
        {
            vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset + i * sizeof(oGFX::IndirectCommand), 1, sizeof(oGFX::IndirectCommand));
        }
    }
}
