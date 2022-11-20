/************************************************************************************//*!
\file           VulkanRenderpass.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines the texture wrapper object

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "VulkanRenderpass.h"

#include <cassert>


void VulkanRenderpass::Init(VulkanDevice& dev,VkRenderPassCreateInfo& ci) 
{
	device = &dev;

	// copy over data
	attachmentDesc.resize(ci.attachmentCount);
	for (size_t i = 0; i < ci.attachmentCount; i++)
	{
		attachmentDesc[i] = ci.pAttachments[i];
	}

	subpassAtt.resize(ci.subpassCount);
	subpassDesc.resize(ci.subpassCount);
	for (size_t i = 0; i < ci.subpassCount; i++)
	{
		auto& subpassAttachments = subpassAtt[i];
		auto& subpassDescription = subpassDesc[i];
		auto& rhs = ci.pSubpasses[i];
		subpassDescription = rhs; // copy everything
		subpassAttachments.colorReferences.resize(rhs.colorAttachmentCount);
		for (size_t cols = 0; cols < rhs.colorAttachmentCount; cols++)
		{
			subpassAttachments.colorReferences[cols] = rhs.pColorAttachments[cols];
		}

		if (rhs.pDepthStencilAttachment)
		{
			subpassAttachments.depthReference = *rhs.pDepthStencilAttachment;
			subpassDescription.pDepthStencilAttachment = &subpassAttachments.depthReference;
		}
		
		//link up to internal pointers		
		subpassDescription.pColorAttachments = subpassAttachments.colorReferences.data();
	}

	subpassDepn.resize(ci.dependencyCount);
	for (size_t i = 0; i < ci.dependencyCount; i++)
	{
		subpassDepn[i] = ci.pDependencies[i];
	}

	//setup structures
	rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rpci.attachmentCount = static_cast<uint32_t>(attachmentDesc.size());
	rpci.pAttachments = attachmentDesc.data();
	rpci.subpassCount = static_cast<uint32_t>(subpassDesc.size());
	rpci.pSubpasses = subpassDesc.data();
	rpci.dependencyCount = static_cast<uint32_t>(subpassDepn.size());
	rpci.pDependencies = subpassDepn.data();

	VK_CHK(vkCreateRenderPass(device->logicalDevice, &rpci, nullptr, &pass));
	VK_NAME(device->logicalDevice, name.empty()?"VULKAN_RENDERPASS" : name.c_str(), pass);
}

void VulkanRenderpass::destroy()
{
	vkDestroyRenderPass(device->logicalDevice, pass, nullptr);
	pass = VK_NULL_HANDLE;
}
