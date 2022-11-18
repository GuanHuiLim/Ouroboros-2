/************************************************************************************//*!
\file           VulkanTexture.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              A texture wrapper object for vulkan texture

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>
#include <vector>

#include "vulkan/vulkan.h"

#include "VulkanDevice.h"

	class VulkanRenderpass
	{
	public:
		std::string name{}; // maybe remove when not debug?
		VulkanDevice* device{ nullptr };
		VkRenderPass pass{VK_NULL_HANDLE};
		VkRenderPassCreateInfo rpci{};

		struct SubpassAttachments
		{
		std::vector<VkAttachmentReference> colorReferences;
		VkAttachmentReference depthReference;
		};

		std::vector<VkSubpassDescription> subpassDesc;
		std::vector<SubpassAttachments> subpassAtt;

		std::vector<VkAttachmentDescription> attachmentDesc;
		std::vector<VkSubpassDependency> subpassDepn;

		void Init(VulkanDevice& dev, VkRenderPassCreateInfo& createinfo);
		void destroy();
		
	};

