/************************************************************************************//*!
\file           FramebufferBuilder.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a framebuffer builder

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "FramebufferBuilder.h"
#include "FramebufferCache.h"

#include "DescriptorAllocator.h"
#include "DescriptorLayoutCache.h"
#include "VulkanUtils.h"
#include "VulkanRenderer.h"

FramebufferBuilder FramebufferBuilder::Begin(FramebufferCache* bufferCache)
{
	FramebufferBuilder builder;

	builder.cache = bufferCache;
	return builder;
}


FramebufferBuilder& FramebufferBuilder::BindImage(vkutils::Texture2D* tex)
{
	textures.push_back(tex);

	return *this;
}

bool FramebufferBuilder::Build(VkFramebuffer& framebuffer, const VulkanRenderpass& renderPass)
{
	uint32_t w, h;
	w = textures.front()->width;
	h = textures.front()->height;
	bool swapchainTarget = textures.front()->targetSwapchain;
	for (size_t i = 0; i < renderPass.rpci.attachmentCount; i++)
	{
		auto& tex = textures[i];
		const auto& attachmentDes = renderPass.rpci.pAttachments[i];
		assert(swapchainTarget == tex->targetSwapchain && "Swapchain Target Unexpected!");
		assert(w == tex->width && h == tex->height && "Incompatible attachment sizes!");

		//verify resource
		if (tex->currentLayout != attachmentDes.initialLayout && attachmentDes.initialLayout != VK_IMAGE_LAYOUT_UNDEFINED)
		{
			std::cout << "Hey unexpected layout for renderpass "<<renderPass.name << "attachment=" <<i <<std::endl;
			std::cout << "\t expected "<< oGFX::vkutils::tools::VkImageLayoutString(attachmentDes.initialLayout) 
				<<" current "<<oGFX::vkutils::tools::VkImageLayoutString(tex->currentLayout) << std::endl;
		}
		tex->currentLayout = renderPass.rpci.pAttachments[i].finalLayout;
	}

	VkFramebufferCreateInfo fbInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	fbInfo.renderPass = renderPass.pass;
	fbInfo.attachmentCount = uint32_t(textures.size());
	fbInfo.width = w;
	fbInfo.height = h;
	fbInfo.layers = 1;

	framebuffer = cache->CreateFramebuffer(&fbInfo, std::move(textures), textures.front()->targetSwapchain);

	return true;
}
