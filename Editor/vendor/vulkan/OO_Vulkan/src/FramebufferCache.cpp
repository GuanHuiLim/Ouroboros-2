/************************************************************************************//*!
\file           FramebufferCache.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              defines a framebuffer cahcer 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "FramebufferCache.h"
#include  <algorithm>
#include <unordered_set>
#include "VulkanUtils.h"
#include "DelayedDeleter.h"
#include "VulkanRenderer.h"

void FramebufferCache::Init(VkDevice newDevice)
{
	device = newDevice;
}

void FramebufferCache::Cleanup()
{
	//delete every descriptor layout held
	for (auto pair : bufferCache){
		vkDestroyFramebuffer(device, pair.second, nullptr);
	}
}

VkFramebuffer FramebufferCache::CreateFramebuffer(VkFramebufferCreateInfo* info,
	std::vector<vkutils::Texture2D*>&& textures,
	bool swapchainTarget,
	bool resourceTrackOnly)
{
	FramebufferInfo bufferInfo;
	bufferInfo.createInfo = *info;
	bufferInfo.targetSwapchain = swapchainTarget;
	bufferInfo.resourceTrackOnly = resourceTrackOnly;

	// this is pretty bad, maybe std::move it here since its just stack based?
	bufferInfo.textures = std::move(textures);

	int lastBinding = -1;

	std::vector<VkImageView> attachment;
	attachment.reserve(bufferInfo.textures.size());
	for (uint32_t i = 0; i < bufferInfo.createInfo.attachmentCount; i++) {
		attachment.push_back(bufferInfo.textures[i]->view);
	}
	bufferInfo.createInfo.pAttachments = attachment.data();

	//try to grab from cache
	auto it = bufferCache.find(bufferInfo);
	if (it != bufferCache.end()){
		return (*it).second;
	}
	else {
		//create a new one (not found)
		std::cout << "[FBCache] Creating a new framebuffer.." << std::endl;
		VkFramebuffer frameBuffer{};
		if (bufferInfo.resourceTrackOnly == false)
		{
			VK_CHK(vkCreateFramebuffer(device, &bufferInfo.createInfo, nullptr, &frameBuffer));
			VK_NAME(device, "famebufferCache::framebuffer", frameBuffer);
		}
		//add to cache
		
		//store the pointers
		bufferInfo.createInfo.pAttachments = attachment.data();

		bufferCache[bufferInfo] = frameBuffer;
		return frameBuffer;
	}
}

void FramebufferCache::ResizeSwapchain(uint32_t width, uint32_t height)
{
	std::vector<FramebufferInfo> targets;
	for (auto& [framebufferInfo, framebuffer] : bufferCache){		
		if (framebufferInfo.targetSwapchain)
		{
			targets.emplace_back(framebufferInfo);			
		}		
	}

	// disgusting
	std::unordered_set<vkutils::Texture2D*> tex;
	for (auto& target: targets)
	{
		for (auto texture:target.textures)
		{
			tex.insert(texture);			
		}
	}
	for (auto texture: tex)
	{
		texture->Resize(width, height);
		auto oldLayout = texture->currentLayout;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) 
			continue;

		texture->currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		auto cmd = VulkanRenderer::get()->beginSingleTimeCommands();
		vkutils::TransitionImage(cmd, *texture, oldLayout);
			VulkanRenderer::get()->endSingleTimeCommands(cmd);
		vkQueueWaitIdle(VulkanRenderer::get()->m_device.graphicsQueue);
	}
	
	for (auto& target : targets)
	{
		auto kvp = bufferCache.extract(target);
		auto& framebufferInfo = kvp.key();

		framebufferInfo.createInfo.width = target.textures.front()->width;
		framebufferInfo.createInfo.height = target.textures.front()->height;
		bufferCache.insert(std::move(kvp));

		if (framebufferInfo.resourceTrackOnly == false)
		{
			std::vector<VkImageView> attachment;
			attachment.reserve(framebufferInfo.textures.size());
			for (uint32_t i = 0; i < framebufferInfo.createInfo.attachmentCount; i++) {
				attachment.push_back(framebufferInfo.textures[i]->view);
			}

			framebufferInfo.createInfo.pAttachments = attachment.data();
			auto& framebuffer = bufferCache[framebufferInfo];

			std::cout << "[FBCache] Resizing framebuffer.." << std::endl;
			
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			VK_CHK(vkCreateFramebuffer(device, &framebufferInfo.createInfo, nullptr, &framebuffer));
			VK_NAME(device, "famebufferCache::framebuffer", framebuffer);
		}

		//add to cache

		//store the pointers
	}
}

void FramebufferCache::DeleteRelated(vkutils::Texture tex)
{
	for (auto iter = bufferCache.begin(); iter != bufferCache.end();)
	{
		bool found = false;
		for (auto&t : iter->first.textures)
		{
			if (t->view == tex.view)
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			std::cout << "[FBCache] Removing fb "<<iter->second<<" hash "<< iter->first.hash() <<"\n";
			VkFramebuffer fb = iter->second;
			DelayedDeleter::get()->DeleteAfterFrames([=]() {				
				vkDestroyFramebuffer(VulkanRenderer::get()->m_device.logicalDevice, fb, nullptr);
				});
			iter = bufferCache.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
}

void FramebufferCache::RegisterFramebuffer(vkutils::Texture2D& tex)
{
	static uint32_t counter = std::numeric_limits<uint32_t>::max();
	--counter;
	FramebufferInfo fbi; //open up
	fbi.resourceTrackOnly = true;
	fbi.targetSwapchain = true;
	fbi.createInfo.attachmentCount = counter;// some information to hash
	fbi.createInfo.layers = counter/2;// some information to hash
	fbi.createInfo.height = counter /4;// some information to hash

	fbi.textures.emplace_back(&tex);
	bufferCache[fbi] = VK_NULL_HANDLE;
}

bool FramebufferCache::FramebufferInfo::operator==(const FramebufferInfo& other) const
{
	if (other.textures.size() != textures.size()){
		return false;
	}
	else {
		//compare each of the bindings is the same. Bindings are assumed to be sorted so they will match
		for (int i = 0; i < textures.size(); i++) {
			if (other.textures[i] != textures[i]){
				return false;
			}
		}
		return true;
	}
}

size_t FramebufferCache::FramebufferInfo::hash() const
{
	using std::size_t;
	using std::hash;

	size_t result = hash<size_t>()(textures.size());
	uint32_t count = 0;
	for (const auto& b : textures)
	{
		//pack the binding data into a single int64. Not fully correct but it's ok
		size_t binding_hash = reinterpret_cast<uint64_t>(b) | size_t(count) << 8 | textures.size() << 16 | static_cast<uint64_t>(b->format) << 24;

		++count;

		//shuffle the packed binding data and xor it with the main hash
		result ^= hash<size_t>()(binding_hash);
	}

	return result;

}
