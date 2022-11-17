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
#include "VulkanTexture.h"
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

VkFramebuffer FramebufferCache::CreateFramebuffer(VkFramebufferCreateInfo* info,std::vector<vkutils::Texture2D*>&& textures, bool swapchainTarget)
{
	FramebufferInfo bufferInfo;
	bufferInfo.createInfo = *info;
	bufferInfo.targetSwapchain = swapchainTarget;

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
		VkFramebuffer frameBuffer;
		VK_CHK(vkCreateFramebuffer(device, &bufferInfo.createInfo, nullptr, &frameBuffer));
		VK_NAME(device, "famebufferCache::framebuffer", frameBuffer);
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
	}
	
	for (auto& target : targets)
	{
		auto kvp = bufferCache.extract(target);
		auto& framebufferInfo = kvp.key();

		framebufferInfo.createInfo.width = target.textures.front()->width;
		framebufferInfo.createInfo.height = target.textures.front()->height;
		bufferCache.insert(std::move(kvp));

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
		//add to cache

		//store the pointers
	}
}

void FramebufferCache::DeleteRelated(vkutils::Texture2D tex)
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
