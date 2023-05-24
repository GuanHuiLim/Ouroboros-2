/************************************************************************************//*!
\file           FramebufferCache.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a framebuffer cahcer

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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

	VkFramebuffer CreateFramebuffer(VkFramebufferCreateInfo* info,
		std::vector<vkutils::Texture2D*>&& textures,
		bool swapchainTarget
		bool resourceTrackOnly = false);
	void ResizeSwapchain(uint32_t width, uint32_t height);

	struct FramebufferInfo {
		//good idea to turn this into a inlined array
		// TODO : FIX horrific pointer stuff
		std::vector<vkutils::Texture2D*> textures;
		VkFramebufferCreateInfo createInfo{};
		bool targetSwapchain{ true };
		bool resourceTrackOnly{ false };

		bool operator==(const FramebufferInfo& other) const;

		size_t hash() const;
	};
	void DeleteRelated(vkutils::Texture2D tex);


private:

	struct FramebufferHash		{

		std::size_t operator()(const FramebufferInfo& k) const{
			return k.hash();
		}
	};

	std::unordered_map<FramebufferInfo, VkFramebuffer, FramebufferHash> bufferCache;
	VkDevice device{};
};
