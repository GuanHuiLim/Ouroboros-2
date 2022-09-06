#define NOMINMAX
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

#include "GraphicsBatch.h"

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

VulkanRenderer* VulkanRenderer::s_vulkanRenderer{ nullptr };

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

	RenderPassDatabase::Shutdown();

	DestroyRenderBuffers();

	ShutdownTreeDebug();

	samplerManager.Shutdown();

	gpuTransformBuffer.destroy();
	debugTransformBuffer.destroy();

	g_GlobalMeshBuffers.IdxBuffer.destroy();
	g_GlobalMeshBuffers.VtxBuffer.destroy();

	if (m_imguiInitialized)
	{
		DestroyImGUI();
	}

	DescLayoutCache.Cleanup();
	DescAlloc.Cleanup();

	lightsBuffer.destroy();

	vkDestroyFramebuffer(m_device.logicalDevice, offscreenFramebuffer, nullptr);
	offscreenFB.destroy(m_device.logicalDevice);
	offscreenDepth.destroy(m_device.logicalDevice);

	if (offscreenPass)
	{
		vkDestroyRenderPass(m_device.logicalDevice, offscreenPass, nullptr);
		offscreenPass = VK_NULL_HANDLE;
	}

	for (size_t i = 0; i < models.size(); i++)
	{
		models[i].destroy(m_device.logicalDevice);
	}	
	for (size_t i = 0; i < g_Textures.size(); i++)
	{
		g_Textures[i].destroy();
	}

	// global sampler pool
	vkDestroyDescriptorPool(m_device.logicalDevice, samplerDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_device.logicalDevice, LayoutDB::bindless, nullptr);

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

	vkDestroyPipeline(m_device.logicalDevice, graphicsPSO, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, wireframePSO, nullptr);

	vkDestroyPipeline(m_device.logicalDevice, indirectPSO, nullptr);
	vkDestroyPipelineLayout(m_device.logicalDevice, indirectPSOLayout, nullptr);

	
	if (renderPass_default)
	{
		vkDestroyRenderPass(m_device.logicalDevice, renderPass_default, nullptr);
		renderPass_default = VK_NULL_HANDLE;
	}
	if (renderPass_default2)
	{
		vkDestroyRenderPass(m_device.logicalDevice, renderPass_default2, nullptr);
		renderPass_default2 = VK_NULL_HANDLE;
	}
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

		CreateSurface(setupSpecs,window);
		// set surface for imgui
		Window::SurfaceFormat = (uint64_t)window.SurfaceFormat;

		AcquirePhysicalDevice(setupSpecs);
		CreateLogicalDevice(setupSpecs);

		//if (m_device.debugMarker)
		//{
		// TODO MAKE SURE THIS IS SUPPORTED
		pfnDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(m_device.logicalDevice, "vkDebugMarkerSetObjectNameEXT");
		//}
		//
		SetupSwapchain();

		InitializeRenderBuffers();

		CreateRenderpass();
		CreateUniformBuffers();
		CreateDescriptorSetLayout();

		fbCache.Init(m_device.logicalDevice);

		*const_cast<VkBuffer*>(gpuTransformBuffer.getBufferPtr()) = VK_NULL_HANDLE;
		std::cout << "gpu xform :" << gpuTransformBuffer.m_size << " " << gpuTransformBuffer.m_capacity << std::endl;
		gpuTransformBuffer.Init(&m_device,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		gpuTransformBuffer.reserve(MAX_OBJECTS);

		// TEMP debug drawing code
		std::cout << "debug xform :" << debugTransformBuffer.m_size << " " << debugTransformBuffer.m_capacity << std::endl;
		*const_cast<VkBuffer*>(debugTransformBuffer.getBufferPtr()) = VK_NULL_HANDLE;
		debugTransformBuffer.Init(&m_device,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		debugTransformBuffer.reserve(MAX_OBJECTS);

		CreateDescriptorSets_GPUScene();

		CreateGraphicsPipeline();

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
		 ptr = new DebugRenderpass;
		rpd->RegisterRenderPass(ptr);
		 ptr = new GBufferRenderPass;
		rpd->RegisterRenderPass(ptr);
		 ptr = new DeferredCompositionRenderpass;
		rpd->RegisterRenderPass(ptr);

		RenderPassDatabase::InitAllRegisteredPasses();

		CreateFramebuffers();

		CreateOffscreenPass();
		CreateOffscreenFB();

		CreateCommandBuffers();
		CreateDescriptorPool();
		CreateSynchronisation();


		InitTreeDebugDraws();
		InitDebugBuffers();
		g_GlobalMeshBuffers.IdxBuffer.Init(&m_device,VK_BUFFER_USAGE_TRANSFER_DST_BIT |VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		g_GlobalMeshBuffers.VtxBuffer.Init(&m_device,VK_BUFFER_USAGE_TRANSFER_DST_BIT |VK_BUFFER_USAGE_TRANSFER_SRC_BIT| VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		
		PROFILE_INIT_VULKAN(&m_device.logicalDevice, &m_device.physicalDevice, &m_device.graphicsQueue, (uint32_t*)&m_device.queueIndices.graphicsFamily, 1, nullptr);
	}
	catch (std::runtime_error e)
	{
		throw e;
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
	catch(...)
	{
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

void VulkanRenderer::CreateRenderpass()
{
	if (renderPass_default)
	{
		vkDestroyRenderPass(m_device.logicalDevice, renderPass_default, nullptr);
		renderPass_default = VK_NULL_HANDLE;
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
	depthAttachment.format = oGFX::ChooseSupportedFormat(m_device,
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
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

	VkResult result = vkCreateRenderPass(m_device.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass_default);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Render Pass");
	}
	VK_NAME(m_device.logicalDevice, "defaultRenderPass",renderPass_default);

	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	VK_CHK(vkCreateRenderPass(m_device.logicalDevice, &renderPassCreateInfo, nullptr, &renderPass_default2));
	VK_NAME(m_device.logicalDevice, "defaultRenderPass_2",renderPass_default2);
	//depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//result = vkCreateRenderPass(m_device.logicalDevice, &renderPassCreateInfo, nullptr, &compositionPass);
	//if (result != VK_SUCCESS)
	//{
	//	throw std::runtime_error("Failed to create Render Pass");
	//}
}

void VulkanRenderer::CreateDescriptorSetLayout()
{
	DescAlloc.Init(m_device.logicalDevice);
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

	numCameras = 2;
	VkDeviceSize vpBufferSize = uboDynamicAlignment * numCameras;

	descriptorSets_uniform.resize(m_swapchain.swapChainImages.size());
	// UBO for each swapchain images
	for (size_t i = 0; i < m_swapchain.swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo vpBufferInfo{};
		vpBufferInfo.buffer = vpUniformBuffer[i];	// buffer to get data from
		vpBufferInfo.offset = 0;					// position of start of data
		vpBufferInfo.range = sizeof(CB::FrameContextUBO);			// size of data

		DescriptorBuilder::Begin(&DescLayoutCache, &DescAlloc)
			.BindBuffer(0, &vpBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.Build(descriptorSets_uniform[i], LayoutDB::uniform);
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
	
	// create a descriptor set layout with given bindings for texture
	VkDescriptorSetLayoutCreateInfo textureLayoutCreateInfo = 
		oGFX::vkutils::inits::descriptorSetLayoutCreateInfo(&samplerLayoutBinding,1);


	VkDescriptorBindingFlags flags[3];
	//flags[0] = 0;
	//flags[1] = 0;
	flags[0] = 	VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo flaginfo{};
	flaginfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	flaginfo.pBindingFlags = flags;
	flaginfo.bindingCount = 1;

	textureLayoutCreateInfo.pNext = &flaginfo;


	VkResult result = vkCreateDescriptorSetLayout(m_device.logicalDevice, &textureLayoutCreateInfo, nullptr, &LayoutDB::bindless);
	VK_NAME(m_device.logicalDevice, "samplerSetLayout", LayoutDB::bindless);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a descriptor set layout!");
	}

}

void VulkanRenderer::CreateGraphicsPipeline()
{
	using namespace oGFX;
	//create pipeline

	//how the data for a single vertex (including infos such as pos, colour, texture, coords, normals etc) is as a whole
	std::vector<VkVertexInputBindingDescription> bindingDescription = oGFX::GetGFXVertexInputBindings();

	//how the data for an attirbute is define in the vertex
	std::vector<VkVertexInputAttributeDescription>attributeDescriptions = oGFX::GetGFXVertexInputAttributes();
	// -- VERTEX INPUT -- 
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo(bindingDescription,attributeDescriptions);
	// __ INPUT ASSEMBLY __
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0 ,VK_FALSE);
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1,1,0);
	//Dynami states to enable	
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	// -- RASTERIZER --
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE,0);
	// -- MULTI SAMPLING --
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT,0);

	//-- BLENDING --
	//Blending decies how to blend a new color being written to a a fragment with the old value
	//blend attachment state (how blending is handled
	
	VkPipelineColorBlendAttachmentState colourState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0x0000000F,VK_TRUE);

	//blending uses equation : (srcColourBlendFactor * new colour ) colourBlendOp ( dstColourBlendFActor*old colour)
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	//summarsed : (VK_BLEND_FACTOR_SRC_ALPHA * new colour) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old colour)
	//			  (new colour alpha * new colour) + ((1 - new colour alpha) * old colour)

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	//summarised : (1 * new alpha) + (0 * old alpha) = new alpha


	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1,&colourState);

	// -- PIPELINE LAYOUT 
	std::array<VkDescriptorSetLayout, 3> descriptorSetLayouts = 
	{
		LayoutDB::gpuscene, // (set = 0)
		LayoutDB::uniform,  // (set = 1)
		LayoutDB::bindless // (set = 2)
	};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
		oGFX::vkutils::inits::pipelineLayoutCreateInfo(descriptorSetLayouts.data(),static_cast<uint32_t>(descriptorSetLayouts.size()));
	VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_ALL, 0, 128 };
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	// indirect pipeline
	VkResult result = vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &indirectPSOLayout);
	VK_NAME(m_device.logicalDevice, "indirectPipeLayout", indirectPSOLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Pipeline Layout!");
	}
	// go back to normal pipelines

	// Create Pipeline Layout
	//result = vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	//if (result != VK_SUCCESS)
	//{
	//	throw std::runtime_error("Failed to create Pipeline Layout!");
	//}
	std::array<VkPipelineShaderStageCreateInfo,2>shaderStages = {};
	
	// -- DEPTH STENCIL TESTING --	
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE,VK_TRUE, VK_COMPARE_OP_LESS);

																	// -- GRAPHICS PIPELINE CREATION --
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = oGFX::vkutils::inits::pipelineCreateInfo(indirectPSOLayout,renderPass_default);
	pipelineCreateInfo.stageCount = 2;								//number of shader stages
	pipelineCreateInfo.pStages = shaderStages.data();				//list of sader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;	//all the fixed funciton pipeline states
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;


	//graphics pipeline creation requires array of shader stages create

	//create graphics pipeline
	shaderStages[0] = LoadShader(m_device,"Shaders/bin/indirect.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(m_device,"Shaders/bin/indirect.frag.spv",VK_SHADER_STAGE_FRAGMENT_BIT);

	pipelineCreateInfo.layout = indirectPSOLayout;
	// Indirect pipeline
	result = vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &indirectPSO);
	VK_NAME(m_device.logicalDevice, "indirectPSO", indirectPSO);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}
	//destroy indirect shader modules 
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);


	pipelineCreateInfo.layout = indirectPSOLayout;
	// we use less for normal pipeline
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 5;

	shaderStages[0] = LoadShader(m_device,"Shaders/bin/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(m_device,"Shaders/bin/shader.frag.spv",VK_SHADER_STAGE_FRAGMENT_BIT);

	result = vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPSO);
	VK_NAME(m_device.logicalDevice, "graphicsPSO", graphicsPSO);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_LINE;
	pipelineCreateInfo.renderPass = renderPass_default;
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &wireframePSO));
	VK_NAME(m_device.logicalDevice, "wireframePSO", wireframePSO);

	//destroy shader modules after pipeline is created
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);

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
			m_swapchain.swapChainImages[i].imageView,
			m_swapchain.depthAttachment.view
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass_default;										//render pass layout the frame buffer will be used with
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();							//list of attachments (1:1 with render pass)
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

void VulkanRenderer::CreateOffscreenPass()
{
	if (offscreenPass)
	{
		vkDestroyRenderPass(m_device.logicalDevice, offscreenPass, nullptr);
		offscreenPass = VK_NULL_HANDLE;
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
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //image data layout aftet render pass ( to change to)

																	   // Depth attachment of render pass
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = oGFX::ChooseSupportedFormat(m_device,
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

	VkResult result = vkCreateRenderPass(m_device.logicalDevice, &renderPassCreateInfo, nullptr, &offscreenPass);
	VK_NAME(m_device.logicalDevice, "offscreenPass", offscreenPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Render Pass");
	}
}

void VulkanRenderer::CreateOffscreenFB()
{
	
	offscreenFB.createAttachment(m_device, m_swapchain.swapChainExtent.width, m_swapchain.swapChainExtent.height,
		m_swapchain.swapChainImageFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);	
	
	
	VkFormat attDepthFormat = oGFX::ChooseSupportedFormat(m_device,
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	offscreenDepth.createAttachment(m_device, m_swapchain.swapChainExtent.width, m_swapchain.swapChainExtent.height,
		attDepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	std::vector<VkImageView> attachments{offscreenFB.view, offscreenDepth.view}; // color attachment

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = renderPass_default;										//render pass layout the frame buffer will be used with
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferCreateInfo.pAttachments = attachments.data();							//list of attachments (1:1 with render pass)
	framebufferCreateInfo.width = m_swapchain.swapChainExtent.width;
	framebufferCreateInfo.height = m_swapchain.swapChainExtent.height;
	framebufferCreateInfo.layers = 1;


	VkResult result = vkCreateFramebuffer(m_device.logicalDevice, &framebufferCreateInfo, nullptr, &offscreenFramebuffer);
	VK_NAME(m_device.logicalDevice, "offscreenFramebuffer", offscreenFramebuffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Framebuffer!");
	}
	myImg = CreateImguiBinding(samplerManager.GetDefaultSampler(), offscreenFB.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	
}

void VulkanRenderer::ResizeOffscreenFB()
{
	vkDestroyFramebuffer(m_device.logicalDevice, offscreenFramebuffer,nullptr);
	offscreenFB.destroy(m_device.logicalDevice);
	offscreenDepth.destroy(m_device.logicalDevice);

	CreateOffscreenFB();
}

void VulkanRenderer::ResizeDeferredFB()
{


}

void VulkanRenderer::SetWorld(GraphicsWorld* world)
{
	// force a sync here
	vkDeviceWaitIdle(m_device.logicalDevice);
	currWorld = world;
}

void VulkanRenderer::CreateLightingBuffers()
{
	oGFX::CreateBuffer(m_device.physicalDevice, m_device.logicalDevice, sizeof(CB::LightUBO), 
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&lightsBuffer.buffer, &lightsBuffer.memory);
	lightsBuffer.size = sizeof(CB::LightUBO);
	lightsBuffer.device = m_device.logicalDevice;
	lightsBuffer.descriptor.buffer = lightsBuffer.buffer;
	lightsBuffer.descriptor.offset = 0;
	lightsBuffer.descriptor.range = sizeof(CB::LightUBO);

	VK_CHK(lightsBuffer.map());
}

void VulkanRenderer::UploadLights()
{
	if (currWorld == nullptr)
		return;

	CB::LightUBO lightUBO{};

	// Current view position
	lightUBO.viewPos = glm::vec4(camera.position, 0.0f);

	// Temporary reroute
	auto& allLights = currWorld->m_HardcodedOmniLights;

	// Gather lights to be uploaded.
	// TODO: Frustum culling for light bounding volume...
	int numLights = glm::clamp((int)allLights.size(), 0, 6);
	for (int i = 0; i < numLights; ++i)
	{
		lightUBO.lights[i] = allLights[i];
	}

	// Only lights that are inside/intersecting the camera frustum should be uploaded.
	memcpy(lightsBuffer.mapped, &lightUBO, sizeof(CB::LightUBO));
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

	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(m_device.physicalDevice,&props);
	size_t minUboAlignment = props.limits.minUniformBufferOffsetAlignment;
	//auto dynamicAlignment = sizeof(glm::mat4);
	uboDynamicAlignment = sizeof(CB::FrameContextUBO);
	if (minUboAlignment > 0) {
		uboDynamicAlignment = (uboDynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}

	numCameras = 2;
	VkDeviceSize vpBufferSize = uboDynamicAlignment * numCameras;

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
		// TODO: Disasble host coherent bit and manuall flush buffers for application
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
	VkDescriptorPoolSize samplerPoolSize = oGFX::vkutils::inits::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);// or MAX_OBJECTS?
	std::vector<VkDescriptorPoolSize> samplerpoolSizes = { samplerPoolSize };

	VkDescriptorPoolCreateInfo samplerPoolCreateInfo = oGFX::vkutils::inits::descriptorPoolCreateInfo(samplerpoolSizes,1); // or MAX_OBJECTS?
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
	variableDescriptorCountAllocInfo.pDescriptorCounts  = variableDescCounts;

	//Descriptor set allocation info
	VkDescriptorSetAllocateInfo setAllocInfo = oGFX::vkutils::inits::descriptorSetAllocateInfo(samplerDescriptorPool,&LayoutDB::bindless,1);
	setAllocInfo.pNext = &variableDescriptorCountAllocInfo;

	//Allocate our descriptor sets
	result = vkAllocateDescriptorSets(m_device.logicalDevice, &setAllocInfo, &descriptorSet_bindless);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate texture descriptor sets!");
	}
}

void VulkanRenderer::CreateDescriptorSets_GPUScene()
{
	VkDescriptorBufferInfo info{};
	info.buffer = gpuTransformBuffer.getBuffer();
	info.offset = 0;
	info.range = VK_WHOLE_SIZE;

	DescriptorBuilder::Begin(&DescLayoutCache, &DescAlloc)
		.BindBuffer(3, &info, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.Build(descriptorSet_gpuscene,LayoutDB::gpuscene);
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
	vkCreateDescriptorPool(m_device.logicalDevice, &dpci, nullptr, &m_imguiConfig.descriptorPools);
	VK_NAME(m_device.logicalDevice, "imguiConfig_descriptorPools", m_imguiConfig.descriptorPools);


	
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
	init_info.MinImageCount = m_swapchain.minImageCount;
	init_info.ImageCount = static_cast<uint32_t>(m_swapchain.swapChainImages.size());
	init_info.CheckVkResultFn = VK_NULL_HANDLE; // can be used to handle the error checking

	
	ImGui_ImplVulkan_Init(&init_info, m_imguiConfig.renderPass);

	// This uploads the ImGUI font package to the GPU
	VkCommandBuffer command_buffer = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
	endSingleTimeCommands(command_buffer); 

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
		fbattachments[0] = m_swapchain.swapChainImages[i].imageView;         // A color attachment from the swap chain
													//fbattachments[1] = m_depthImage.imageView;  // A depth attachment
		VK_CHK(vkCreateFramebuffer(m_device.logicalDevice, &_ci, nullptr, &m_imguiConfig.buffers[i])); 
		VK_NAME(m_device.logicalDevice, "imguiconfig_Framebuffer", m_imguiConfig.buffers[i]);
	}

	m_imguiInitialized = true;

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
		fbattachments[0] = m_swapchain.swapChainImages[i].imageView;         // A color attachment from the swap chain
																			 //fbattachments[1] = m_depthImage.imageView;  // A depth attachment
		VK_CHK(vkCreateFramebuffer(m_device.logicalDevice, &_ci, nullptr, &m_imguiConfig.buffers[i]));
		VK_NAME(m_device.logicalDevice, "imguiconfig_buffers", m_imguiConfig.buffers[i]);
	}
}

void VulkanRenderer::DebugGUIcalls()
{
	if(ImGui::Begin("img"))
	{
		const char* views[]  = { "Lookat", "FirstPerson" };
		ImGui::ListBox("Camera View", reinterpret_cast<int*>(&camera.m_CameraMovementType), views, 2);
		auto sz = ImGui::GetContentRegionAvail();
		ImGui::Image(myImg, { sz.x,sz.y });
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
			ImGui::BulletText("World Position");
			ImGui::Image(gbuff->deferredImg[POSITION], imageSize, ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 1));
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
	ImGui_ImplVulkan_Shutdown();
	if (windowPtr->m_type == Window::WindowType::WINDOWS32)
	{
		ImGui_ImplWin32_Shutdown();
	}
	m_imguiInitialized = false;
}

void VulkanRenderer::AddDebugBox(const AABB& aabb, const oGFX::Color& col, size_t loc)
{
	static std::vector<uint32_t> boxindices{
		0,1,
		0,2,
		0,3,
		1,4,
		1,5,
		3,5,
		3,6,
		2,6,
		2,4,
		6,7,
		5,7,
		4,7
	};
		
	if (loc == size_t(-1))
	{
		auto sz = g_debugDrawVerts.size();
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0], -aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); //0
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0],  aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); // 1
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0], -aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 2
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0], -aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); // 3
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0],  aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 4
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0],  aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); // 5
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0], -aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 6
		g_debugDrawVerts.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0],  aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 7
		for (auto x : boxindices)
		{
			g_debugDrawIndices.push_back(x + static_cast<uint32_t>(sz));
		}
	}
	else
	{
		auto& debug = g_DebugDraws[loc];

		auto sz = debug.vertex.size();
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0], -aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); //0
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0],  aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); // 1
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0], -aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 2
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0], -aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); // 3
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{ -aabb.halfExt[0],  aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 4
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0],  aabb.halfExt[1], -aabb.halfExt[2] },{/*normal*/},col }); // 5
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0], -aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 6
		debug.vertex.push_back(oGFX::Vertex{ aabb.center + Point3D{  aabb.halfExt[0],  aabb.halfExt[1],  aabb.halfExt[2] },{/*normal*/},col }); // 7
		for (auto x : boxindices)
		{
			debug.indices.push_back(x + static_cast<uint32_t>(sz));
		}
	}

	
}

void VulkanRenderer::AddDebugSphere(const Sphere& sphere, const oGFX::Color& col, size_t loc)
{
	static std::vector<oGFX::Vertex> vertices;
	static std::vector<uint32_t> indices;
	static bool once = [&]() {
		auto [sphVertices, spfIndices] = icosahedron::make_icosphere(2,false);
		vertices.reserve(sphVertices.size());
		for (auto&& v : sphVertices)
		{
			vertices.emplace_back(oGFX::Vertex{ v });
		}
		indices.reserve(spfIndices.size() * 3);
		for (auto&& ind : spfIndices) 
		{
			indices.emplace_back(ind.vertex[0]);
			indices.emplace_back(ind.vertex[1]);
			indices.emplace_back(ind.vertex[0]); 
			indices.emplace_back(ind.vertex[2]);
			indices.emplace_back(ind.vertex[2]);
			indices.emplace_back(ind.vertex[1]);
		}
		return true;
	}();
	
	if (loc == size_t(-1))
	{
		auto currsz = g_debugDrawVerts.size();
		g_debugDrawVerts.reserve(g_debugDrawVerts.size() + vertices.size());
		for (const auto& v : vertices)
		{
			oGFX::Vertex vert{ v };
			vert.pos = vert.pos*sphere.radius + sphere.center;
			vert.col = col;
			g_debugDrawVerts.push_back(vert);
		}

		g_debugDrawIndices.reserve( g_debugDrawIndices.size() + indices.size());
		for (const auto ind : indices) 
		{
			g_debugDrawIndices.emplace_back(ind+static_cast<uint32_t>(currsz));
		}
	}
	else
	{
		auto& debug = g_DebugDraws[loc];
		auto currsz = debug.vertex.size();
		debug.vertex.reserve(debug.vertex.size() + vertices.size());
		for (const auto& v : vertices)
		{
			oGFX::Vertex vert{ v };
			vert.pos = vert.pos*sphere.radius + sphere.center;
			vert.col = col;
			debug.vertex.push_back(vert);
		}
		debug.indices.reserve( debug.indices.size() + indices.size());
		for (const auto ind : indices) 
		{
			debug.indices.emplace_back(ind+static_cast<uint32_t>(currsz));
		}
	}
	
}

void VulkanRenderer::AddDebugTriangle(const Triangle& tri, const oGFX::Color& col, size_t loc)
{

	if (loc == size_t(-1))
	{
		auto sz = g_debugDrawVerts.size();
		g_debugDrawVerts.push_back(oGFX::Vertex{ tri.v0,{/*normal*/},col }); //0
		g_debugDrawVerts.push_back(oGFX::Vertex{ tri.v1,{/*normal*/},col }); //1
		g_debugDrawVerts.push_back(oGFX::Vertex{ tri.v2,{/*normal*/},col }); //2
		
		g_debugDrawIndices.push_back(0 + static_cast<uint32_t>(sz)); // E0
		g_debugDrawIndices.push_back(1 + static_cast<uint32_t>(sz)); // E0
		g_debugDrawIndices.push_back(1 + static_cast<uint32_t>(sz)); // E1
		g_debugDrawIndices.push_back(2 + static_cast<uint32_t>(sz)); // E1
		g_debugDrawIndices.push_back(2 + static_cast<uint32_t>(sz)); // E2
		g_debugDrawIndices.push_back(0 + static_cast<uint32_t>(sz)); // E2
		
	}
	else
	{
		auto& debug = g_DebugDraws[loc];

		auto sz = debug.vertex.size();
		debug.vertex.push_back(oGFX::Vertex{ tri.v0,{/*normal*/},col }); //0
		debug.vertex.push_back(oGFX::Vertex{ tri.v1,{/*normal*/},col }); //1
		debug.vertex.push_back(oGFX::Vertex{ tri.v2,{/*normal*/},col }); //2

		debug.indices.push_back(0 + static_cast<uint32_t>(sz)); // E0
		debug.indices.push_back(1 + static_cast<uint32_t>(sz)); // E0
		debug.indices.push_back(1 + static_cast<uint32_t>(sz)); // E1
		debug.indices.push_back(2 + static_cast<uint32_t>(sz)); // E1
		debug.indices.push_back(2 + static_cast<uint32_t>(sz)); // E2
		debug.indices.push_back(0 + static_cast<uint32_t>(sz)); // E2
	}
}



void VulkanRenderer::InitializeRenderBuffers()
{
	// In this function, all global rendering related buffers should be initialized, ONCE.

	// Note: Moved here from VulkanRenderer::UpdateIndirectCommands
    m_device.CreateBuffer(
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &indirectCommandsBuffer,
        MAX_OBJECTS * sizeof(oGFX::IndirectCommand));
    VK_NAME(m_device.logicalDevice, "Indirect Command Buffer", indirectCommandsBuffer.buffer);

	// Note: Moved here from VulkanRenderer::UpdateInstanceData
    m_device.CreateBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &instanceBuffer,
        MAX_OBJECTS * sizeof(oGFX::InstanceData));
    VK_NAME(m_device.logicalDevice, "Instance Buffer", instanceBuffer.buffer);

	constexpr uint32_t MAX_LIGHTS = 512;
	// TODO: Currently this is only for OmniLightInstance.
	// You should also support various light types such as spot lights, etc...

	m_device.CreateBuffer(
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		&globalLightBuffer,
		MAX_LIGHTS * sizeof(SpotLightInstance));
    VK_NAME(m_device.logicalDevice, "Light Buffer", globalLightBuffer.buffer);

	constexpr uint32_t MAX_GLOBAL_BONES = 2048;
	constexpr uint32_t MAX_SKINNING_VERTEX_BUFFER_SIZE = 4 * 1024 * 1024; // 4MB

    m_device.CreateBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &boneMatrixBuffer,
		MAX_GLOBAL_BONES * sizeof(glm::mat4x4));
    VK_NAME(m_device.logicalDevice, "Bone Matrix Buffer", boneMatrixBuffer.buffer);

    m_device.CreateBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &skinningVertexBuffer,
		MAX_SKINNING_VERTEX_BUFFER_SIZE);
    VK_NAME(m_device.logicalDevice, "Skinning Vertex Buffer", skinningVertexBuffer.buffer);


	// TODO: Move other global GPU buffer initialization here...
}

void VulkanRenderer::DestroyRenderBuffers()
{
	indirectCommandsBuffer.destroy();
	instanceBuffer.destroy();
	globalLightBuffer.destroy();
	boneMatrixBuffer.destroy();
	skinningVertexBuffer.destroy();
}

void VulkanRenderer::GenerateCPUIndirectDrawCommands()
{
	PROFILE_SCOPED();

	auto gb = GraphicsBatch::Init(currWorld, this, MAX_OBJECTS);
	gb.GenerateBatches();
	auto& allObjectsCommands = gb.GetBatch(GraphicsBatch::ALL_OBJECTS);

	objectCount = 0;
	for (auto& indirectCmd : allObjectsCommands)
	{
		objectCount += indirectCmd.instanceCount;
	}

	vkutils::Buffer stagingBuffer;	
	m_device.CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		allObjectsCommands.size() * sizeof(oGFX::IndirectCommand),
		allObjectsCommands.data());

	// Better to catch this on the software side early than the Vulkan validation layer
	// TODO: Fix this gracefully
	if (allObjectsCommands.size() > MAX_OBJECTS)
	{
		MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of indirect command buffer.", L"BAD ERROR");
	}

	m_device.CopyBuffer(&stagingBuffer, &indirectCommandsBuffer, m_device.graphicsQueue);

	stagingBuffer.destroy();
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


	if (currWorld)
	{
		for (auto& ent :  currWorld->GetAllObjectInstances())
		{
			// TODO: This needs urgent fixing..
			size_t x = gpuTransform.size();
			size_t len = x + models[ent.modelID].meshCount;
			mat4 xform = ent.localToWorld;
			for (; x < len; x++)
			{
				GPUTransform gpt;
				gpt.row0 = vec4(xform[0][0], xform[1][0], xform[2][0], xform[3][0]);
				gpt.row1 = vec4(xform[0][1], xform[1][1], xform[2][1], xform[3][1]);
				gpt.row2 = vec4(xform[0][2], xform[1][2], xform[2][2], xform[3][2]);
				gpuTransform.emplace_back(gpt);
			}
		}
	}
	
	gpuTransformBuffer.writeTo(gpuTransform.size(), gpuTransform.data());
	// TODO: Must the entire buffer be uploaded every frame?

	std::vector<oGFX::InstanceData> instanceData;
	instanceData.reserve(objectCount);
	if (currWorld)
	{
		uint32_t matCnt = 0;
		for (auto& ent: currWorld->GetAllObjectInstances())
		{
			oGFX::InstanceData id;
			size_t sz = instanceData.size();
			for (size_t x = 0; x < models[ent.modelID].meshCount; x++)
			{
				// This is per entity. Should be per material.
				uint32_t albedo = ent.bindlessGlobalTextureIndex_Albedo;
				uint32_t normal = ent.bindlessGlobalTextureIndex_Normal;
				uint32_t roughness = ent.bindlessGlobalTextureIndex_Roughness;
				uint32_t metallic = ent.bindlessGlobalTextureIndex_Metallic;
				constexpr uint32_t invalidIndex = 0xFFFFFFFF;
				if (albedo == invalidIndex)
					albedo = 0;
				if (normal == invalidIndex)
					normal = 1;
				if (roughness == invalidIndex)
					roughness = 0;
				if (metallic == invalidIndex)
					metallic = 1;

				uint32_t albedo_normal = albedo << 16 | (normal & 0xFFFF) ;
				uint32_t roughness_metallic = roughness << 16 | (metallic & 0xFFFF);

				id.instanceAttributes = uvec4(sz+x, matCnt, albedo_normal, roughness_metallic);
				instanceData.emplace_back(id);
			}
			++matCnt;
		}
		
	}
	

	vkutils::Buffer stagingBuffer;
	m_device.CreateBuffer(
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer,
		instanceData.size() * sizeof(oGFX::InstanceData),
		instanceData.data());

    // Better to catch this on the software side early than the Vulkan validation layer
	// TODO: Fix this gracefully
    if (instanceData.size() > MAX_OBJECTS)
    {
		MESSAGE_BOX_ONCE(windowPtr->GetRawHandle(), L"You just busted the max size of instance buffer.", L"BAD ERROR");
    }

	m_device.CopyBuffer(&stagingBuffer, &instanceBuffer, m_device.graphicsQueue);

	stagingBuffer.destroy();
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
	if (currWorld == nullptr) 
		return;

	PROFILE_SCOPED();

	//wait for given fence to signal from last draw before continuing
	VK_CHK(vkWaitForFences(m_device.logicalDevice, 1, &drawFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max()));
	//mainually reset fences
	VK_CHK(vkResetFences(m_device.logicalDevice, 1, &drawFences[currentFrame]));
	
	UpdateUniformBuffers();
	UploadInstanceData();	
	GenerateCPUIndirectDrawCommands();

	{
		PROFILE_SCOPED("vkAcquireNextImageKHR");

        //1. get the next available image to draw to and set something to signal when we're finished with the image ( a semaphore )
		// -- GET NEXT IMAGE
		//get  index of next image to be drawn to , and signal semaphore when ready to be drawn to
        VkResult res = vkAcquireNextImageKHR(m_device.logicalDevice, m_swapchain.swapchain, std::numeric_limits<uint64_t>::max(),
            imageAvailable[currentFrame], VK_NULL_HANDLE, &swapchainIdx);
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
	if (currWorld == nullptr)
		return;

	this->BeginDraw(); // TODO: Clean this up...

	UpdateDebugBuffers();
    {
		// Command list has already started inside VulkanRenderer::Draw
        PROFILE_GPU_CONTEXT(commandBuffers[swapchainIdx]);
        PROFILE_GPU_EVENT("CommandList");

        //this->SimplePass(); // Unsued
		// Manually schedule the order of the render pass execution. (single threaded)
		{
            RenderPassDatabase::GetRenderPass<ShadowPass>()->Draw();
            RenderPassDatabase::GetRenderPass<GBufferRenderPass>()->Draw();
			RenderPassDatabase::GetRenderPass<DeferredCompositionRenderpass>()->Draw();
			RenderPassDatabase::GetRenderPass<DebugRenderpass>()->Draw();
		}
    }
}

void VulkanRenderer::Present()
{
	if (currWorld == nullptr) 
		return;

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
	submitInfo.pWaitDstStageMask = waitStages; //stages to check semapheres at
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[swapchainIdx];	// command buffer to submit
	submitInfo.signalSemaphoreCount = 1;						// number of semaphores to signal
	submitInfo.pSignalSemaphores = &renderFinished[currentFrame];				// semphores to signal when command buffer finished

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
		Window::PollEvents();
		if (windowPtr->windowShouldClose) return false;
	}
	m_swapchain.Init(m_instance, m_device);
	CreateRenderpass();
	//CreateDepthBufferImage();
	CreateFramebuffers();

	ResizeGUIBuffers();
	ResizeOffscreenFB();
	ResizeDeferredFB();

	return true;
}

void VulkanRenderer::InitTreeDebugDraws()
{
	for (size_t i = 0; i < debugDrawBufferCnt; i++)
	{
		g_DebugDraws[i].vbo.Init(&m_device,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		g_DebugDraws[i].ibo.Init(&m_device,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	}
}

void VulkanRenderer::ShutdownTreeDebug()
{
	for (size_t i = 0; i < debugDrawBufferCnt; i++)
	{
		g_DebugDraws[i].vbo.destroy();
		g_DebugDraws[i].ibo.destroy();
	}
}

Model* VulkanRenderer::LoadModelFromFile(const std::string& file)
{
	// new model loader
	
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(file,
		  aiProcess_Triangulate                // Make sure we get triangles rather than nvert polygons
		| aiProcess_LimitBoneWeights           // 4 weights for skin model max
		| aiProcess_GenUVCoords                // Convert any type of mapping to uv mapping
		| aiProcess_TransformUVCoords          // preprocess UV transformations (scaling, translation ...)
		| aiProcess_FindInstances              // search for instanced meshes and remove them by references to one master
		| aiProcess_CalcTangentSpace           // calculate tangents and bitangents if possible
		| aiProcess_JoinIdenticalVertices      // join identical vertices/ optimize indexing
		| aiProcess_RemoveRedundantMaterials   // remove redundant materials
		| aiProcess_FindInvalidData            // detect invalid model data, such as invalid normal vectors
		| aiProcess_PreTransformVertices       // TODO: remove for skinning?
		| aiProcess_FlipUVs						// TODO: some mesh need
		| aiProcess_GenNormals					// TODO: some mesh need
	);

	if (!scene)
	{
		return nullptr; // Dont explode...
		//throw std::runtime_error("Failed to load model! (" + file + ")");
	}

	std::vector<std::string> textureNames = MeshContainer::LoadMaterials(scene);
	std::vector<int> matToTex(textureNames.size());
	// Loop over textureNames and create textures for them
	for (size_t i = 0; i < textureNames.size(); i++)
	{
		// if material had no texture, set '0' to indicate no texture, texture 0 will be reserved fora  default texture
		if (textureNames[i].empty())
		{
			matToTex[i] = 0;
		}
		else
		{
			// otherwise create texture and set value to index of new texture
			matToTex[i] = CreateTexture(textureNames[i]);
		}
	}

	auto modelResourceIndex = models.size();
	auto& model = models.emplace_back(std::move(gfxModel()));

	for (auto& node : model.nodes)
	{
		for (auto& mesh : node->meshes)
		{
			mesh->textureIndex = matToTex[mesh->textureIndex];
		}
	}

	Model* m = new Model;
	m->gfxIndex = static_cast<uint32_t>(modelResourceIndex);
	model.cpuModel = m;
	//std::vector<oGFX::Vertex> verticeBuffer;
	//std::vector<uint32_t> indexBuffer;
	model.meshCount= 0 ;
	model.loadNode(nullptr, scene, *scene->mRootNode, 0, m->vertices, m->indices);

	for (auto& node : model.nodes)
	{
		for (auto& mesh : node->meshes)
		{
			model.meshCount += 1;
			mesh->indicesOffset += g_GlobalMeshBuffers.IdxOffset;
			mesh->vertexOffset += g_GlobalMeshBuffers.VtxOffset;
		}
		for (auto& child: node->children)
		{
			for (auto& mesh : child->meshes)
			{
				model.meshCount += 1;
				mesh->indicesOffset += g_GlobalMeshBuffers.IdxOffset;
				mesh->vertexOffset += g_GlobalMeshBuffers.VtxOffset;
			}
		}
	}
	
	LoadMeshFromBuffers(m->vertices, m->indices, &model);

	return m;
}

Model* VulkanRenderer::LoadMeshFromBuffers(std::vector<oGFX::Vertex>& vertex, std::vector<uint32_t>& indices, gfxModel* model)
{
	uint32_t index = 0;
	Model* m{ nullptr };

	if (model == nullptr)
	{
		index = static_cast<uint32_t>(models.size());
		models.emplace_back(std::move(gfxModel()));
		model = &models[index];
		Node* n = new Node{};
		oGFX::Mesh* msh = new oGFX::Mesh{};
		msh->indicesOffset = static_cast<uint32_t>(g_GlobalMeshBuffers.IdxOffset);
		msh->vertexOffset = static_cast<uint32_t>(g_GlobalMeshBuffers.VtxOffset);
		msh->indicesCount = static_cast<uint32_t>(indices.size());
		msh->vertexCount = static_cast<uint32_t>(vertex.size());
		model->meshCount= 1;
		n->meshes.push_back(msh);
		model->nodes.push_back(n);

		m = new Model();
		m->vertices = vertex;
		m->indices = indices;
		m->gfxIndex = static_cast<uint32_t>(index);

		model->cpuModel = m;
	}

	model->indices.count = static_cast<uint32_t>(indices.size());
	model->vertices.count = static_cast<uint32_t>(vertex.size());

	g_GlobalMeshBuffers.IdxBuffer.writeTo(indices.size(), indices.data(), g_GlobalMeshBuffers.IdxOffset);
	g_GlobalMeshBuffers.VtxBuffer.writeTo(vertex.size(), vertex.data(), g_GlobalMeshBuffers.VtxOffset);

	model->indices.offset = g_GlobalMeshBuffers.IdxOffset;
	model->vertices.offset = g_GlobalMeshBuffers.VtxOffset;

	g_GlobalMeshBuffers.IdxOffset += model->indices.count ;
	g_GlobalMeshBuffers.VtxOffset += model->vertices.count;

	return m;

	//{
	//	using namespace oGFX;
	//	//get size of buffer needed for vertices
	//	VkDeviceSize bufferSize = sizeof(Vertex) * vertex.size();
	//
	//	//temporary buffer to stage vertex data before transferring to GPU
	//	VkBuffer stagingBuffer;
	//	VkDeviceMemory stagingBufferMemory; 
	//	//create buffer and allocate memory to it
	//
	//	CreateBuffer(m_device.physicalDevice,m_device.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
	//
	//	//MAP MEMORY TO VERTEX BUFFER
	//	void *data = nullptr;												//1. create a pointer to a point in normal memory
	//	vkMapMemory(m_device.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);	//2. map the vertex buffer to that point
	//	memcpy(data, vertex.data(), (size_t)bufferSize);					//3. copy memory from vertices vector to the point
	//	vkUnmapMemory(m_device.logicalDevice, stagingBufferMemory);							//4. unmap the vertex buffer memory
	//
	//																						//create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	//																						// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by the GPU and not the CPU (host)
	//	CreateBuffer(m_device.physicalDevice, m_device.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &model->vertices.buffer, &model->vertices.memory); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT make this buffer local to the GPU
	//
	//																							  //copy staging buffer to vertex buffer on GPU
	//	CopyBuffer(m_device.logicalDevice, m_device.graphicsQueue, m_device.commandPool, stagingBuffer, model->vertices.buffer, bufferSize);
	//
	//	//clean up staging buffer parts
	//	vkDestroyBuffer(m_device.logicalDevice, stagingBuffer, nullptr);
	//	vkFreeMemory(m_device.logicalDevice, stagingBufferMemory, nullptr);
	//}
	//
	////CreateIndexBuffer
	//{
	//	using namespace oGFX;
	//	//get size of buffer needed for vertices
	//	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();
	//
	//	//temporary buffer to stage vertex data before transferring to GPU
	//	VkBuffer stagingBuffer;
	//	VkDeviceMemory stagingBufferMemory; 
	//	//create buffer and allocate memory to it
	//	CreateBuffer(m_device.physicalDevice,m_device.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	//		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);
	//
	//	//MAP MEMORY TO VERTEX BUFFER
	//	void *data = nullptr;												//1. create a pointer to a point in normal memory
	//	vkMapMemory(m_device.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);	//2. map the vertex buffer to that point
	//	memcpy(data, indices.data(), (size_t)bufferSize);					//3. copy memory from vertices vector to the point
	//	vkUnmapMemory(m_device.logicalDevice, stagingBufferMemory);				//4. unmap the vertex buffer memory
	//
	//																			//create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
	//																			// buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by the GPU and not the CPU (host)
	//	CreateBuffer(m_device.physicalDevice, m_device.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
	//		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &model->indices.buffer, &model->indices.memory); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT make this buffer local to the GPU
	//
	//																							//copy staging buffer to vertex buffer on GPU
	//	CopyBuffer(m_device.logicalDevice, m_device.graphicsQueue, m_device.commandPool, stagingBuffer, model->indices.buffer, bufferSize);
	//
	//	//clean up staging buffer parts
	//	vkDestroyBuffer(m_device.logicalDevice, stagingBuffer, nullptr);
	//	vkFreeMemory(m_device.logicalDevice, stagingBufferMemory, nullptr);
	//}
	//return m;
}

void VulkanRenderer::SetMeshTextures(uint32_t modelID, uint32_t alb, uint32_t norm, uint32_t occlu, uint32_t rough)
{
	models[modelID].textures = { alb,norm,occlu,rough };
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
	int descriptorLoc = UpdateBindlessGlobalTexture(g_Textures[ind]);

	//return location of set with texture
	return descriptorLoc;

}

uint32_t VulkanRenderer::CreateTexture(const std::string& file)
{
	// Create texture image and get its location in array
	uint32_t textureImageLoc = CreateTextureImage(file);

	//create texture descriptor
	int descriptorLoc = UpdateBindlessGlobalTexture(g_Textures[textureImageLoc]);

	//return location of set with texture
	return descriptorLoc;
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
	g_debugDrawVertBuffer.Init(&m_device,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	g_debugDrawIndxBuffer.Init(&m_device,VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void VulkanRenderer::UpdateDebugBuffers()
{
	g_debugDrawVertBuffer.reserve(g_debugDrawVerts.size() );
	g_debugDrawIndxBuffer.reserve(g_debugDrawIndices.size());

	g_debugDrawVertBuffer.writeTo(g_debugDrawVerts.size() , g_debugDrawVerts.data());
	g_debugDrawIndxBuffer.writeTo(g_debugDrawIndices.size() , g_debugDrawIndices.data());
}

void VulkanRenderer::UpdateUniformBuffers()
{		
	PROFILE_SCOPED();

	float height = static_cast<float>(windowPtr->m_height);
	float width = static_cast<float>(windowPtr->m_width);
	float ar = width / height;

	CB::FrameContextUBO m_FrameContextUBO;
	m_FrameContextUBO.projection = camera.matrices.perspective;
	m_FrameContextUBO.view = camera.matrices.view;
	m_FrameContextUBO.viewProjection = m_FrameContextUBO.projection * m_FrameContextUBO.view;
	m_FrameContextUBO.cameraPosition = glm::vec4(camera.position,1.0);
	m_FrameContextUBO.renderTimer.x += 1 / 60.0f; // Fake total time... (TODO: Fix me)
	m_FrameContextUBO.renderTimer.y = glm::sin(m_FrameContextUBO.renderTimer.x * 0.5f * glm::pi<float>());
	m_FrameContextUBO.renderTimer.z = glm::cos(m_FrameContextUBO.renderTimer.x * 0.5f * glm::pi<float>());
	m_FrameContextUBO.renderTimer.w = 0.0f; // unused

	void *data;
	vkMapMemory(m_device.logicalDevice, vpUniformBufferMemory[swapchainIdx], 0, uboDynamicAlignment, 0, &data);
	memcpy(data, &m_FrameContextUBO, sizeof(CB::FrameContextUBO));

	VkMappedMemoryRange memRng{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE};
	memRng.memory = vpUniformBufferMemory[swapchainIdx];
	memRng.offset = 0;
	memRng.size = uboDynamicAlignment;
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

	auto indx = g_Textures.size();
	g_Textures.push_back(vkutils::Texture2D());

	auto& texture = g_Textures[indx];
	
	texture.fromBuffer((void*)imageInfo.imgData.data(), imageSize, imageInfo.format, imageInfo.w, imageInfo.h,imageInfo.mipInformation, &m_device, m_device.graphicsQueue);
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

uint32_t VulkanRenderer::UpdateBindlessGlobalTexture(vkutils::Texture2D texture)
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
void SetDefaultViewportAndScissor(VkCommandBuffer commandBuffer)
{
	auto& vr = *VulkanRenderer::get();
    auto* windowPtr = vr.windowPtr;
    const float vpHeight = (float)vr.m_swapchain.swapChainExtent.height;
    const float vpWidth = (float)vr.m_swapchain.swapChainExtent.width;
    VkViewport viewport = { 0.0f, vpHeight, vpWidth, -vpHeight, 0.0f, 1.0f };
    VkRect2D scissor = { {0, 0}, {uint32_t(windowPtr->m_width), uint32_t(windowPtr->m_height) } };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
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
