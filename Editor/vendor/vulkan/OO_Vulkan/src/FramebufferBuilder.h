/************************************************************************************//*!
\file           FramebufferBuilder.h
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
#pragma once
#include "vulkan/vulkan.h"
#include "VulkanTexture.h"
#include "VulkanRenderpass.h"
#include <vector>

class FramebufferCache;

class FramebufferBuilder {
public:
	static FramebufferBuilder Begin(FramebufferCache* bufferCache);

	FramebufferBuilder& BindImage(vkutils::Texture2D* tex);

	bool Build(VkFramebuffer& framebuffer, const VulkanRenderpass& renderPass);
private:

	std::vector<vkutils::Texture2D*> textures;

	FramebufferCache* cache{ nullptr };
};

