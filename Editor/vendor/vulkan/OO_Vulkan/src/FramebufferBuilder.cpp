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


FramebufferBuilder& FramebufferBuilder::BindImage(vk::Texture2D& tex)
{
	textures.push_back(tex);

	return *this;
}

bool FramebufferBuilder::Build(VkFramebuffer& framebuffer, VkRenderPass renderPass)
{
	std::vector<VkImageView> attachments;
	attachments.reserve(textures.size());
	uint32_t w, h;
	w = textures.front().width;
	h = textures.front().height;
	for (auto& tex : textures)
	{
		assert(w == tex.width && h == tex.height); // incompatible attachment sizes!
		attachments.push_back(tex.view);
	}

	VkFramebufferCreateInfo fbInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	fbInfo.renderPass = renderPass;
	fbInfo.attachmentCount = attachments.size();
	fbInfo.pAttachments = attachments.data();
	fbInfo.width = w;
	fbInfo.height = h;
	fbInfo.layers = 1;

	framebuffer = cache->CreateFramebuffer(&fbInfo);

	return true;
}
