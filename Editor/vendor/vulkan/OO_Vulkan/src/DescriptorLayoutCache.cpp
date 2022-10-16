/************************************************************************************//*!
\file           DescriptorLayoutCache.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a descriptor layout cache for vulkan

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "DescriptorLayoutCache.h"
#include  <algorithm>

#include "VulkanUtils.h"

void DescriptorLayoutCache::Init(VkDevice newDevice)
{
	device = newDevice;
}

void DescriptorLayoutCache::Cleanup()
{
	//delete every descriptor layout held
	for (auto pair : layoutCache)
	{
		vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
	}
}

VkDescriptorSetLayout DescriptorLayoutCache::CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info)
{
	DescriptorLayoutInfo layoutinfo;
	layoutinfo.bindings.reserve(info->bindingCount);
	bool isSorted = true;
	int lastBinding = -1;

	//copy from the direct info struct into our own one
	for (uint32_t i = 0; i < info->bindingCount; i++)
	{
		layoutinfo.bindings.push_back(info->pBindings[i]);

		//check that the bindings are in strict increasing order
		if (int32_t(info->pBindings[i].binding) > lastBinding){
			lastBinding = info->pBindings[i].binding;
		}
		else
		{
			isSorted = false;
		}
	}
	//sort the bindings if they aren't in order
	if (!isSorted)
	{
		std::sort(layoutinfo.bindings.begin(), layoutinfo.bindings.end(), [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b ){
			return a.binding < b.binding;
			});
	}

	//try to grab from cache
	auto it = layoutCache.find(layoutinfo);
	if (it != layoutCache.end()){
		return (*it).second;
	}
	else
	{
		//create a new one (not found)
		VkDescriptorSetLayout layout;
		VK_CHK(vkCreateDescriptorSetLayout(device, info, nullptr, &layout));
		VK_NAME(device, "LayoutCache::layout", layout);
		//add to cache
		layoutCache[layoutinfo] = layout;
		return layout;
	}
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
{
	if (other.bindings.size() != bindings.size())
	{
		return false;
	}
	else 
	{
		//compare each of the bindings is the same. Bindings are assumed to be sorted so they will match
		for (int i = 0; i < bindings.size(); i++) {
			if (other.bindings[i].binding != bindings[i].binding){
				return false;
			}
			if (other.bindings[i].descriptorType != bindings[i].descriptorType){
				return false;
			}
			if (other.bindings[i].descriptorCount != bindings[i].descriptorCount){
				return false;
			}
			if (other.bindings[i].stageFlags != bindings[i].stageFlags){
				return false;
			}
		}
		return true;
	}
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
{
	using std::size_t;
	using std::hash;

	size_t result = hash<size_t>()(bindings.size());

	for (const VkDescriptorSetLayoutBinding& b : bindings)
	{
		//pack the binding data into a single int64. Not fully correct but it's ok
		size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

		//shuffle the packed binding data and xor it with the main hash
		result ^= hash<size_t>()(binding_hash);
	}

	return result;

}
