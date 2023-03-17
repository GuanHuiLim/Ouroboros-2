/************************************************************************************//*!
\file           GfxSampler.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a sampler managing system

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "GfxSampler.h"

#include "VulkanUtils.h"
#include "VulkanRenderer.h"

VkSampler GfxSamplerManager::textureSampler = nullptr;
VkSampler GfxSamplerManager::deferredSampler = nullptr;
VkSampler GfxSamplerManager::shadowSampler = nullptr;
VkSampler GfxSamplerManager::edgeClampSampler = nullptr;
VkSampler GfxSamplerManager::blackBorderSampler = nullptr;
// TODO: Add more sampler objects as needed...

void GfxSamplerManager::Init()
{
    auto& vr = *VulkanRenderer::get();
    auto& device = vr.m_device.logicalDevice;
    float maxAni = vr.m_device.properties.limits.maxSamplerAnisotropy;
    VkBool32 aniEnabled = vr.m_device.enabledFeatures.samplerAnisotropy;

    {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;							// how to render when image is magnified on screen
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;							// how to render when image is minified on the screen
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;		// how to handle texture wrap in U direction
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;		// how to handle texture wrap in V direction
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;		// how to handle texture wrap in W direction
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;		// border beyond texture ( only works for border clamp )
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;					// Whether coords should be normalized (between 0 and 1)
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;			// Mipmap interpolation mode
        samplerCreateInfo.mipLodBias = 0.0f;									// Level of details bias for mip level
        samplerCreateInfo.minLod = 0.0f;										// minimum level of detail to pick mip level
        samplerCreateInfo.maxLod = 0.0f;										// maximum level of detail to pick mip level
        samplerCreateInfo.anisotropyEnable = aniEnabled;							// Enable anisotropy
        samplerCreateInfo.maxAnisotropy = maxAni;									// Anisotropy sample level

        VkResult result = vkCreateSampler(device, &samplerCreateInfo, nullptr, &textureSampler);
        VK_NAME(device, "textureSampler", textureSampler);
        if (result != VK_SUCCESS)
        {
            std::cerr << "Failed to create a texture sampler!" << std::endl;
            throw std::runtime_error("Failed to create a texture sampler!");
        }
    }

    {
        VkSamplerCreateInfo samplerCreateInfo = oGFX::vkutils::inits::samplerCreateInfo();
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = maxAni;
        samplerCreateInfo.anisotropyEnable = aniEnabled;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &deferredSampler));
        VK_NAME(device, "deferredSampler", deferredSampler);
    }

    {
        VkSamplerCreateInfo samplerCreateInfo = oGFX::vkutils::inits::samplerCreateInfo();
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = maxAni;
        samplerCreateInfo.anisotropyEnable = aniEnabled;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &shadowSampler));
        VK_NAME(device, "shadowSampler", shadowSampler);
    }

    {
        VkSamplerCreateInfo samplerCreateInfo = oGFX::vkutils::inits::samplerCreateInfo();
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = maxAni;
        samplerCreateInfo.anisotropyEnable = aniEnabled;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &edgeClampSampler));
        VK_NAME(device, "ssaoSampler", edgeClampSampler);
    }


    {
        VkSamplerCreateInfo samplerCreateInfo = oGFX::vkutils::inits::samplerCreateInfo();
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = maxAni;
        samplerCreateInfo.anisotropyEnable = aniEnabled;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        VK_CHK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &blackBorderSampler));
        VK_NAME(device, "blackBorderSampler", blackBorderSampler);
    }
}

void GfxSamplerManager::Shutdown()
{
    auto& vr = *VulkanRenderer::get();
    auto& device = vr.m_device.logicalDevice;

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroySampler(device, deferredSampler, nullptr);
    vkDestroySampler(device, shadowSampler, nullptr);
    vkDestroySampler(device, edgeClampSampler, nullptr);
    vkDestroySampler(device, blackBorderSampler, nullptr);

    // TODO: Add more sampler objects as needed...
}
