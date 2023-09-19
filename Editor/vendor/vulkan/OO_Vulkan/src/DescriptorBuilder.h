/************************************************************************************//*!
\file           DescriptorBuilder.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a descriptor builder class

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "vulkan/vulkan.h"
#include <vector>

class DescriptorLayoutCache;
class DescriptorAllocator;

class DescriptorBuilder
{
public:
	static DescriptorBuilder Begin();

	DescriptorBuilder& BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
	DescriptorBuilder& BindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count = 1);

	bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
	bool Build(VkDescriptorSet& set);
	bool BuildLayout(VkDescriptorSetLayout& layout);
private:

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	std::vector<VkDescriptorImageInfo> imageinfos;
	std::vector<VkDescriptorBufferInfo> bufferinfos;

	DescriptorLayoutCache* cache{ nullptr };
	DescriptorAllocator* alloc{nullptr};
};

