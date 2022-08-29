#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <unordered_map>

class DescriptorLayoutCache {
public:
	void Init(VkDevice newDevice);
	void Cleanup();

	VkDescriptorSetLayout CreateDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info);

	struct DescriptorLayoutInfo {
		//good idea to turn this into a inlined array
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		bool operator==(const DescriptorLayoutInfo& other) const;

		size_t hash() const;
	};



private:

	struct DescriptorLayoutHash		{

		std::size_t operator()(const DescriptorLayoutInfo& k) const{
			return k.hash();
		}
	};

	std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layoutCache;
	VkDevice device{};
};
