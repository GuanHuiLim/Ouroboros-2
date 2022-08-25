#include "DeferredCompositionRenderpass.h"

#include "VulkanRenderer.h"
#include "Window.h"
#include "VulkanUtils.h"

#include "GBufferRenderPass.h"

#include <array>

DECLARE_RENDERPASS(DeferredCompositionRenderpass);


//struct test
//{
//	test()
//	{
//		auto ptr = new DeferredCompositionRenderpass;
//		RenderPassDatabase::Get()->RegisterRenderPass(ptr);
//		std::cout<< "KILL ME PLS" << std::endl;
//	}
//}t;

void DeferredCompositionRenderpass::Init()
{

}

void DeferredCompositionRenderpass::CreatePSO()
{
	CreateDescriptors();
	CreatePipeline(); // Dependency on GBuffer Init()
}

void DeferredCompositionRenderpass::Draw()
{
	auto& vr = *VulkanRenderer::get();
	auto swapchainIdx = vr.swapchainIdx;
	auto* windowPtr = vr.windowPtr;

    const VkCommandBuffer cmdlist = vr.commandBuffers[swapchainIdx];
    PROFILE_GPU_CONTEXT(cmdlist);
    PROFILE_GPU_EVENT("DeferredComposition");

	std::array<VkClearValue, 2> clearValues{};
	//clearValues[0].color = { 0.6f,0.65f,0.4f,1.0f };
	clearValues[0].color = { 0.1f,0.1f,0.1f,1.0f };
	clearValues[1].depthStencil.depth = { 1.0f };

	//Information about how to begin a render pass (only needed for graphical applications)
	VkRenderPassBeginInfo renderPassBeginInfo = oGFX::vkutils::inits::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = vr.renderPass_default;                  //render pass to begin
	renderPassBeginInfo.renderArea.offset = { 0,0 };                                     //start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = vr.m_swapchain.swapChainExtent; //size of region to run render pass on (Starting from offset)
	renderPassBeginInfo.pClearValues = clearValues.data();                               //list of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer =  vr.swapChainFramebuffers[swapchainIdx];

	vkCmdBeginRenderPass(cmdlist, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	SetDefaultViewportAndScissor(cmdlist);

	vkCmdBindPipeline(cmdlist, VK_PIPELINE_BIND_POINT_GRAPHICS, pso_DeferredLightingComposition);
	std::array<VkDescriptorSet, 2> descriptorSetGroup = 
	{
		vr.descriptorSets_uniform[swapchainIdx],
		vr.descriptorSet_bindless
	};

	vkCmdBindDescriptorSets(cmdlist, VK_PIPELINE_BIND_POINT_GRAPHICS, layout_DeferredLightingComposition, 0, 1, &vr.descriptorSet_DeferredComposition, 0, nullptr);

	DrawFullScreenQuad(cmdlist);

	vkCmdEndRenderPass(cmdlist);
}

void DeferredCompositionRenderpass::Shutdown()
{
	auto& vr = *VulkanRenderer::get();
	vkDestroyPipelineLayout(vr.m_device.logicalDevice, layout_DeferredLightingComposition, nullptr);
	//vkDestroyRenderPass(vr.m_device.logicalDevice,renderpass_DeferredLightingComposition, nullptr);
	vkDestroyPipeline(vr.m_device.logicalDevice, pso_DeferredLightingComposition, nullptr);
}

void DeferredCompositionRenderpass::CreateDescriptors()
{
	auto& vr = *VulkanRenderer::get();
	// At this point, all dependent resources (gbuffer etc) must be ready.
	auto gbuffer = RenderPassDatabase::GetRenderPass<GBufferRenderPass>();
	assert(gbuffer != nullptr);

    auto& m_device = vr.m_device;

    if (vr.descriptorSet_DeferredComposition)
        return;

    // TODO: Share this function?
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vr.m_device.physicalDevice, &props);
    size_t minUboAlignment = props.limits.minUniformBufferOffsetAlignment;
    if (minUboAlignment > 0)
    {
        uboDynamicAlignment = (uboDynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t numLights = 1;
    VkDeviceSize vpBufferSize = uboDynamicAlignment * numLights;

    //// LightData buffer size

    // Image descriptors for the offscreen color attachments
    VkDescriptorImageInfo texDescriptorPosition = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
		gbuffer->att_position.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorNormal = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
		gbuffer->att_normal.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorAlbedo = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
		gbuffer->att_albedo.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkDescriptorImageInfo texDescriptorMaterial = oGFX::vkutils::inits::descriptorImageInfo(
        GfxSamplerManager::GetSampler_Deferred(),
        gbuffer->att_material.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// TODO: Proper light buffer
	// TODO: How to handle shadow map sampling?
	std::cout << "Desc add:" << &VulkanRenderer::get()->DescLayoutCache << std::endl;

	DescriptorBuilder b = DescriptorBuilder::Begin(&VulkanRenderer::get()->DescLayoutCache,
		&VulkanRenderer::get()->DescAlloc);

	b.BindImage(1, &texDescriptorPosition, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    b.BindImage(2, &texDescriptorNormal, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    b.BindImage(3, &texDescriptorAlbedo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    b.BindImage(4, &texDescriptorMaterial, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    b.BindBuffer(5, &vr.lightsBuffer.descriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
    b.Build(vr.descriptorSet_DeferredComposition, vr.descriptorSetLayout_DeferredComposition);
}

void DeferredCompositionRenderpass::CreatePipeline()
{
	auto& vr = *VulkanRenderer::get();
	auto& m_device = vr.m_device;

	std::vector<VkDescriptorSetLayout> setLayouts{ vr.descriptorSetLayout_DeferredComposition };

	VkPipelineLayoutCreateInfo plci = oGFX::vkutils::inits::pipelineLayoutCreateInfo(setLayouts.data(),static_cast<uint32_t>(setLayouts.size()));	
	plci.pushConstantRangeCount = 1;
	plci.pPushConstantRanges = &vr.pushConstantRange;

	VK_CHK(vkCreatePipelineLayout(m_device.logicalDevice, &plci, nullptr, &layout_DeferredLightingComposition));
	VK_NAME(m_device.logicalDevice, "compositionPipeLayout", layout_DeferredLightingComposition);

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = oGFX::vkutils::inits::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = oGFX::vkutils::inits::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentState = oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = oGFX::vkutils::inits::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportState = oGFX::vkutils::inits::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleState = oGFX::vkutils::inits::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = oGFX::vkutils::inits::pipelineDynamicStateCreateInfo(dynamicStateEnables);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkGraphicsPipelineCreateInfo pipelineCI = oGFX::vkutils::inits::pipelineCreateInfo(vr.indirectPSOLayout, vr.renderPass_default);
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

	// Final fullscreen composition pass pipeline
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	shaderStages[0] = vr.LoadShader(m_device, "Shaders/bin/deferredlighting.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = vr.LoadShader(m_device, "Shaders/bin/deferredlighting.frag.spv",VK_SHADER_STAGE_FRAGMENT_BIT);

	// Empty vertex input state, vertices are generated by the vertex shader
	VkPipelineVertexInputStateCreateInfo emptyInputState = oGFX::vkutils::inits::pipelineVertexInputStateCreateInfo();
	pipelineCI.pVertexInputState = &emptyInputState;
	pipelineCI.renderPass = vr.renderPass_default;
	pipelineCI.layout = layout_DeferredLightingComposition;
	colorBlendState = oGFX::vkutils::inits::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
	blendAttachmentState= oGFX::vkutils::inits::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

	VK_CHK(vkCreateGraphicsPipelines(m_device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pso_DeferredLightingComposition));
	VK_NAME(m_device.logicalDevice, "compositionPipe", pso_DeferredLightingComposition);

	vkDestroyShaderModule(m_device.logicalDevice,shaderStages[0].module , nullptr);
	vkDestroyShaderModule(m_device.logicalDevice, shaderStages[1].module, nullptr);
}
