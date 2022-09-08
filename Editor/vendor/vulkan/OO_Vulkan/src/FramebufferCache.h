#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <unordered_map>

class FramebufferCache {
public:
	void Init(VkDevice newDevice);
	void Cleanup();

	VkFramebuffer CreateFramebuffer(VkFramebufferCreateInfo* info);

	struct FramebufferInfo {
		//good idea to turn this into a inlined array
		std::vector<VkImageView> attachments;
		VkFramebufferCreateInfo createInfo{};

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
