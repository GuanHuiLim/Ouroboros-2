/************************************************************************************//*!
\file           DescriptorBuilder.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief               Declares a descriptor builder class

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "DescriptorBuilder.h"

#include "DescriptorAllocator.h"
#include "DescriptorLayoutCache.h"
#include "VulkanUtils.h"

DescriptorBuilder DescriptorBuilder::Begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator)
{
	DescriptorBuilder builder;

	builder.cache = layoutCache;
	builder.alloc = allocator;
	return builder;
}

DescriptorBuilder& DescriptorBuilder::BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
	//create the descriptor binding for the layout
	VkDescriptorSetLayoutBinding newBinding = oGFX::vkutils::inits::descriptorSetLayoutBinding(type,stageFlags,binding,1);
	bindings.push_back(newBinding);

	//create the descriptor write
	VkWriteDescriptorSet newWrite = oGFX::vkutils::inits::writeDescriptorSet(VK_NULL_HANDLE,type,binding,bufferInfo,1);
	
	writes.push_back(newWrite);
	return *this;
}

DescriptorBuilder& DescriptorBuilder::BindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count)
{
	//create the descriptor binding for the layout
	VkDescriptorSetLayoutBinding newBinding = oGFX::vkutils::inits::descriptorSetLayoutBinding(type, stageFlags, binding, count);
	bindings.push_back(newBinding);

	//create the descriptor write
	VkWriteDescriptorSet newWrite = oGFX::vkutils::inits::writeDescriptorSet(VK_NULL_HANDLE,type,binding,imageInfo,count);

	writes.push_back(newWrite);
	return *this;
}


bool DescriptorBuilder::Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
{
	BuildLayout(layout);

	//allocate descriptor
	bool success = alloc->Allocate(&set, layout);
	if (!success)
	{
		return false;
	};

	//write descriptor
	for (VkWriteDescriptorSet& w : writes)
	{
		w.dstSet = set;
	}

	vkUpdateDescriptorSets(alloc->device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	return true;
}


bool DescriptorBuilder::Build(VkDescriptorSet& set)
{
	VkDescriptorSetLayout l;
	return Build(set,l);
}

bool DescriptorBuilder::BuildLayout(VkDescriptorSetLayout& layout)
{
	//build layout first
	VkDescriptorSetLayoutCreateInfo layoutInfo = oGFX::vkutils::inits::descriptorSetLayoutCreateInfo(bindings.data(), static_cast<uint32_t>(bindings.size()));

	layout = cache->CreateDescriptorLayout(&layoutInfo);

	return true;
}
