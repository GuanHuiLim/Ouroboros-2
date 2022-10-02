/************************************************************************************//*!
\file           DescriptorAllocator.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a descriptor allocator

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "DescriptorAllocator.h"
#include "VulkanUtils.h"

void DescriptorAllocator::ResetPools()
{
	for (auto p : usedPools){
		vkResetDescriptorPool(device, p, 0);
		freePools.push_back(p);
	}

	//clear the used pools, since we've put them all in the free pools
	usedPools.clear();

	//reset the current pool handle back to null
	currentPool = VK_NULL_HANDLE;
}

bool DescriptorAllocator::Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
{
	//initialize the currentPool handle if it's null
	if (currentPool == VK_NULL_HANDLE){

		currentPool = GrabPool();
		usedPools.push_back(currentPool);
	}

	VkDescriptorSetAllocateInfo allocInfo = oGFX::vkutils::inits::descriptorSetAllocateInfo(currentPool,&layout,1);

	//try to allocate the descriptor set
	VkResult allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
	bool needReallocate = false;

	switch (allocResult) {
	case VK_SUCCESS:
	//all good, return
	return true;
	case VK_ERROR_FRAGMENTED_POOL:
	case VK_ERROR_OUT_OF_POOL_MEMORY:
	//reallocate pool
	needReallocate = true;
	break;
	default:
	//unrecoverable error
	return false;
	}

	if (needReallocate){
		//allocate a new pool and retry
		currentPool = GrabPool();
		usedPools.push_back(currentPool);

		allocInfo.descriptorPool = currentPool;
		allocResult = vkAllocateDescriptorSets(device, &allocInfo, set);
		std::cout << "Descriptor set allocated: " << std::endl;;

		//if it still fails then we have big issues
		VK_CHK(allocResult);
		if (allocResult == VK_SUCCESS){
			return true;
		}
	}

	return false;
}

void DescriptorAllocator::Init(VkDevice newDevice)
{
	device = newDevice;
}

void DescriptorAllocator::Cleanup()
{
	//delete every pool held
	for (auto p : freePools)
	{
		vkDestroyDescriptorPool(device, p, nullptr);
	}
	for (auto p : usedPools)
	{
		vkDestroyDescriptorPool(device, p, nullptr);
	}
}

VkDescriptorPool DescriptorAllocator::GrabPool()
{
	//there are reusable pools availible
	if (freePools.size() > 0)
	{
		//grab pool from the back of the vector and remove it from there.
		VkDescriptorPool pool = freePools.back();
		freePools.pop_back();
		return pool;
	}
	else
	{
		//no pools availible, so create a new one
		return CreatePool(device, descriptorSizes, 1000, 0);
	}
}

VkDescriptorPool DescriptorAllocator::CreatePool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
{	
	std::vector<VkDescriptorPoolSize> sizes;
	sizes.reserve(poolSizes.sizes.size());
	for (auto sz : poolSizes.sizes) {
		sizes.push_back({ sz.first, uint32_t(sz.second * count) });
	}

	VkDescriptorPoolCreateInfo pool_info = oGFX::vkutils::inits::descriptorPoolCreateInfo(sizes,count);
	pool_info.flags = flags;

	VkDescriptorPool descriptorPool;
	VK_CHK(vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool));
	VK_NAME(device, "Allocator::descriptorPool", descriptorPool);

	return descriptorPool;
}
