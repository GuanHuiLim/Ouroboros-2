/************************************************************************************//*!
\file           DebugRenderpass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a debug drawing pass

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "DebugRenderpass.h"

#include <array>
#include <typeindex>

#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "FramebufferBuilder.h"
#include "GBufferRenderPass.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"

DECLARE_RENDERPASS(DebugRenderpass);

DECLARE_RENDERPASS(DebugDrawRenderpass);

void DebugDrawRenderpass::Init()
{
	CreateDebugRenderpass();
	CreatePipeline();
	InitDebugBuffers();
}

bool DebugDrawRenderpass::SetupDependencies()
{
	// TODO: If debug drawing is disabled, return false.
	
	// READ: Scene Depth
	// WRITE: Color Output
	// etc

	return true;
}

void DebugDrawRenderpass::Draw()
{
	auto& vr = *VulkanRenderer::get();

	auto swapchainIdx = vr.swapchainIdx;
	auto* windowPtr = vr.windowPtr;
	auto& commandBuffers = vr.commandBuffers;

	std::array<VkClearValue, 2> clearValues{};
	//clearValues[0].color = { 0.6f,0.65f,0.4f,1.0f };
	clearValues[0].color = { 0.1f,0.1f,0.1f,1.0f };
	clearValues[1].depthStencil.depth = {1.0f };
	//Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = debugRenderpass;									//render pass to begin
	renderPassBeginInfo.renderArea.offset = { 0,0 };								//start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = vr.m_swapchain.swapChainExtent;			//size of region to run render pass on (Starting from offset)
	renderPassBeginInfo.pClearValues = clearValues.data();							//list of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); // no clearing

	auto& depthAtt = RenderPassDatabase::GetRenderPass<GBufferRenderPass>()->attachments[GBufferAttachmentIndex::DEPTH];

	VkFramebuffer fb;
	FramebufferBuilder::Begin(&vr.fbCache)
		.BindImage(&vr.currWorld->renderTargets[vr.renderIteration])
		.BindImage(&vr.currWorld->depthTargets[vr.renderIteration])
		.Build(fb,debugRenderpass);

	renderPassBeginInfo.framebuffer = fb;
	
	const VkCommandBuffer cmdlist = vr.commandBuffers[swapchainIdx];
	PROFILE_GPU_CONTEXT(cmdlist);
	PROFILE_GPU_EVENT("DebugDraw");

	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	rhi::CommandList cmd{ cmdlist };
	{
		cmd.SetDefaultViewportAndScissor();

		uint32_t dynamicOffset = static_cast<uint32_t>(vr.renderIteration * oGFX::vkutils::tools::UniformBufferPaddedSize(sizeof(CB::FrameContextUBO), 
			vr.m_device.properties.limits.minUniformBufferOffsetAlignment));
		cmd.BindDescriptorSet(PSOLayoutDB::defaultPSOLayout, 0, 
			std::array<VkDescriptorSet, 3>
			{
				vr.descriptorSet_gpuscene,
				vr.descriptorSets_uniform[swapchainIdx],
				vr.descriptorSet_bindless
			},
			1, &dynamicOffset
		);

		cmd.BindVertexBuffer(BIND_POINT_VERTEX_BUFFER_ID, 1, vr.g_DebugDrawVertexBufferGPU.getBufferPtr());
		cmd.BindIndexBuffer(vr.g_DebugDrawIndexBufferGPU.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		VkPipeline pso = m_DebugDrawPSOSelector.GetPSO(vr.m_DebugDrawDepthTest, false, false);
		cmd.BindPSO(pso);
		cmd.DrawIndexed((uint32_t)(vr.g_DebugDrawIndexBufferGPU.size()), 1);
	}


	vkCmdEndRenderPass(cmdlist);
}

void DebugDrawRenderpass::Shutdown()
{
	VulkanRenderer* vr = VulkanRenderer::get();
	auto& device = vr->m_device.logicalDevice;

	vkDestroyRenderPass(device, debugRenderpass, nullptr);

	for (auto& pso : m_DebugDrawPSOSelector.psos)
	{
		if (pso)
		{
			vkDestroyPipeline(device, pso, nullptr);
		}
	}

	vr->g_DebugDrawVertexBufferGPU.destroy();
	vr->g_DebugDrawIndexBufferGPU.destroy();
}

void DebugDrawRenderpass::CreateDebugRenderpass()
{
	auto& vr = *VulkanRenderer::get();
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = vr.m_swapchain.swapChainImageFormat;  //format to use for attachment
	colourAttachment.samples = VK_SAMPLE_COUNT_1_BIT;//number of samples to use for multisampling
	colourAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;//descripts what to do with attachment before rendering
	colourAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;//describes what to do with attachment after rendering
	colourAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; //describes what do with with stencil before rendering
	colourAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; //describes what do with with stencil before rendering

																		//frame buffer data will be stored as image, but images can be given different data layouts
																		//to give optimal use for certain operations
	colourAttachment.initialLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; //image data layout before render pass starts
																		 //colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //image data layout aftet render pass ( to change to)
	colourAttachment.finalLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; //image data layout aftet render pass ( to change to)
	
	// If editor??
	//colourAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //image data layout aftet render pass ( to change to)

																	   // Depth attachment of render pass
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = vr.G_DEPTH_FORMAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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


	VK_CHK( vkCreateRenderPass(vr.m_device.logicalDevice, &renderPassCreateInfo, nullptr, &debugRenderpass));
	VK_NAME(vr.m_device.logicalDevice, "debugRenderpass", debugRenderpass);

}

void DebugDrawRenderpass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	const char* shaderVS = "Shaders/bin/debugdraw.vert.spv";
	const char* shaderPS = "Shaders/bin/debugdraw.frag.spv";

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages =
	{
		vr.LoadShader(m_device, shaderVS, VK_SHADER_STAGE_VERTEX_BIT),
		vr.LoadShader(m_device, shaderPS, VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	const std::vector<VkVertexInputBindingDescription> bindingDescription =
	{
		oGFX::vkutils::inits::vertexInputBindingDescription(BIND_POINT_VERTEX_BUFFER_ID,sizeof(oGFX::DebugVertex),VK_VERTEX_INPUT_RATE_VERTEX),
	};

	const std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
	{
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(oGFX::DebugVertex, pos)),
		oGFX::vkutils::inits::vertexInputAttributeDescription(BIND_POINT_VERTEX_BUFFER_ID,2,VK_FORMAT_R32G32B32_SFLOAT,offsetof(oGFX::DebugVertex, col)),
	};

	using oGFX::vkutils::inits::Creator;
	auto vertexInputCreateInfo   = Creator<VkPipelineVertexInputStateCreateInfo>(bindingDescription, attributeDescriptions);
	auto inputAssembly           = Creator<VkPipelineInputAssemblyStateCreateInfo>(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	auto viewportStateCreateInfo = Creator<VkPipelineViewportStateCreateInfo>();
	auto multisamplingCreateInfo = Creator<VkPipelineMultisampleStateCreateInfo>();
	auto rasterizerCreateInfo    = Creator<VkPipelineRasterizationStateCreateInfo>(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	const std::vector dynamicState{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	auto dynamicStateCreateInfo  = Creator<VkPipelineDynamicStateCreateInfo>(dynamicState);
	auto depthStencilCreateInfo  = Creator<VkPipelineDepthStencilStateCreateInfo>(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS);

	VkPipelineColorBlendAttachmentState colourState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0x0000000F,VK_TRUE);
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;
	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1,&colourState);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = oGFX::vkutils::inits::pipelineCreateInfo(PSOLayoutDB::defaultPSOLayout,vr.renderPass_default);
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.renderPass = debugRenderpass;

	// TESTING
	if constexpr(false)
	{
		oGFX::vkutils::inits::PSOCreatorWrapper psoCreator;
		psoCreator.Set<VkPipelineVertexInputStateCreateInfo>(bindingDescription, attributeDescriptions);
		psoCreator.Set<VkPipelineInputAssemblyStateCreateInfo>(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
		psoCreator.Set<VkPipelineViewportStateCreateInfo>();
		psoCreator.Set<VkPipelineMultisampleStateCreateInfo>();
		psoCreator.Set<VkPipelineRasterizationStateCreateInfo>(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		const std::vector dynamicState{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		psoCreator.Set<VkPipelineDynamicStateCreateInfo>(dynamicState);
		psoCreator.Set<VkPipelineDepthStencilStateCreateInfo>(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS);
		auto cbas = psoCreator.Get<VkPipelineColorBlendAttachmentState>() = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0x0000000F, VK_TRUE);
		cbas.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		cbas.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		cbas.colorBlendOp = VK_BLEND_OP_ADD;
		cbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		cbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		cbas.alphaBlendOp = VK_BLEND_OP_ADD;
		psoCreator.Get<VkPipelineColorBlendStateCreateInfo>() = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &colourState);
		psoCreator.SetRenderPass(debugRenderpass);
		psoCreator.SetAndCompile();
	}

	// Pre-build all permutations. Sadly, this is usually more safer than runtime compilation.
	{
		const std::array<VkPolygonMode, 2> fillmodes =
		{
			VkPolygonMode::VK_POLYGON_MODE_LINE, // Line/Wireframe
			VkPolygonMode::VK_POLYGON_MODE_FILL, // Solids
			//VkPolygonMode::VK_POLYGON_MODE_POINT // Points
		};

		const std::array<bool, 2> depthTests = 
		{
			false, true 
		};

		for (unsigned i = 0; i < fillmodes.size(); ++i)
		{
			const auto fillmode = fillmodes[i];
			rasterizerCreateInfo.polygonMode = fillmode;

			// Special case - not done.
			// Need to handle the shader.
			// Pipeline topology is set to POINT_LIST, but PointSize is not written to in the shader corresponding to VK_SHADER_STAGE_VERTEX_BIT.
			if (fillmode == VkPolygonMode::VK_POLYGON_MODE_POINT)
				inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

			for (unsigned j = 0; j < depthTests.size(); ++j)
			{
				const auto depthtest = depthTests[j];
				depthStencilCreateInfo.depthTestEnable = depthtest;

				// Create all permutations of PSO needed
				{
					VkPipeline& pso = m_DebugDrawPSOSelector.psos[i + 2ull * j];
					VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pso));
					VK_NAME(m_device.logicalDevice, "DebugDrawLinesPSO", pso);
				}
			}
		}

	}

	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}

void DebugDrawRenderpass::InitDebugBuffers()
{
}
