#include "DebugRenderpass.h"

#include <array>
#include <typeindex>

#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"

DECLARE_RENDERPASS(DebugRenderpass);

void DebugRenderpass::Init()
{
	CreatePushconstants();
	CreateDebugRenderpass();
	CreatePipeline();
	InitDebugBuffers();
}

void DebugRenderpass::Draw()
{
	auto swapchainIdx = VulkanRenderer::swapchainIdx;
	auto* windowPtr = VulkanRenderer::windowPtr;
	auto& commandBuffers = VulkanRenderer::commandBuffers;

	std::array<VkClearValue, 2> clearValues{};
	//clearValues[0].color = { 0.6f,0.65f,0.4f,1.0f };
	clearValues[0].color = { 0.1f,0.1f,0.1f,1.0f };
	clearValues[1].depthStencil.depth = {1.0f };
	//Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vk::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = debugRenderpass;									//render pass to begin
	renderPassBeginInfo.renderArea.offset = { 0,0 };								//start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = VulkanRenderer::m_swapchain.swapChainExtent;			//size of region to run render pass on (Starting from offset)
	renderPassBeginInfo.pClearValues = clearValues.data();							//list of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); // no clearing

	renderPassBeginInfo.framebuffer =  VulkanRenderer::swapChainFramebuffers[swapchainIdx];
	
	const VkCommandBuffer cmdlist = VulkanRenderer::commandBuffers[swapchainIdx];
    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("DebugRender");

	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	SetDefaultViewportAndScissor(cmdlist);

	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_GRAPHICS, linesPipeline);

	uint32_t dynamicOffset = 0;
	std::array<VkDescriptorSet, 3> descriptorSetGroup =
	{ 
		VulkanRenderer::g0_descriptors,  
		VulkanRenderer::uniformDescriptorSets[swapchainIdx],
		VulkanRenderer::globalSamplers 
	};
	vkCmdBindDescriptorSets(cmdlist,VK_PIPELINE_BIND_POINT_GRAPHICS,  VulkanRenderer::indirectPipeLayout,
		0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 1, &dynamicOffset);

	glm::mat4 xform{ 1.0f };
	vkCmdPushConstants(cmdlist,
		VulkanRenderer::indirectPipeLayout,
		VK_SHADER_STAGE_ALL,    	// stage to push constants to
		0,							// offset of push constants to update
		sizeof(glm::mat4),			// size of data being pushed
		glm::value_ptr(xform));		// actualy data being pushed (could be an array));

	VkDeviceSize offsets[] = { 0 };	

	// just draw the whole set of debug stuff
	vkCmdBindIndexBuffer(cmdlist,  VulkanRenderer::g_debugDrawIndxBuffer.getBuffer(),0, VK_INDEX_TYPE_UINT32);
	vkCmdBindVertexBuffers(cmdlist, VERTEX_BUFFER_ID, 1,VulkanRenderer::g_debugDrawVertBuffer.getBufferPtr(), offsets);
	vkCmdDrawIndexed(cmdlist, static_cast<uint32_t>(VulkanRenderer::g_debugDrawIndxBuffer.size()) , 1, 0, 0, 0);

	for (size_t i = 0; i < VulkanRenderer::debugDrawBufferCnt; i++)
	{
		if (VulkanRenderer::g_b_drawDebug[i])
		{
			auto& debug = VulkanRenderer::g_DebugDraws[i];
			vkCmdBindIndexBuffer(cmdlist,  debug.ibo.getBuffer(),0, VK_INDEX_TYPE_UINT32);
			vkCmdBindVertexBuffers(cmdlist, VERTEX_BUFFER_ID, 1, debug.vbo.getBufferPtr(), offsets);
			vkCmdDrawIndexed(cmdlist, static_cast<uint32_t>(debug.indices.size()) , 1, 0, 0, 0);
		}
	}

	vkCmdEndRenderPass(cmdlist);
}

void DebugRenderpass::Shutdown()
{
	auto& m_device = VulkanRenderer::m_device;

	vkDestroyRenderPass(m_device.logicalDevice, debugRenderpass, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, linesPipeline, nullptr);
	VulkanRenderer::g_debugDrawVertBuffer.destroy();
	VulkanRenderer::g_debugDrawIndxBuffer.destroy();
}

void DebugRenderpass::CreateDebugRenderpass()
{
	VkAttachmentDescription colourAttachment = {};
	colourAttachment.format = VulkanRenderer::m_swapchain.swapChainImageFormat;  //format to use for attachment
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

																	   // Depth attachment of render pass
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = oGFX::ChooseSupportedFormat(VulkanRenderer::m_device,
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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


	VK_CHK( vkCreateRenderPass(VulkanRenderer::m_device.logicalDevice, &renderPassCreateInfo, nullptr, &debugRenderpass));
	VK_NAME(VulkanRenderer::m_device.logicalDevice, "debugRenderpass", debugRenderpass);

}

void DebugRenderpass::CreatePipeline()
{
	using namespace oGFX;

	auto& bindingDescription = GetGFXVertexInputBindings();
	auto& attributeDescriptions= GetGFXVertexInputAttributes();
	auto& m_device = VulkanRenderer::m_device;
	
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vk::inits::pipelineVertexInputStateCreateInfo(bindingDescription,attributeDescriptions);
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = oGFX::vk::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0 ,VK_FALSE);
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = oGFX::vk::inits::pipelineViewportStateCreateInfo(1,1,0);
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = oGFX::vk::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL,VK_CULL_MODE_BACK_BIT,VK_FRONT_FACE_COUNTER_CLOCKWISE,0);
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = oGFX::vk::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT,0);

	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = oGFX::vk::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	
	VkPipelineColorBlendAttachmentState colourState = oGFX::vk::inits::pipelineColorBlendAttachmentState(0x0000000F,VK_TRUE);
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = oGFX::vk::inits::pipelineColorBlendStateCreateInfo(1,&colourState);

	// -- PIPELINE LAYOUT 
	std::array<VkDescriptorSetLayout, 3> descriptorSetLayouts = {VulkanRenderer::g0_descriptorsLayout, 
																VulkanRenderer::descriptorSetLayout,
																VulkanRenderer::samplerSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =  
		oGFX::vk::inits::pipelineLayoutCreateInfo(descriptorSetLayouts.data(),static_cast<uint32_t>(descriptorSetLayouts.size()));
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	// indirect pipeline
	//VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &VulkanRenderer::indirectPipeLayout));
	//VK_NAME(VulkanRenderer::m_device.logicalDevice, "indirectPipeLayout", indirectPipeLayout);
	// go back to normal pipelines

	// Create Pipeline Layout
	//result = vkCreatePipelineLayout(m_device.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	//if (result != VK_SUCCESS)
	//{
	//	throw std::runtime_error("Failed to create Pipeline Layout!");
	//}
	std::array<VkPipelineShaderStageCreateInfo,2>shaderStages = {};
	
	// -- DEPTH STENCIL TESTING --	
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = oGFX::vk::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE,VK_TRUE, VK_COMPARE_OP_LESS);

																	// -- GRAPHICS PIPELINE CREATION --
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = oGFX::vk::inits::pipelineCreateInfo(VulkanRenderer::indirectPipeLayout,VulkanRenderer::defaultRenderPass);
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

	// we use less for normal pipeline
	vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 5;

	shaderStages[0] = VulkanRenderer::LoadShader(m_device,"Shaders/bin/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = VulkanRenderer::LoadShader(m_device,"Shaders/bin/shader.frag.spv",VK_SHADER_STAGE_FRAGMENT_BIT);
	
	rasterizerCreateInfo.polygonMode = VkPolygonMode::VK_POLYGON_MODE_LINE;
	inputAssembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	rasterizerCreateInfo.lineWidth = 1.0f;
	//rasterizerCreateInfo.cullMode = VK_CULL_MODE_NONE;
	pipelineCreateInfo.renderPass = debugRenderpass;
	depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
	//depthStencilCreateInfo.depthTestEnable = VK_FALSE;
	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &linesPipeline));
	VK_NAME(m_device.logicalDevice, "linesPipeline", linesPipeline);

	//destroy shader modules after pipeline is created
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}

void DebugRenderpass::CreatePushconstants()
{
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //shader stage push constant will go to
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(VulkanRenderer::PushConstData);
}

void DebugRenderpass::InitDebugBuffers()
{
}
