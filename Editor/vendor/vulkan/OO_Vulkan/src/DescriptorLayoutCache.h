/************************************************************************************//*!
\file           DescriptorLayoutCache.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a descriptor layout cache for vulkan

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#include <unordered_map>
#include <mutex>

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
	std::mutex m_mut;
};
