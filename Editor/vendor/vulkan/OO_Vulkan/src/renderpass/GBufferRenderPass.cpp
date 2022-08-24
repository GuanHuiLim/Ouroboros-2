#include "GBufferRenderPass.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "VulkanFramebufferAttachment.h"
#include "FramebufferCache.h"
#include "FramebufferBuilder.h"

#include "../shaders/shared_structs.h"
#include "MathCommon.h"

#include "DeferredCompositionRenderpass.h"

#include <array>

DECLARE_RENDERPASS(GBufferRenderPass);

void GBufferRenderPass::Init()
{
	SetupRenderpass();
	SetupFramebuffer();
}

void GBufferRenderPass::CreatePSO()
{
	CreatePipeline();
}

void GBufferRenderPass::Draw()
{
	if (!VulkanRenderer::deferredRendering)
		return;

	auto& device = VulkanRenderer::m_device;
	auto& swapchain = VulkanRenderer::m_swapchain;
	auto& commandBuffers = VulkanRenderer::commandBuffers;
	auto& swapchainIdx = VulkanRenderer::swapchainIdx;
	auto* windowPtr = VulkanRenderer::windowPtr;

    const VkCommandBuffer cmdlist = commandBuffers[swapchainIdx];
    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("GBuffer");

	constexpr VkClearColorValue zeroFloat4 = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear values for all attachments written in the fragment shader
	std::array<VkClearValue, GBufferAttachmentIndex::MAX_ATTACHMENTS> clearValues;
	clearValues[GBufferAttachmentIndex::POSITION].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::NORMAL]  .color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::ALBEDO]  .color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::MATERIAL].color = zeroFloat4;
	clearValues[GBufferAttachmentIndex::DEPTH]   .depthStencil = { 1.0f, 0 };
	
	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vk::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass =  renderpass_GBuffer;
	renderPassBeginInfo.framebuffer = framebuffer_GBuffer;
	renderPassBeginInfo.renderArea.extent.width = swapchain.swapChainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapchain.swapChainExtent.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	// VulkanRenderer::ResizeSwapchain() destroys the depth attachment. This causes the renderpass to fail on resize
	// TODO: handle all framebuffer resizes gracefully
	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	SetDefaultViewportAndScissor(cmdlist);

	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_GRAPHICS, pso_GBufferDefault);
	std::array<VkDescriptorSet, 3> descriptorSetGroup = 
	{
		VulkanRenderer::g0_descriptors,
		VulkanRenderer::uniformDescriptorSets[swapchainIdx],
		VulkanRenderer::globalSamplers
	};
	
	uint32_t dynamicOffset = 0;
	vkCmdBindDescriptorSets(cmdlist, VK_PIPELINE_BIND_POINT_GRAPHICS, VulkanRenderer::indirectPipeLayout,
		0, static_cast<uint32_t>(descriptorSetGroup.size()), descriptorSetGroup.data(), 1, &dynamicOffset);
	
	// Bind merged mesh vertex & index buffers, instancing buffers.
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmdlist, VERTEX_BUFFER_ID, 1, VulkanRenderer::g_MeshBuffers.VtxBuffer.getBufferPtr(), offsets);
    vkCmdBindIndexBuffer(cmdlist, VulkanRenderer::g_MeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindVertexBuffers(cmdlist, INSTANCE_BUFFER_ID, 1, &VulkanRenderer::instanceBuffer.buffer, offsets);

    const VkBuffer idcb = VulkanRenderer::indirectCommandsBuffer.buffer;
    const uint32_t count = (uint32_t)VulkanRenderer::m_DrawIndirectCommandsCPU.size();

	DrawIndexedIndirect(cmdlist, idcb, 0, count, sizeof(VkDrawIndexedIndirectCommand));

	// Other draw calls that are not supported by MDI
    // TODO: Deprecate this, or handle it gracefully. Leaving this here.
    if constexpr (false)
    {
        for (auto& entity : VulkanRenderer::entities)
        {
            auto& model = VulkanRenderer::models[entity.modelID];

            glm::mat4 xform(1.0f);
            xform = glm::translate(xform, entity.position);
            xform = glm::rotate(xform, glm::radians(entity.rot), entity.rotVec);
            xform = glm::scale(xform, entity.scale);

			vkCmdPushConstants(cmdlist,
				VulkanRenderer::indirectPipeLayout,
				VK_SHADER_STAGE_ALL,        // stage to push constants to
				0,                          // offset of push constants to update
				sizeof(glm::mat4),          // size of data being pushed
				glm::value_ptr(xform));     // actualy data being pushed (could be an array));

            VkDeviceSize offsets[] = { 0 };
            vkCmdBindIndexBuffer(cmdlist, VulkanRenderer::g_MeshBuffers.IdxBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindVertexBuffers(cmdlist, VERTEX_BUFFER_ID, 1, VulkanRenderer::g_MeshBuffers.VtxBuffer.getBufferPtr(), offsets);
            vkCmdBindVertexBuffers(cmdlist, INSTANCE_BUFFER_ID, 1, &VulkanRenderer::instanceBuffer.buffer, offsets);
            //vkCmdDrawIndexed(commandBuffers[swapchainImageIndex], model.indices.count, 1, model.indices.offset, model.vertices.offset, 0);
        }
    }

	vkCmdEndRenderPass(cmdlist);
}

void GBufferRenderPass::Shutdown()
{
	auto& m_device = VulkanRenderer::m_device;
	att_albedo.destroy(m_device.logicalDevice);
	att_position.destroy(m_device.logicalDevice);
	att_normal.destroy(m_device.logicalDevice);
	att_material.destroy(m_device.logicalDevice);
	att_depth.destroy(m_device.logicalDevice);
	vkDestroyFramebuffer(m_device.logicalDevice, framebuffer_GBuffer, nullptr);
	vkDestroyRenderPass(m_device.logicalDevice,renderpass_GBuffer, nullptr);
	vkDestroyPipeline(m_device.logicalDevice, pso_GBufferDefault, nullptr);
}

void GBufferRenderPass::SetupRenderpass()
{
	auto& m_device = VulkanRenderer::m_device;
	auto& m_swapchain = VulkanRenderer::m_swapchain;

	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	// TODO: Texture format optimization/packing?
	att_position.createAttachment(m_device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	att_normal  .createAttachment(m_device, width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	att_albedo  .createAttachment(m_device, width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	att_material.createAttachment(m_device, width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	att_depth   .createAttachment(m_device, width, height, VulkanRenderer::G_DEPTH_FORMAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

	// Set up separate renderpass with references to the color and depth attachments
	std::array<VkAttachmentDescription, GBufferAttachmentIndex::MAX_ATTACHMENTS> attachmentDescs = {};

	// Init attachment properties
	for (uint32_t i = 0; i < GBufferAttachmentIndex::MAX_ATTACHMENTS; ++i)
	{
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (i == GBufferAttachmentIndex::DEPTH)
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// Formats
	attachmentDescs[GBufferAttachmentIndex::POSITION].format = att_position.format;
	attachmentDescs[GBufferAttachmentIndex::NORMAL]  .format = att_normal.format;
	attachmentDescs[GBufferAttachmentIndex::ALBEDO]  .format = att_albedo.format;
	attachmentDescs[GBufferAttachmentIndex::MATERIAL].format = att_material.format;
	attachmentDescs[GBufferAttachmentIndex::DEPTH]   .format = att_depth.format;

	std::vector<VkAttachmentReference> colorReferences;
	colorReferences.push_back({ GBufferAttachmentIndex::POSITION, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::NORMAL,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::ALBEDO,   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ GBufferAttachmentIndex::MATERIAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = GBufferAttachmentIndex::DEPTH;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHK(vkCreateRenderPass(m_device.logicalDevice, &renderPassInfo, nullptr, &renderpass_GBuffer));
	VK_NAME(m_device.logicalDevice, "deferredPass", renderpass_GBuffer);
}

void GBufferRenderPass::SetupFramebuffer()
{
	std::array<VkImageView, GBufferAttachmentIndex::MAX_ATTACHMENTS> attachments;
	attachments[GBufferAttachmentIndex::POSITION] = att_position.view;
	attachments[GBufferAttachmentIndex::NORMAL]   = att_normal.view;
	attachments[GBufferAttachmentIndex::ALBEDO]   = att_albedo.view;
	attachments[GBufferAttachmentIndex::MATERIAL] = att_material.view;
	attachments[GBufferAttachmentIndex::DEPTH]    = VulkanRenderer::m_swapchain.depthAttachment.view;

	auto& m_device = VulkanRenderer::m_device;
	auto& m_swapchain = VulkanRenderer::m_swapchain;
	const uint32_t width = m_swapchain.swapChainExtent.width;
	const uint32_t height = m_swapchain.swapChainExtent.height;

	vk::Texture2D tex[4];
	tex[0].forFrameBuffer(VK_FORMAT_R16G16B16A16_SFLOAT, width, height, &m_device);
	tex[1].forFrameBuffer(VK_FORMAT_R16G16B16A16_SFLOAT, width, height, &m_device);
	tex[2].forFrameBuffer(VK_FORMAT_R8G8B8A8_UNORM, width, height, &m_device);
	tex[3].forFrameBuffer(VK_FORMAT_R8G8B8A8_UNORM, width, height, &m_device);

	//FramebufferCache g_cache;
	//g_cache.Init(VulkanRenderer::m_device.logicalDevice);
	//
	//VkFramebuffer fb;
	//FramebufferBuilder::Begin(&g_cache)
	//	.BindImage(tex[0])
	//	.BindImage(tex[1])
	//	.BindImage(tex[2])
	//	.BindImage(tex[3])
	//	.Build(fb,renderpass_GBuffer);

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = renderpass_GBuffer;
	fbufCreateInfo.pAttachments = attachments.data();
	fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbufCreateInfo.width = VulkanRenderer::m_swapchain.swapChainExtent.width;
	fbufCreateInfo.height = VulkanRenderer::m_swapchain.swapChainExtent.height;
	fbufCreateInfo.layers = 1;
	VK_CHK(vkCreateFramebuffer(VulkanRenderer::m_device.logicalDevice, &fbufCreateInfo, nullptr, &framebuffer_GBuffer));
	VK_NAME(VulkanRenderer::m_device.logicalDevice, "deferredFB", framebuffer_GBuffer);

	deferredImg[GBufferAttachmentIndex::POSITION] = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_position.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredImg[GBufferAttachmentIndex::NORMAL]   = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_normal.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredImg[GBufferAttachmentIndex::ALBEDO]   = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_albedo.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredImg[GBufferAttachmentIndex::MATERIAL] = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_material.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	//deferredImg[GBufferAttachmentIndex::DEPTH]    = ImGui_ImplVulkan_AddTexture(GfxSamplerManager::GetSampler_Deferred(), att_depth.view, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}


void GBufferRenderPass::CreatePipeline()
{
	auto& m_device = VulkanRenderer::m_device;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vk::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vk::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vk::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vk::inits::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vk::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vk::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vk::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vk::inits::pipelineCreateInfo(VulkanRenderer::indirectPipeLayout, VulkanRenderer::defaultRenderPass);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();


	// Vertex input state from glTF model for pipeline rendering models
	//how the data for a single vertex (including infos such as pos, colour, texture, coords, normals etc) is as a whole
	auto& bindingDescription = oGFX::GetGFXVertexInputBindings();

	//how the data for an attirbute is define in the vertex
	auto& attributeDescriptions = oGFX::GetGFXVertexInputAttributes();

	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	// -- VERTEX INPUT -- 
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = oGFX::vk::inits::pipelineVertexInputStateCreateInfo(bindingDescription,attributeDescriptions);
	//vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	//vertexInputCreateInfo.vertexAttributeDescriptionCount = 5;

	vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

	pipelineCI.pVertexInputState = &vertexInputCreateInfo;

	// Offscreen pipeline
	shaderStages[0] = VulkanRenderer::LoadShader(m_device, "Shaders/bin/gbuffer.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = VulkanRenderer::LoadShader(m_device, "Shaders/bin/gbuffer.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Separate render pass
	pipelineCI.renderPass = renderpass_GBuffer;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, GBufferAttachmentIndex::TOTAL_COLOR_ATTACHMENTS> blendAttachmentStates =
	{
		oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
		oGFX::vk::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
	};

	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_GBufferDefault));
	VK_NAME(m_device.logicalDevice, "GBufferPSO", pso_GBufferDefault);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);

}

