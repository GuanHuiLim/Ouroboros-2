#pragma once

#include <vulkan/vulkan.h>

class GfxSamplerManager
{
public:

    void Init();
    void Shutdown();

    static const VkSampler GetDefaultSampler() { return textureSampler; }
    static const VkSampler GetSampler_Deferred() { return deferredSampler; }
    // TODO: Add more sampler objects as needed...

    // List of some default sampler types to consider:
    // PointWrap
    // PointClamp
    // PointBorder
    // BilinearWrap
    // BilinearClamp
    // BilinearBorder
    // TrilinearWrap
    // TrilinearClamp
    // TrilinearBorder
    // DefaultSampler
    // ShadowSampler

private:

    static VkSampler textureSampler;
    static VkSampler deferredSampler;
    // TODO: Add more sampler objects as needed...
};
