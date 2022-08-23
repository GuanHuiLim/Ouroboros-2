#include "GfxSampler.h"

#include "VulkanUtils.h"
#include "VulkanRenderer.h"

VkSampler GfxSamplerManager::textureSampler = nullptr;
VkSampler GfxSamplerManager::deferredSampler = nullptr;
// TODO: Add more sampler objects as needed...

void GfxSamplerManager::Init()
{
    auto& device = VulkanRenderer::m_device.logicalDevice;

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
        samplerCreateInfo.anisotropyEnable = VK_TRUE;							// Enable anisotropy
        samplerCreateInfo.maxAnisotropy = 16;									// Anisotropy sample level

        VkResult result = vkCreateSampler(device, &samplerCreateInfo, nullptr, &textureSampler);
        VK_NAME(device, "textureSampler", textureSampler);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create a texture sampler!");
        }
    }

    {
        VkSamplerCreateInfo samplerCreateInfo = oGFX::vk::inits::samplerCreateInfo();
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &deferredSampler));
        VK_NAME(device, "deferredSampler", deferredSampler);
    }
}

void GfxSamplerManager::Shutdown()
{
    auto& device = VulkanRenderer::m_device.logicalDevice;

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroySampler(device, deferredSampler, nullptr);
    // TODO: Add more sampler objects as needed...
}
