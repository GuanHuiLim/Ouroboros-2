#include "FramebufferBuilder.h"
#include "FramebufferCache.h"

#include "DescriptorAllocator.h"
#include "DescriptorLayoutCache.h"
#include "VulkanUtils.h"

FramebufferBuilder FramebufferBuilder::Begin(FramebufferCache* bufferCache)
{
	FramebufferBuilder builder;

	builder.cache = bufferCache;
	return builder;
}


FramebufferBuilder& FramebufferBuilder::BindImage(vkutils::Texture2D* tex)
{
	textures.push_back(tex);

	return *this;
}

bool FramebufferBuilder::Build(VkFramebuffer& framebuffer, VkRenderPass renderPass)
{
	uint32_t w, h;
	w = textures.front()->width;
	h = textures.front()->height;
	bool swapchainTarget = textures.front()->targetSwapchain;
	for (auto& tex : textures)
	{
		assert(swapchainTarget == tex->targetSwapchain && "Swapchain Target Unexpected!");
		assert(w == tex->width && h == tex->height && "Incompatible attachment sizes!");
	}

	VkFramebufferCreateInfo fbInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = uint32_t(textures.size());
	fbInfo.width = w;
	fbInfo.height = h;
	fbInfo.layers = 1;

	framebuffer = cache->CreateFramebuffer(&fbInfo, std::move(textures), textures.front()->targetSwapchain);

	return true;
}
