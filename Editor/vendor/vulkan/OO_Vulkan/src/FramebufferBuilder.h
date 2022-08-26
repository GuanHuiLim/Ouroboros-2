#pragma once
#include "vulkan/vulkan.h"
#include "VulkanTexture.h"
#include <vector>

class FramebufferCache;

class FramebufferBuilder {
public:
	static FramebufferBuilder Begin(FramebufferCache* bufferCache);

	FramebufferBuilder& BindImage(vkutils::Texture2D& tex);

	bool Build(VkFramebuffer& framebuffer, VkRenderPass renderPass);
private:

	std::vector<vkutils::Texture2D> textures;

	FramebufferCache* cache;
};

