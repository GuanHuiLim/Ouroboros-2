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
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_win32.h>


#include "../shaders/shared_structs.h"

#include "GfxRenderpass.h"

#include "renderpass/DeferredCompositionRenderpass.h"
#include "renderpass/GBufferRenderPass.h"
#include "renderpass/DebugRenderpass.h"
#include "renderpass/ShadowPass.h"
#include "renderpass/SSAORenderPass.h"
#include "renderpass/ForwardParticlePass.h"
#include "renderpass/BloomPass.h"
#if defined (ENABLE_DECAL_IMPLEMENTATION)
	#include "renderpass/ForwardDecalRenderpass.h"
#endif

#include "DefaultMeshCreator.h"

#include "GraphicsBatch.h"
#include "FramebufferBuilder.h"
#include "DelayedDeleter.h"

#include "IcoSphereCreator.h"

#include "Profiling.h"

#include <vector>
#include <set>
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>
#include <random>
#include <filesystem>
#include <sstream>

VulkanRenderer* VulkanRenderer::s_vulkanRenderer{ nullptr };

// vulkan debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	
	// Ignore all performance related warnings for now..
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT && !(messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT))
	{
		int x;
		std::cerr << pCallbackData->pMessage << std::endl<< std::endl;
		//assert(false); temp comment out
		x=5; // for breakpoint
	}

	return VK_FALSE;
}

int VulkanRenderer::ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{
		auto* hdl = viewport->PlatformHandle;
		(void)vk_allocator;
		Window temp;
		temp.rawHandle = hdl;
		try
		{
			VulkanRenderer::get()->m_instance.CreateSurface(temp, *(VkSurfaceKHR*)out_vk_surface);
		}
		catch (std::runtime_error e)
		{
			temp.rawHandle = nullptr;
			return 1;
		}
		temp.rawHandle = nullptr;
		return 0;
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

	

	for (size_t i = 0; i < renderTargets.size(); i++)
	{
		if (renderTargets[i].texture.image)
		{
			renderTargets[i].texture.destroy();
		}
		if (renderTargets[i].depth.image)
		{
			renderTargets[i].depth.destroy();
		}
	}

	RenderPassDatabase::Shutdown();

#ifdef _DEBUG
	DestroyDebugMessenger();
#endif // _DEBUG

	fbCache.Cleanup();

	DestroyRenderBuffers();

	samplerManager.Shutdown();

	gpuTransformBuffer.destroy();

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
		vkDestroyBuffer(m_device.logicalDevice, vpUniformBuffer[i], nullptr);
		vkFreeMemory(m_device.logicalDevice, vpUniformBufferMemory[i], nullptr);
	}

	for (size_t i = 0; i < drawFences.size(); i++)
	{
		vkDestroyFence(m_device.logicalDevice, drawFences[i], nullptr);
		vkDestroySemaphore(m_device.logicalDevice, renderFinished[i], nullptr);
		vkDestroySemaphore(m_device.logicalDevice, imageAvailable[i], nullptr);
	}

	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::defaultPSOLayout, nullptr);
	vkDestroyPipelineLayout(m_device.logicalDevice, PSOLayoutDB::PSO_fullscreenBlitLayout, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, pso_utilFullscreenBlit, nullptr);

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

void VulkanRenderer::Init(const oGFX::SetupInfo& setupSpecs, Window& window)
{
	try
	{	
		CreateInstance(setupSpecs);

#ifdef _DEBUG
		CreateDebugCallback();
#endif // _DEBUG

		CreateSurface(setupSpecs,window);
		// set surface for imgui
		Window::SurfaceFormat = (uint64_t)window.SurfaceFormat;

		AcquirePhysicalDevice(setupSpecs);
		CreateLogicalDevice(setupSpecs);

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

		CreateDefaultRenderpass();
		CreateUniformBuffers();
		CreateDefaultDescriptorSetLayout();

		fbCache.Init(m_device.logicalDevice);

		gpuTransformBuffer.Init(&m_device,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
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

		// Initialize all sampler objects
		samplerManager.Init();

		// Calls "Init()" on all registered render passes. Order is not guarunteed.
		auto rpd = RenderPassDatabase::Get();
		GfxRenderpass* ptr;
		ptr = new ShadowPass;
		rpd->RegisterRenderPass(ptr);
		 ptr = new DebugDrawRenderpass;
		rpd->RegisterRenderPass(ptr);
		 ptr = new GBufferRenderPass;
		rpd->RegisterRenderPass(ptr);
		 ptr = new DeferredCompositionRenderpass;
		rpd->RegisterRenderPass(ptr);
		ptr = new SSAORenderPass;
		rpd->RegisterRenderPass(ptr);
		ptr = new ForwardParticlePass;
		rpd->RegisterRenderPass(ptr);
		ptr = new BloomPass;
		rpd->RegisterRenderPass(ptr);
#if defined (ENABLE_DECAL_IMPLEMENTATION)
		ptr = new ForwardDecalRenderpass;
		rpd->RegisterRenderPass(ptr);
#endif

		CreateFramebuffers();

		CreateCommandBuffers();
		CreateDescriptorPool();

		g_Textures.reserve(2048);

		uint32_t whiteTexture = 0xFFFFFFFF; // ABGR
		uint32_t blackTexture = 0xFF000000; // ABGR
		uint32_t normalTexture = 0xFFFF8080; // ABGR
		uint32_t pinkTexture = 0xFFA040A0; // ABGR

		whiteTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&whiteTexture));
		blackTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&blackTexture));
		normalTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&normalTexture));
		pinkTextureID = CreateTexture(1, 1, reinterpret_cast<unsigned char*>(&pinkTexture));
		
		RenderPassDatabase::InitAllRegisteredPasses();

		
		auto& shadowTexture =RenderPassDatabase::GetRenderPass<ShadowPass>()->shadow_depth;
		shadowTexture.updateDescriptor();

		CreateSynchronisation();

		InitDebugBuffers();
		
		InitDefaultPrimatives();

		PROFILE_INIT_VULKAN(&m_device.logicalDevice, &m_device.physicalDevice, &m_device.graphicsQueue, (uint32_t*)&m_device.queueIndices.graphicsFamily, 1, nullptr);
	}
	catch (const std::exception& e)
	{
		std::cout << "VulkanRenderer::Init failed: " << e.what() << std::endl;
		throw e; // ???? wtf?
	}
	catch (...)
	{
		std::cout << "caught something unexpected" << std::endl;
	}
}

void VulkanRenderer::CreateInstance(const oGFX::SetupInfo& setupSpecs)
{
	try
	{
		m_instance.Init(setupSpecs);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "Caught something, re-throwing from : " << __FUNCSIG__ << std::endl;
		throw;
	}
}

class SDL_Window;
void VulkanRenderer::CreateSurface(const oGFX::SetupInfo& setupSpecs, Window& window)
{
    windowPtr = &window;
	if (window.m_type == Window::WindowType::SDL2)
	{
		assert(setupSpecs.SurfaceFunctionPointer); // Surface pointer doesnt work	
		std::function<void()> fn = setupSpecs.SurfaceFunctionPointer;
		fn();
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
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;//descripts what to do with attachment before rendering
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//describes what to do with attachment after rendering
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //describes what do with with stencil before rendering
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //describes what do with with stencil before rendering

	//frame buffer data will be stored as image, but images can be given different data layouts
	//to give optimal use for certain operations
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //image data layout before render pass starts
	//colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //image data layout aftet render pass ( to change to)
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; //image data layout aftet render pass ( to change to)
	
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


	renderpassAttachments[0].format = G_HDR_FORMAT;
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
		vpBufferInfo.buffer = vpUniformBuffer[i];	// buffer to get data from
		vpBufferInfo.offset = 0;					// position of start of data
		vpBufferInfo.range = sizeof(CB::FrameContextUBO);// size of data

		DescriptorBuilder::Begin(&DescLayoutCache, &descAllocs[swapchainIdx])
			.BindBuffer(0, &vpBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
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
		throw std::runtime_error("Failed to create a descriptor set layout!");
	}
}

void VulkanRenderer::FullscreenBlit(VkCommandBuffer inCmd, vkutils::Texture2D& src, VkImageLayout srcFinal, vkutils::Texture2D& dst, VkImageLayout dstFinal) 
{
	
	const VkCommandBuffer cmdlist = inCmd;
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("Blit");

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = { 0.0f,0.0f,0.0f,0.0f };

	//Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass_default_noDepth.pass;                  //render pass to begin
	renderPassBeginInfo.renderArea.offset = { 0,0 };                                     //start point of render pass in pixels
	glm::uvec2 renderSize = glm::vec2{ dst.width,dst.height };

	renderPassBeginInfo.renderArea.extent = VkExtent2D{ renderSize.x,renderSize.y }; //size of region to run render pass on (Starting from offset)
	renderPassBeginInfo.pClearValues = clearValues.data();                               //list of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	VkFramebuffer currentFB;
	FramebufferBuilder::Begin(&fbCache)
		.BindImage(&dst)
		//.BindImage(&vr.renderTargets[vr.renderTargetInUseID].depth) //no depth
		.Build(currentFB, renderPass_default_noDepth);
	renderPassBeginInfo.framebuffer = currentFB;

	vkutils::TransitionImage(cmdlist, src, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	VkDescriptorImageInfo texdesc = oGFX::vkutils::inits::descriptorImageInfo(
		GfxSamplerManager::GetSampler_SSAOEdgeClamp(),
		src.view,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	rhi::CommandList cmd{ cmdlist ,"Fullscreen Blit"};
	std::array<VkViewport, 1>viewports{ VkViewport{0,renderSize.y * 1.0f,renderSize.x * 1.0f,renderSize.y * -1.0f} };
	cmd.SetViewport(0, viewports.size(), viewports.data());
	VkRect2D scissor{ {}, {renderSize.x,renderSize.y} };
	cmd.SetScissor(scissor);



	// create descriptor for this pass
	DescriptorBuilder::Begin(&DescLayoutCache, &descAllocs[swapchainIdx])
		.BindImage(1, &texdesc, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build(descriptorSet_fullscreenBlit, SetLayoutDB::util_fullscreenBlit);

	cmd.BindPSO(pso_utilFullscreenBlit);

	SSAOPC pc{};
	VkPushConstantRange range;
	range.offset = 0;
	range.size = sizeof(SSAOPC);

	cmd.SetPushConstant(PSOLayoutDB::PSO_fullscreenBlitLayout, range, &pc);

	uint32_t dynamicOffset = static_cast<uint32_t>(renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO),
		m_device.properties.limits.minUniformBufferOffsetAlignment));
	cmd.BindDescriptorSet(PSOLayoutDB::PSO_fullscreenBlitLayout, 0,
		std::array<VkDescriptorSet, 1>
		{
			descriptorSet_fullscreenBlit,
		},
		0
	);

	cmd.DrawFullScreenQuad();
	vkCmdEndRenderPass(cmdlist);

	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		src.image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		src.currentLayout,
		srcFinal,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	src.currentLayout = srcFinal;

	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmdlist,
		dst.image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		dst.currentLayout,
		dstFinal,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
	dst.currentLayout = dstFinal;
}

void VulkanRenderer::BlitFramebuffer(VkCommandBuffer cmd, vkutils::Texture2D& src,VkImageLayout srcFinal, vkutils::Texture2D& dst,VkImageLayout dstFinal)
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
		dst.image,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		dst.currentLayout,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmd,
		src.image,
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
			src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
			src.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dst.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageCopyRegion);
	}

	// Transition destination image to general layout, which is the required layout for mapping the image memory later on
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		cmd,
		dst.image,
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
		src.image,
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
	VkDescriptorSetLayoutBinding binding = oGFX::vkutils::inits::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
	VkDescriptorSetLayoutCreateInfo dci = oGFX::vkutils::inits::descriptorSetLayoutCreateInfo(&binding,1);
	SetLayoutDB::util_fullscreenBlit= DescLayoutCache.CreateDescriptorLayout(&dci);

	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &SetLayoutDB::util_fullscreenBlit;
	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &PSOLayoutDB::PSO_fullscreenBlitLayout));
	VK_NAME(m_device.logicalDevice, "fullscreenPSOLayout", PSOLayoutDB::PSO_fullscreenBlitLayout);
	
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
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT , VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::PSO_fullscreenBlitLayout, renderPass_default_noDepth.pass);
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
	pipelineCI.renderPass = renderPass_default_noDepth.pass;
	pipelineCI.layout = PSOLayoutDB::PSO_fullscreenBlitLayout;
	colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_utilFullscreenBlit));
	VK_NAME(m_device.logicalDevice, "pso_blit", pso_utilFullscreenBlit);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr); // destroy vert
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr); // destroy fragment

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
			throw std::runtime_error("Failed to create a Framebuffer!");
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
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = m_device.commandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;	// VK_COMMAND_BUFFER_LEVEL_PRIMARY : buffer you submit directly to queue, cant be called  by other buffers
															//VK_COMMAND_BUFFER_LEVEL_SECONDARY :  buffer cant be called directly, can be called from other buffers via "vkCmdExecuteCommands" when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	//allocate command buffers and place handles in array of buffers
	VkResult result = vkAllocateCommandBuffers(m_device.logicalDevice, &cbAllocInfo, commandBuffers.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Command Buffers!");
	}
}

void VulkanRenderer::ResizeDeferredFB()
{


}

void VulkanRenderer::SetWorld(GraphicsWorld* world)
{
	// force a sync here
	vkDeviceWaitIdle(m_device.logicalDevice);
	currWorld = world;

	if (currWorld)
	{
		
	}

}

void VulkanRenderer::InitWorld(GraphicsWorld* world)
{
	assert(world && "dont pass nullptr");

	for (uint32_t x = 0; x < world->numCameras; ++x)
	{
		auto& wrdID = world->targetIDs[x];
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
			if (image.image == VK_NULL_HANDLE)
			{
				image.name = "GW_"+std::to_string(wrdID)+":COL";
				image.forFrameBuffer(&m_device, G_HDR_FORMAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
					m_swapchain.swapChainExtent.width,m_swapchain.swapChainExtent.height);
						
			}
			if (image.image&&renderTargets[wrdID].imguiTex == 0)
			{
				renderTargets[wrdID].imguiTex = CreateImguiBinding(samplerManager.GetDefaultSampler(), image.view, image.imageLayout);				
			}
			auto& depth =  renderTargets[wrdID].depth;
			if (depth.image == VK_NULL_HANDLE)
			{
				depth.name = "GW_"+std::to_string(wrdID)+":DEPTH";
				depth.forFrameBuffer(&m_device, G_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
					m_swapchain.swapChainExtent.width,m_swapchain.swapChainExtent.height);
				
				//world->imguiID[0] = CreateImguiBinding(samplerManager.GetDefaultSampler(), depth.view, depth.imageLayout);
			}

			//assignment 
			world->imguiID [x] = renderTargets[wrdID].imguiTex;
		}		
	}	
	world->initialized = true;
}

void VulkanRenderer::DestroyWorld(GraphicsWorld* world)
{
	assert(world && "dont pass nullptr");
	assert(world->initialized && "World should exist dont destroy non-init world");
	for (uint32_t x = 0; x < world->numCameras; ++x)
	{
		auto& wrdID = world->targetIDs[x];
		renderTargets[wrdID].inUse = false;
		wrdID = -1;
		numAllocatedCameras--;
	}	
	world->initialized = false;
}

int32_t VulkanRenderer::GetPixelValue(uint32_t fbID, glm::vec2 uv)
{

	uv = glm::clamp(uv, { 0.0,0.0 }, { 1.0,1.0 });

	// Bad but only editor uses this
	auto& physicalDevice = m_device.physicalDevice;
	auto& device = m_device.logicalDevice;
	vkQueueWaitIdle(m_device.graphicsQueue);
	vkDeviceWaitIdle(device);



	bool supportsBlit = true;
	// Check blit support for source and destination
	VkFormatProperties formatProps;

	
	auto& target = RenderPassDatabase::GetRenderPass<GBufferRenderPass>()->attachments[GBufferAttachmentIndex::ENTITY_ID];
	if (target.currentLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		return -1;

	// Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
	vkGetPhysicalDeviceFormatProperties(physicalDevice, target.format, &formatProps);
	if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
		supportsBlit = false;
	}

	// Check if the device supports blitting to linear images
	vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R32_SINT, &formatProps);
	if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
		supportsBlit = false;
	}

	// Source for the copy is the last rendered swapchain image
	VkImage srcImage = target.image;

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
	VkImage dstImage;
	VK_CHK(vkCreateImage(device, &imageCreateCI, nullptr, &dstImage));
	VK_NAME(device, "COPY_DST_EDITOR_ID", dstImage);
	// Create memory to back up the image
	VkMemoryRequirements memRequirements;
	VkMemoryAllocateInfo memAllocInfo(oGFX::vkutils::inits::memoryAllocateInfo());
	VkDeviceMemory dstImageMemory;
	vkGetImageMemoryRequirements(device, dstImage, &memRequirements);
	memAllocInfo.allocationSize = memRequirements.size;
	// Memory must be host visible to copy from
	memAllocInfo.memoryTypeIndex = oGFX::FindMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits
		, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	VK_CHK(vkAllocateMemory(device, &memAllocInfo, nullptr, &dstImageMemory));
	VK_CHK(vkBindImageMemory(device, dstImage, dstImageMemory, 0));

	// Do the actual blit from the swapchain image to our host visible destination image
	
	VkCommandBuffer copyCmd = beginSingleTimeCommands();
	VK_NAME(device, "COPY_DST_EDITOR_ID_CMD_LIST", copyCmd);
	// Transition destination image to transfer destination layout
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		copyCmd,
		dstImage,
		0,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	// Transition swapchain image from present to transfer source layout
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		target.currentLayout,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	// If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
	if (supportsBlit)
	{
		// Define the region to blit (we will blit the whole swapchain image)
		VkOffset3D blitSize;
		blitSize.x = target.width;
		blitSize.y = target.height;
		blitSize.z = 1;
		VkImageBlit imageBlitRegion{};
		imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlitRegion.srcSubresource.layerCount = 1;
		imageBlitRegion.srcOffsets[1] = blitSize;
		imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBlitRegion.dstSubresource.layerCount = 1;
		imageBlitRegion.dstOffsets[1] = blitSize;

		// Issue the blit command
		vkCmdBlitImage(
			copyCmd,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
		imageCopyRegion.extent.width = target.width;
		imageCopyRegion.extent.height = target.height;
		imageCopyRegion.extent.depth = 1;

		// Issue the copy command
		vkCmdCopyImage(
			copyCmd,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&imageCopyRegion);
	}

	// Transition destination image to general layout, which is the required layout for mapping the image memory later on
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		copyCmd,
		dstImage,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	// Transition back the format after the blit is done
	oGFX::vkutils::tools::insertImageMemoryBarrier(
		copyCmd,
		srcImage,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		target.currentLayout,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

	vkEndCommandBuffer(copyCmd);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;
	//submitInfo.waitSemaphoreCount = 1; //number of semaphores to wait on
	//submitInfo.pWaitSemaphores = &readyForCopy[(currentFrame+MAX_FRAME_DRAWS+1) % MAX_FRAME_DRAWS]; //list of semaphores to wait on
	//VkPipelineStageFlags waitStages[] = {
	//	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	//};
	//submitInfo.pWaitDstStageMask = waitStages; //stages to check semapheres at


	vkQueueSubmit(m_device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device.graphicsQueue);
	vkFreeCommandBuffers(m_device.logicalDevice, m_device.commandPool, 1, &copyCmd);


	// Get layout of the image (including row pitch)
	VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
	VkSubresourceLayout subResourceLayout;
	vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

	// Map image memory so we can start copying from it
	const char* data;
	VK_CHK(vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)& data));
	data += subResourceLayout.offset;

	
	// If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
	bool colorSwizzle = false;
	// Check if source is BGR
	// Note: Not complete, only contains most common and basic BGR surface formats for demonstration purposes
	if (!supportsBlit)
	{
		std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
		//colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), swapChain.colorFormat) != formatsBGR.end());
	}

	glm::uvec2 pixels = glm::uvec2{ target.width * uv.x,target.height * (uv.y) };
	pixels = glm::clamp(pixels, { 0,0 }, { target.width-1,target.height-1 });
	uint32_t indx = (pixels.x + pixels.y * target.width);
	int32_t value = ((int32_t*)data)[indx];
	//uint32_t value = ((uint32_t*)data)[pixels.x * (pixels.y * subResourceLayout.rowPitch)];

	// Clean up resources
	vkUnmapMemory(device, dstImageMemory);
	vkFreeMemory(device, dstImageMemory, nullptr);
	vkDestroyImage(device, dstImage, nullptr);

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

	m_numShadowcastLights = 0;
	int32_t gridIdx = 0;

	std::vector<LocalLightInstance> spotLights;
	auto& lights = currWorld->GetAllOmniLightInstances();
	spotLights.reserve(lights.size());
	int viewIter{};
	for (auto& e : lights)
	{
		LocalLightInstance si;
		if (e.info.x > 0)
		{
			
			e.info.y = gridIdx;			
			if (e.info.x == 1)
			{
				// loop through all faces
				for (size_t i = 0; i < 6; i++)
				{
					++m_numShadowcastLights;
					si.view[i] = e.view[i];
					++gridIdx;
				}
			}
			else
			{
				++m_numShadowcastLights;
				si.view[0] = e.view[++viewIter%6];		
				++gridIdx;
			}
		}

		si.info = e.info;
		si.position = e.position;
		si.color = e.color;
		si.radius = e.radius;
		si.projection = e.projection;
		
		spotLights.emplace_back(si);
	}

	globalLightBuffer.writeTo(spotLights.size(),spotLights.data());

}

void VulkanRenderer::UploadBones()
{

}

void VulkanRenderer::CreateSynchronisation()
{
	imageAvailable.resize(MAX_FRAME_DRAWS);
	renderFinished.resize(MAX_FRAME_DRAWS);

	drawFences.resize(MAX_FRAME_DRAWS);
	//Semaphore creation information
	VkSemaphoreCreateInfo semaphorecreateInfo = {};
	semaphorecreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	//fence creating information
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(m_device.logicalDevice, &semaphorecreateInfo, nullptr, &imageAvailable[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_device.logicalDevice, &semaphorecreateInfo, nullptr, &renderFinished[i]) != VK_SUCCESS ||
			vkCreateFence(m_device.logicalDevice, &fenceCreateInfo, nullptr,&drawFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a Semaphore and/or Fence!");
		}
		VK_NAME(m_device.logicalDevice, "imageAvailable", imageAvailable[i]);
		VK_NAME(m_device.logicalDevice, "renderFinished", renderFinished[i]);
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
	vpUniformBufferMemory.resize(m_swapchain.swapChainImages.size());
	//modelDUniformBuffer.resize(swapChainImages.size());
	//modelDUniformBufferMemory.resize(swapChainImages.size());

	//create uniform buffers
	for (size_t i = 0; i < m_swapchain.swapChainImages.size(); i++)
	{
		oGFX::CreateBuffer(m_device.physicalDevice, m_device.logicalDevice, vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
			//| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			, &vpUniformBuffer[i], &vpUniformBufferMemory[i]);
		/*createBuffer(mainDevice.physicalDevice, mainDevice.logicalDevice, modelBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &modelDUniformBuffer[i], &modelDUniformBufferMemory[i]);*/
	}
}

void VulkanRenderer::CreateDescriptorPool()
{
	// CREATE UNIFORM DESCRIPTOR POOL
	//descriptor is an individual piece of data // it is NOT a descriptor SET
	// Type of descriptors + how many DESCRIPTORS, not DESCRIPTOR_SETS (combined makes the pool size)

	// ViewProjection pool
	VkDescriptorPoolSize vpPoolsize = oGFX::vkutils::inits::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, static_cast<uint32_t>(vpUniformBuffer.size()));
	VkDescriptorPoolSize attachmentPool = oGFX::vkutils::inits::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);

	//// LightData pool (DYNAMIC)
	//VkDescriptorPoolSize modelPoolSize{};
	//modelPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	//modelPoolSize.descriptorCount = static_cast<uint32_t>(modelDUniformBuffer.size());

	//list of pool sizes
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes = { vpPoolsize,attachmentPool /*, modelPoolSize*/ };

	//data to create the descriptor pool
	VkDescriptorPoolCreateInfo poolCreateInfo = oGFX::vkutils::inits::descriptorPoolCreateInfo(descriptorPoolSizes,static_cast<uint32_t>(m_swapchain.swapChainImages.size()+1));
	//create descriptor pool
	VkResult result = vkCreateDescriptorPool(m_device.logicalDevice, &poolCreateInfo, nullptr, &descriptorPool);
	VK_NAME(m_device.logicalDevice, "descriptorPool", descriptorPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a descriptor pool!");
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
		throw std::runtime_error("Failed to create a descriptor pool!");
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
		throw std::runtime_error("Failed to allocate texture descriptor sets!");
	}
}

void VulkanRenderer::CreateDescriptorSets_GPUScene()
{
	VkDescriptorBufferInfo info{};
	info.buffer = gpuTransformBuffer.getBuffer();
	info.offset = 0;
	info.range = VK_WHOLE_SIZE;

	DescriptorBuilder::Begin(&DescLayoutCache, &descAllocs[swapchainIdx])
		.BindBuffer(3, gpuTransformBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.BindBuffer(4, gpuBoneMatrixBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.BindBuffer(5, objectInformationBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(descriptorSet_gpuscene,SetLayoutDB::gpuscene);
}

void VulkanRenderer::CreateDescriptorSets_Lights()
{
	VkDescriptorBufferInfo info{};
	info.buffer = globalLightBuffer.getBuffer();
	info.offset = 0;
	info.range = VK_WHOLE_SIZE;

	for (size_t i = 0; i < m_swapchain.swapChainImages.size(); i++)
	{
		DescriptorBuilder::Begin(&DescLayoutCache, &descAllocs[i])
			.BindBuffer(4, &info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
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
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// TODO: make sure we set the previous renderpass to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL 
	// since this will be the final pass (before presentation) instead
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


	if (vkCreateRenderPass(m_device.logicalDevice, &info, nullptr, &m_imguiConfig.renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Could not create Dear ImGui's render pass");
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

	m_imguiInitialized = true;
	RestartImgui();


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
			auto gbuff = RenderPassDatabase::GetRenderPass<GBufferRenderPass>();
	
			auto shadows = RenderPassDatabase::GetRenderPass<ShadowPass>();
	
			const float renderWidth = float(windowPtr->m_width);
			const float renderHeight = float(windowPtr->m_height);
			const float aspectRatio = renderHeight / renderWidth;
			const ImVec2 imageSize = { sz.x, sz.x * aspectRatio };
	
			//auto gbuff = GBufferRenderPass::Get();
			//ImGui::BulletText("World Position");
			//ImGui::Image(gbuff->deferredImg[POSITION], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("World Normal");
			ImGui::Image(gbuff->deferredImg[NORMAL], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("Albedo");
			ImGui::Image(gbuff->deferredImg[ALBEDO], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("Material");
			ImGui::Image(gbuff->deferredImg[MATERIAL], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
			ImGui::BulletText("Depth (TODO)");
			//ImGui::Image(gbuff->deferredImg[3], { sz.x,sz.y/4 });
			ImGui::Image(shadows->shadowImg ,imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
		}
	}
	ImGui::End();
}

void VulkanRenderer::DrawGUI()
{
	PROFILE_SCOPED();
	
	VkRenderPassBeginInfo GUIpassInfo = {};
	GUIpassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	GUIpassInfo.renderPass  = m_imguiConfig.renderPass;
	GUIpassInfo.framebuffer = m_imguiConfig.buffers[swapchainIdx];
	GUIpassInfo.renderArea = { {0, 0}, {m_swapchain.swapChainExtent}};

    const VkCommandBuffer cmdlist = commandBuffers[swapchainIdx];

	vkCmdBeginRenderPass(cmdlist, &GUIpassInfo, VK_SUBPASS_CONTENTS_INLINE);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdlist);
	vkCmdEndRenderPass(cmdlist);
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

void VulkanRenderer::DestroyImGUI()
{
	if (m_imguiInitialized == false) return;

	vkDeviceWaitIdle(m_device.logicalDevice);

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
	init_info.MinImageCount = m_swapchain.minImageCount + 1;
	init_info.ImageCount = static_cast<uint32_t>(m_swapchain.swapChainImages.size());
	init_info.CheckVkResultFn = VK_NULL_HANDLE; // can be used to handle the error checking
	init_info.CheckVkResultFn = checkresult; // can be used to handle the error checking

	ImGui_ImplVulkan_Init(&init_info, m_imguiConfig.renderPass);

	// This uploads the ImGUI font package to the GPU
	VkCommandBuffer command_buffer = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	endSingleTimeCommands(command_buffer); 

}


void VulkanRenderer::InitializeRenderBuffers()
{
	// In this function, all global rendering related buffers should be initialized, ONCE.

	// Note: Moved here from VulkanRenderer::UpdateIndirectCommands
	indirectCommandsBuffer.Init(&m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VK_NAME(m_device.logicalDevice, "Indirect Command Buffer", indirectCommandsBuffer.getBuffer()); 

	shadowCasterCommandsBuffer.Init(&m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	shadowCasterCommandsBuffer.reserve(MAX_OBJECTS);
	VK_NAME(m_device.logicalDevice, "Shadow Command Buffer", shadowCasterCommandsBuffer.m_buffer);

	// Note: Moved here from VulkanRenderer::UpdateInstanceData
	instanceBuffer.Init(&m_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    VK_NAME(m_device.logicalDevice, "Instance Buffer", instanceBuffer.getBuffer());

	objectInformationBuffer.Init(&m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	//objectInformationBuffer.reserve(MAX_OBJECTS);  
	VK_NAME(m_device.logicalDevice, "Object inforBuffer", objectInformationBuffer.getBuffer());

	constexpr uint32_t MAX_LIGHTS = 512;
	// TODO: Currently this is only for OmniLightInstance.
	// You should also support various light types such as spot lights, etc...

	globalLightBuffer.Init(&m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT |VK_BUFFER_USAGE_TRANSFER_SRC_BIT| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	//globalLightBuffer.reserve(MAX_LIGHTS);
    VK_NAME(m_device.logicalDevice, "Light Buffer", globalLightBuffer.getBuffer());

	constexpr uint32_t MAX_GLOBAL_BONES = 2048;
	constexpr uint32_t MAX_SKINNING_VERTEX_BUFFER_SIZE = 4 * 1024 * 1024; // 4MB

	gpuBoneMatrixBuffer.Init(&m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	//gpuBoneMatrixBuffer.reserve(MAX_GLOBAL_BONES * sizeof(glm::mat4x4));
    VK_NAME(m_device.logicalDevice, "Bone Matrix Buffer", gpuBoneMatrixBuffer.getBuffer());

	skinningVertexBuffer.Init(&m_device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	//skinningVertexBuffer.reserve(MAX_SKINNING_VERTEX_BUFFER_SIZE);  
    VK_NAME(m_device.logicalDevice, "Skinning Vertex Buffer", skinningVertexBuffer.getBuffer());

	g_GlobalMeshBuffers.IdxBuffer.Init(&m_device,VK_BUFFER_USAGE_TRANSFER_DST_BIT |VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	g_GlobalMeshBuffers.VtxBuffer.Init(&m_device,VK_BUFFER_USAGE_TRANSFER_DST_BIT |VK_BUFFER_USAGE_TRANSFER_SRC_BIT| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	//g_GlobalMeshBuffers.IdxBuffer.reserve(8 * 1000 * 1000);
	//g_GlobalMeshBuffers.VtxBuffer.reserve(1 * 1000 * 1000);
	
	g_particleCommandsBuffer.Init(&m_device, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
	//g_particleCommandsBuffer.reserve(1024); // commands are generally per emitter. shouldnt have so many..

	for (size_t i = 0; i < g_particleDatas.size(); i++)
	{
		g_particleDatas[i].Init(&m_device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		//g_particleDatas[i].reserve(100000*10); // 10 max particle systems
	}

	// TODO: Move other global GPU buffer initialization here...
}

void VulkanRenderer::DestroyRenderBuffers()
{
	indirectCommandsBuffer.destroy();
	shadowCasterCommandsBuffer.destroy();
	instanceBuffer.destroy();
	objectInformationBuffer.destroy();
	globalLightBuffer.destroy();
	gpuBoneMatrixBuffer.destroy();
	skinningVertexBuffer.destroy();

	g_particleCommandsBuffer.destroy();
	for (size_t i = 0; i < g_particleDatas.size(); i++)
	{
		g_particleDatas[i].destroy();
	}
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
			objectCount += indirectCmd.instanceCount;
		}


		if (objectCount == 0)
			return;


		// Better to catch this on the software side early than the Vulkan validation layer
		// TODO: Fix this gracefully
		if (allObjectsCommands.size() > MAX_OBJECTS)
		{
			MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of indirect command buffer.", L"BAD ERROR");
		}

		indirectCommandsBuffer.writeTo(allObjectsCommands.size(), allObjectsCommands.data());

	}

	// shadow commands
	{
		auto& shadowObjects = batches.GetBatch(GraphicsBatch::SHADOW_CAST);
		if (shadowObjects.size() > MAX_OBJECTS)
		{
			MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of indirect command buffer.", L"BAD ERROR");
		}
		shadowCasterCommandsBuffer.clear();
		shadowCasterCommandsBuffer.writeTo(shadowObjects.size(), (void*)shadowObjects.data(), 0);
	}

	{
		auto& particleCommands = batches.GetParticlesBatch();
		auto& particleData = batches.GetParticlesData();

		g_particleCommandsBuffer.clear();
		g_particleDatas[swapchainIdx].clear();

		g_particleCommandsBuffer.writeTo(particleCommands.size(), particleCommands.data());		
		g_particleDatas[swapchainIdx].writeTo(particleData.size(), particleData.data());
		
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
	std::vector<oGFX::InstanceData> instanceData;
	instanceData.reserve(objectCount);
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
					oGFX::InstanceData id;
					//size_t sz = instanceData.size();
					//for (size_t x = 0; x < g_globalModels[ent.modelID].meshCount; x++)
					{
						// This is per entity. Should be per material.
						uint32_t albedo = ent.bindlessGlobalTextureIndex_Albedo;
						uint32_t normal = ent.bindlessGlobalTextureIndex_Normal;
						uint32_t roughness = ent.bindlessGlobalTextureIndex_Roughness;
						uint32_t metallic = ent.bindlessGlobalTextureIndex_Metallic;
						const uint8_t perInstanceData = ent.instanceData;
						constexpr uint32_t invalidIndex = 0xFFFFFFFF;
						if (albedo == invalidIndex)
							albedo = whiteTextureID; // TODO: Dont hardcode this bindless texture index
						if (normal == invalidIndex)
							normal = blackTextureID; // TODO: Dont hardcode this bindless texture index
						if (roughness == invalidIndex)
							roughness = whiteTextureID; // TODO: Dont hardcode this bindless texture index
						if (metallic == invalidIndex)
							metallic = blackTextureID; // TODO: Dont hardcode this bindless texture index

						// Important: Make sure this index packing matches the unpacking in the shader
						const uint32_t albedo_normal = albedo << 16 | (normal & 0xFFFF);
						const uint32_t roughness_metallic = roughness << 16 | (metallic & 0xFFFF);
						const uint32_t instanceID = uint32_t(indexCounter); // the instance id should point to the entity
						auto res = ent.flags & ObjectInstanceFlags::SKINNED; 
						auto isSkin = (res== ObjectInstanceFlags::SKINNED);
						const uint32_t unused = (uint32_t)perInstanceData | isSkin << 8; //matCnt;

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
						id.instanceAttributes = uvec4(instanceID, unused, albedo_normal, roughness_metallic);
						instanceData.emplace_back(id);
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
				gpuTransform.emplace_back(gpt);
			}
			// skined mesh
			GPUObjectInformation oi;
			oi.entityID = ent.entityID;
			oi.materialIdx = 7; // tem,p
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
	

	if (instanceData.empty())
	{
		return;
	}

	gpuTransformBuffer.writeTo(gpuTransform.size(), gpuTransform.data());
	gpuBoneMatrixBuffer.writeTo(boneMatrices.size(), boneMatrices.data());

	objectInformationBuffer.writeTo(objectInformation.size(), objectInformation.data());

    // Better to catch this on the software side early than the Vulkan validation layer
	// TODO: Fix this gracefully
    if (instanceData.size() > MAX_OBJECTS)
    {
		MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of instance buffer.", L"BAD ERROR");
    }

	instanceBuffer.writeTo(instanceData.size(), instanceData.data());

}

bool VulkanRenderer::PrepareFrame()
{
	if (resizeSwapchain || windowPtr->m_width == 0 ||windowPtr->m_height == 0)
	{
		m_prepared = ResizeSwapchain();
		if (m_prepared == false)
			return false;
		resizeSwapchain = false;
	}

	
	return true;
}

void VulkanRenderer::BeginDraw()
{
	PROFILE_SCOPED();

	//wait for given fence to signal from last draw before continuing
	VK_CHK(vkWaitForFences(m_device.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max()));
	//mainually reset fences
	VK_CHK(vkResetFences(m_device.logicalDevice, 1, &drawFences[currentFrame]));

	{
		PROFILE_SCOPED("vkAcquireNextImageKHR");

        //1. get the next available image to draw to and set something to signal when we're finished with the image ( a semaphore )
		// -- GET NEXT IMAGE
		//get  index of next image to be drawn to , and signal semaphore when ready to be drawn to
        VkResult res = vkAcquireNextImageKHR(m_device.logicalDevice, m_swapchain.swapchain, std::numeric_limits<uint64_t>::max(),
            imageAvailable[currentFrame], VK_NULL_HANDLE, &swapchainIdx);


		DelayedDeleter::get()->Update();
		descAllocs[swapchainIdx].ResetPools();

		shadowsRendered = false;

		if (currWorld)
		{
			batches = GraphicsBatch::Init(currWorld, this, MAX_OBJECTS);
			batches.GenerateBatches();
		}

		UpdateUniformBuffers();
		UploadInstanceData();	
		UploadLights();
		GenerateCPUIndirectDrawCommands();

		DescriptorBuilder::Begin(&DescLayoutCache, &descAllocs[swapchainIdx])
			.BindBuffer(3, gpuTransformBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.BindBuffer(4, gpuBoneMatrixBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.BindBuffer(5, objectInformationBuffer.GetBufferInfoPtr(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.Build(descriptorSet_gpuscene,SetLayoutDB::gpuscene);

		auto uniformMinAlignment = m_device.properties.limits.minUniformBufferOffsetAlignment;
		auto paddedAlignment = oGFX::vkutils::tools::UniformBufferPaddedSize(2*sizeof(CB::FrameContextUBO), uniformMinAlignment);
		
		VkDescriptorBufferInfo vpBufferInfo{};
		vpBufferInfo.buffer = vpUniformBuffer[swapchainIdx];	// buffer to get data from
		vpBufferInfo.offset = 0;				// position of start of data
		vpBufferInfo.range = sizeof(CB::FrameContextUBO);		// size of data
		DescriptorBuilder::Begin(&DescLayoutCache, &descAllocs[swapchainIdx])
			.BindBuffer(0, &vpBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(descriptorSets_uniform[swapchainIdx], SetLayoutDB::FrameUniform);


		if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR /*|| WINDOW_RESIZED*/)
        {
            resizeSwapchain = true;
			m_prepared = false;
        }
	}

	{
		PROFILE_SCOPED("Begin Command Buffer");

        //Information about how to begin each command buffer
        VkCommandBufferBeginInfo bufferBeginInfo = oGFX::vkutils::inits::commandBufferBeginInfo();
        //start recording commanders to command buffer!
        VkResult result = vkBeginCommandBuffer(commandBuffers[swapchainIdx], &bufferBeginInfo);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to start recording a Command Buffer!");
        }
	}
}

void VulkanRenderer::RenderFrame()
{
	PROFILE_SCOPED();

	this->BeginDraw(); // TODO: Clean this up...

	bool shouldRunDebugDraw = UploadDebugDrawBuffers();
    {
		// Command list has already started inside VulkanRenderer::Draw
        PROFILE_GPU_CONTEXT(commandBuffers[swapchainIdx]);
        PROFILE_GPU_EVENT("CommandList");

        //this->SimplePass(); // Unsued
		// Manually schedule the order of the render pass execution. (single threaded)
		if(currWorld)
		{
			renderIteration = 0;
			for (size_t i = 0; i < currWorld->numCameras; i++)
			{		
				if (currWorld->shouldRenderCamera[i] == false)
				{
					continue;
				}

				renderTargetInUseID = currWorld->targetIDs[i];
				VkMemoryBarrier memoryBarrier{};
				memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;	

				vkCmdPipelineBarrier(commandBuffers[swapchainIdx],
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // srcStageMask
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // dstStageMask
					VK_DEPENDENCY_BY_REGION_BIT,		  // dependancy flag
					0,                                    // memoryBarrierCount
					nullptr,                       // pMemoryBarriers
					0, NULL, 0, NULL
				);
				if (shadowsRendered == false) // only render shadowpass once... 
				{
					//generally works until we need to perform better frustrum culling....
					RenderPassDatabase::GetRenderPass<ShadowPass>()->Draw();
					shadowsRendered = true;
				}
				//RenderPassDatabase::GetRenderPass<ZPrepassRenderpass>()->Draw();
				RenderPassDatabase::GetRenderPass<GBufferRenderPass>()->Draw();
				//RenderPassDatabase::GetRenderPass<DeferredDecalRenderpass>()->Draw();
				RenderPassDatabase::GetRenderPass<SSAORenderPass>()->Draw();

				RenderPassDatabase::GetRenderPass<DeferredCompositionRenderpass>()->Draw();
				RenderPassDatabase::GetRenderPass<ForwardParticlePass>()->Draw();
				RenderPassDatabase::GetRenderPass<BloomPass>()->Draw();
				//RenderPassDatabase::GetRenderPass<ForwardRenderpass>()->Draw();
#if defined		(ENABLE_DECAL_IMPLEMENTATION)
				RenderPassDatabase::GetRenderPass<ForwardDecalRenderpass>()->Draw();
#endif				
				//if (shouldRunDebugDraw) // for now need to run regardless because of transition.. TODO: FIX IT ONE DAY
				{
					RenderPassDatabase::GetRenderPass<DebugDrawRenderpass>()->dodebugRendering = shouldRunDebugDraw;
					RenderPassDatabase::GetRenderPass<DebugDrawRenderpass>()->Draw();
				}

				++renderIteration;
			}
			auto& dst = m_swapchain.swapChainImages[swapchainIdx];
			if (currWorld->numCameras > 1)
			{
				// TODO: Very bad pls fix
				auto thisID = currWorld->targetIDs[1];
				auto& texture = renderTargets[thisID].texture;		

				vkutils::TransitionImage(commandBuffers[swapchainIdx], texture, VK_IMAGE_LAYOUT_GENERAL);

				auto nextID = currWorld->targetIDs[0];
				auto& nextTexture = renderTargets[nextID].texture;
				FullscreenBlit(commandBuffers[swapchainIdx], nextTexture, VK_IMAGE_LAYOUT_GENERAL, dst, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
				
			}
			else
			{
				auto thisID = currWorld->targetIDs[0];
				auto& texture = renderTargets[thisID].texture;
				FullscreenBlit(commandBuffers[swapchainIdx], texture, VK_IMAGE_LAYOUT_GENERAL, dst, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL);
			}
			// only blit main framebuffer
			
		}
    }
}

void VulkanRenderer::Present()
{

	PROFILE_SCOPED();

	//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[swapchainImageIndex]);
	//stop recording to command buffer
	VkResult result = vkEndCommandBuffer(commandBuffers[swapchainIdx]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to stop recording a Command Buffer!");
	}

	//2. Submit command buffer to queue for execution, make sure it waits for image to be signalled as available before drawing
	//		and signals when it has finished rendering
	// --SUBMIT COMMAND BUFFER TO RENDER
	// Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1; //number of semaphores to wait on
	submitInfo.pWaitSemaphores = &imageAvailable[currentFrame]; //list of semaphores to wait on
	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	std::vector <VkSemaphore> frameSemaphores = { renderFinished[currentFrame],
	};

	submitInfo.pWaitDstStageMask = waitStages; //stages to check semapheres at
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[swapchainIdx];	// command buffer to submit
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(frameSemaphores.size());						// number of semaphores to signal
	submitInfo.pSignalSemaphores = frameSemaphores.data();				// semphores to signal when command buffer finished

																				//submit command buffer to queue
	result = vkQueueSubmit(m_device.graphicsQueue, 1, &submitInfo, drawFences[currentFrame]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit command buffer to queue!");
	}

	//3. present image t oscreen when it has signalled finished rendering
	// -- PRESENT RENDERED IMAGE TO SCREEN --
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinished[currentFrame];	//semaphores to wait on
	presentInfo.swapchainCount = 1;					//number of swapchains to present to
	presentInfo.pSwapchains = &m_swapchain.swapchain;			//swapchains to present images to
	presentInfo.pImageIndices = &swapchainIdx;		//index of images in swapchains to present

															//present image
	PROFILE_GPU_PRESENT(m_swapchain.swapchain);
	try
	{
		result = vkQueuePresentKHR(m_device.presentationQueue, &presentInfo);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR /*|| WINDOW_RESIZED*/)
		{
			resizeSwapchain = true;
			m_prepared = false;
			return;
		}
		else if(result != VK_SUCCESS && result!= VK_SUBOPTIMAL_KHR)
		{
			std::cout << (int)result;
			throw std::runtime_error("Failed to present image!");
		}
	}
	catch(std::runtime_error e){
		std::cout << e.what();
	}
	//get next frame (use % MAX_FRAME_DRAWS to keep value below max frames)
	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

bool VulkanRenderer::ResizeSwapchain()
{
	while (windowPtr->m_height == 0 || windowPtr->m_width == 0)
	{
		if (windowPtr->m_type == Window::WINDOWS32)
		{
			Window::PollEvents();
		}
		else
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
			desc_image[0].sampler = image.sampler;
			desc_image[0].imageView = image.view;
			desc_image[0].imageLayout = image.imageLayout;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = (VkDescriptorSet)currWorld->imguiID[x];
			write_desc[0].descriptorCount = 1;
			if (image.imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
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

ModelFileResource* VulkanRenderer::GetDefaultCube()
{
	return def_cube.get();
}

ModelFileResource* VulkanRenderer::LoadModelFromFile(const std::string& file)
{
	std::stringstream os;
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

	

	os <<"[Loading] " << file << std::endl;
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
	os << "Meshes" << scene->mNumMeshes << std::endl;
	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		auto& mesh = scene->mMeshes[i];
		os << "\tMesh" << i << " " << mesh->mName.C_Str() << std::endl;
		os << "\t\tverts:"  << mesh->mNumVertices << std::endl;
		os << "\t\tbones:"  << mesh->mNumBones << std::endl;
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
	std::cout << os.str();
	os.clear();

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


	ModelFileResource* modelFile = new ModelFileResource;
	modelFile->fileName = file;

	auto mdlResourceIdx = g_globalModels.size();
	modelFile->meshResource = static_cast<uint32_t>(mdlResourceIdx);
	auto& mdl{ g_globalModels.emplace_back(gfxModel{}) };
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
		
		for (size_t i = 0; i < mdl.skeleton->boneWeights.size(); i++)
		{
			//auto& ref = mdl.skeleton->boneWeights[i];
			//os << i;
			//for (size_t x = 0; x < 4; x++)
			//{
			//	os << " [" << ref.boneIdx[x] << "," << ref.boneWeights[x] <<"]";
			//}
			//os << std::endl;
		}

	}
	
	for (auto& sm : mdl.m_subMeshes)
	{
		mdl.vertexCount += sm.vertexCount;
		mdl.indicesCount += sm.indicesCount;
	}

	//mData->sceneInfo = new Node();
	//always has one transform, root
	modelFile->ModelSceneLoad(scene, *scene->mRootNode, nullptr, glm::mat4{ 1.0f });
		
	//model.loadNode(nullptr, scene, *scene->mRootNode, 0, *mData);
	auto cI_offset = g_GlobalMeshBuffers.IdxOffset;
	auto cV_offset = g_GlobalMeshBuffers.VtxOffset;
	
	{
		LoadMeshFromBuffers(modelFile->vertices, modelFile->indices, &mdl);
	}

	os << "\t [Meshes loaded] " << modelFile->sceneMeshCount << std::endl;

	std::cout << os.str();
	return modelFile;
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
	}

	submesh.vertexCount = aimesh->mNumVertices;
	submesh.baseVertex = static_cast<uint32_t>(cacheVoffset);
	submesh.indicesCount = indicesCnt;
	submesh.baseIndices = static_cast<uint32_t>(cacheIoffset);
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
	std::cout << "Writing to vtx from data " << model->baseVertex
		<< " for " << model->vertexCount
		<<" total " << model->baseVertex+model->vertexCount
		<< " at GPU buffer " << g_GlobalMeshBuffers.VtxOffset
		<< std::endl;
	g_GlobalMeshBuffers.IdxBuffer.writeTo(model->indicesCount, indices.data() + model->baseIndices,
		g_GlobalMeshBuffers.IdxOffset);
	g_GlobalMeshBuffers.VtxBuffer.writeTo(model->vertexCount, vertex.data() + model->baseVertex,
		g_GlobalMeshBuffers.VtxOffset);

	// now we update them to the global offset
	model->baseIndices= g_GlobalMeshBuffers.IdxOffset;
	model->baseVertex= g_GlobalMeshBuffers.VtxOffset;

	g_GlobalMeshBuffers.IdxOffset += model->indicesCount;
	g_GlobalMeshBuffers.VtxOffset += model->vertexCount;

	if (model->skeleton)
	{
		auto& sk = model->skeleton;
		skinningVertexBuffer.writeTo(sk->boneWeights.size(), sk->boneWeights.data(), model->baseVertex);
	}

	return m;
}

void VulkanRenderer::LoadBoneInformation(ModelFileResource& fileData,
	oGFX::Skeleton& skeleton,
	aiMesh& aimesh,
	std::vector<oGFX::BoneWeight>& boneWeights,
	uint32_t& vCnt
)
{
	uint32_t numBones = 0;

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
						std::cout << "Discarded weight: [" << key<<",\t"<< minW << "]" << std::endl;
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
		std::cout <<prefix<< "Creating bone " << node_name << std::endl;
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
	VkCommandBufferAllocateInfo allocInfo= oGFX::vkutils::inits::commandBufferAllocateInfo(m_device.commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY,1);

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device.logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

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

	vkQueueSubmit(m_device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_device.graphicsQueue);

	vkFreeCommandBuffers(m_device.logicalDevice, m_device.commandPool, 1, &commandBuffer);
}

uint32_t VulkanRenderer::CreateTexture(uint32_t width, uint32_t height, unsigned char* imgData)
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

	//create texture descriptor
	int descriptorLoc = AddBindlessGlobalTexture(g_Textures[ind]);

	//return location of set with texture
	return descriptorLoc;

}

uint32_t VulkanRenderer::CreateTexture(const std::string& file)
{
	// Create texture image and get its location in array
	uint32_t textureImageLoc = CreateTextureImage(file);

	//create texture descriptor
	int descriptorLoc = AddBindlessGlobalTexture(g_Textures[textureImageLoc]);

	//return location of set with texture
	return descriptorLoc;
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
	texture.fromBuffer((void*)imageData.imgData.data(), imageSize, imageData.format, imageData.w, imageData.h, imageData.mipInformation, &m_device, m_device.transferQueue);
	texture.updateDescriptor();
	std::vector<VkWriteDescriptorSet> writeSets
	{
		oGFX::vkutils::inits::writeDescriptorSet(descriptorSet_bindless, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texture.descriptor),
	};
	writeSets[0].dstArrayElement = textureID;
	vkUpdateDescriptorSets(m_device.logicalDevice, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);

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
	g_DebugDrawVertexBufferGPU.Init(&m_device,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	g_DebugDrawIndexBufferGPU.Init(&m_device,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
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

	g_DebugDrawVertexBufferGPU.reserve(g_DebugDrawVertexBufferCPU.size() );
	g_DebugDrawIndexBufferGPU.reserve(g_DebugDrawIndexBufferCPU.size());

	// Copy CPU debug draw buffers to the GPU
	g_DebugDrawVertexBufferGPU.writeTo(g_DebugDrawVertexBufferCPU.size() , g_DebugDrawVertexBufferCPU.data());
	g_DebugDrawIndexBufferGPU.writeTo(g_DebugDrawIndexBufferCPU.size() , g_DebugDrawIndexBufferCPU.data());

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
	vkMapMemory(m_device.logicalDevice, vpUniformBufferMemory[swapchainIdx], 0, numCameras*alignedRange, 0, &data);

	memcpy(data, &frameContextUBO[0], sizeof(CB::FrameContextUBO));
	memcpy((char*)data+alignedRange, &frameContextUBO[1], sizeof(CB::FrameContextUBO));

	VkMappedMemoryRange memRng{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
	memRng.memory = vpUniformBufferMemory[swapchainIdx];
	memRng.offset =  0;
	memRng.size =  numCameras*alignedRange;
	VK_CHK(vkFlushMappedMemoryRanges(m_device.logicalDevice, 1, &memRng));

	vkUnmapMemory(m_device.logicalDevice, vpUniformBufferMemory[swapchainIdx]);
}

uint32_t VulkanRenderer::CreateTextureImage(const std::string& fileName)
{
	//Load image file
	oGFX::FileImageData imageData;
	imageData.Create(fileName);
	
	//int width{}, height{};
	//VkDeviceSize imageSize;
	//unsigned char *imageData = oGFX::LoadTextureFromFile(fileName, width, height, imageSize);

	auto value = CreateTextureImage(imageData);

	imageData.Free();
	return value;
}

uint32_t VulkanRenderer::CreateTextureImage(const oGFX::FileImageData& imageInfo)
{
	VkDeviceSize imageSize = imageInfo.dataSize;
	totalTextureSizeLoaded += imageSize;

	auto indx = g_Textures.size();
	g_Textures.push_back(vkutils::Texture2D());

	auto& texture = g_Textures[indx];
	
	texture.fromBuffer((void*)imageInfo.imgData.data(), imageSize, imageInfo.format, imageInfo.w, imageInfo.h,imageInfo.mipInformation, &m_device, m_device.transferQueue);
	texture.name = imageInfo.name;

	//setup imgui binding
	g_imguiIDs.push_back(CreateImguiBinding(texture.sampler, texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

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

uint32_t VulkanRenderer::AddBindlessGlobalTexture(vkutils::Texture2D texture)
{
	std::vector<VkWriteDescriptorSet> writeSets
	{
		oGFX::vkutils::inits::writeDescriptorSet(descriptorSet_bindless, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texture.descriptor),
	};

	//auto index = static_cast<uint32_t>(samplerDescriptorSets.size());
	//samplerDescriptorSets.push_back(globalSamplers); // Wtf???
	uint32_t index = bindlessGlobalTexturesNextIndex++;
	writeSets[0].dstArrayElement = index;

	vkUpdateDescriptorSets(m_device.logicalDevice, static_cast<uint32_t>(writeSets.size()), writeSets.data(), 0, nullptr);

	return index;
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
	
}

ImTextureID VulkanRenderer::GetImguiID(uint32_t textureID)
{
	return g_imguiIDs[textureID];
}

ImTextureID VulkanRenderer::CreateImguiBinding(VkSampler s, VkImageView v, VkImageLayout l)
{
	
	if (VulkanRenderer::get()->m_imguiInitialized == false)
	{
		return 0;
	}	
	
	return ImGui_ImplVulkan_AddTexture(s,v,l);
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
