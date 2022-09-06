#include "FramebufferCache.h"
#include  <algorithm>

#include "VulkanUtils.h"

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

VkFramebuffer FramebufferCache::CreateFramebuffer(VkFramebufferCreateInfo* info)
{
	FramebufferInfo bufferInfo;
	bufferInfo.createInfo = *info;

	// this is pretty bad, maybe std::move it here since its just stack based?
	bufferInfo.attachments.reserve(info->attachmentCount);

	int lastBinding = -1;

	// signed unsigned changed care...
	//copy from the direct info struct into our own one
	for (uint32_t i = 0; i < info->attachmentCount; i++) {
		bufferInfo.attachments.push_back(info->pAttachments[i]);
	}

	//try to grab from cache
	auto it = bufferCache.find(bufferInfo);
	if (it != bufferCache.end()){
		return (*it).second;
	}
	else {
		//create a new one (not found)
		std::cout << "[FBCache] Creating a new framebuffer.." << std::endl;
		VkFramebuffer frameBuffer;
		VK_CHK(vkCreateFramebuffer(device, info, nullptr, &frameBuffer));
		VK_NAME(device, "famebufferCache::framebuffer", frameBuffer);
		//add to cache
		
		//store the pointers
		bufferInfo.createInfo.pAttachments = bufferInfo.attachments.data();

		bufferCache[bufferInfo] = frameBuffer;
		return frameBuffer;
	}
}

bool FramebufferCache::FramebufferInfo::operator==(const FramebufferInfo& other) const
{
	if (other.attachments.size() != attachments.size()){
		return false;
	}
	else {
		//compare each of the bindings is the same. Bindings are assumed to be sorted so they will match
		for (int i = 0; i < attachments.size(); i++) {
			if (other.attachments[i] != attachments[i]){
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

	size_t result = hash<size_t>()(attachments.size());
	uint32_t count = 0;
	for (const VkImageView& b : attachments)
	{
		//pack the binding data into a single int64. Not fully correct but it's ok
		size_t binding_hash = reinterpret_cast<uint64_t>(b) | size_t(count) << 8 | attachments.size() << 16 | 0 << 24;

		++count;

		//shuffle the packed binding data and xor it with the main hash
		result ^= hash<size_t>()(binding_hash);
	}

	return result;

}
