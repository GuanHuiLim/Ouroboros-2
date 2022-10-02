#pragma once
#include "vulkan/vulkan.h"
#include <vector>

class DescriptorLayoutCache;
class DescriptorAllocator;

class DescriptorBuilder
{
public:
	static DescriptorBuilder Begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator );

	DescriptorBuilder& BindBuffer(uint32_t binding, const VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
	DescriptorBuilder& BindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

	bool Build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
	bool Build(VkDescriptorSet& set);
private:

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	DescriptorLayoutCache* cache{ nullptr };
	DescriptorAllocator* alloc{nullptr};
};

