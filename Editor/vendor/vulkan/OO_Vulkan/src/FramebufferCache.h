#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <unordered_map>

namespace vkutils
{
class Texture2D;
}

class FramebufferCache {
public:
	void Init(VkDevice newDevice);
	void Cleanup();

	VkFramebuffer CreateFramebuffer(VkFramebufferCreateInfo* info,std::vector<vkutils::Texture2D*>&& textures, bool swapchainTarget);
	void ResizeSwapchain(uint32_t width, uint32_t height);

	struct FramebufferInfo {
		//good idea to turn this into a inlined array
		// TODO : FIX horrific pointer stuff
		std::vector<vkutils::Texture2D*> textures;
		VkFramebufferCreateInfo createInfo{};
		bool targetSwapchain{ true };

		bool operator==(const FramebufferInfo& other) const;

		size_t hash() const;
	};



private:

	struct FramebufferHash		{

		std::size_t operator()(const FramebufferInfo& k) const{
			return k.hash();
		}
	};

	std::unordered_map<FramebufferInfo, VkFramebuffer, FramebufferHash> bufferCache;
	VkDevice device{};
};
