/************************************************************************************//*!
\file           GfxSampler.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a sampler managing system

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <vulkan/vulkan.h>

class GfxSamplerManager
{
public:

    void Init();
    void Shutdown();

    static const VkSampler GetDefaultSampler() { return textureSampler; }
    static const VkSampler GetSampler_Deferred() { return deferredSampler; }
    static const VkSampler GetSampler_ShowMapClamp() { return shadowSampler; }
    static const VkSampler GetSampler_SSAOEdgeClamp() { return ssaoClampSampler; }
    static const VkSampler GetSampler_EdgeClamp() { return linearClampSampler; }
    static const VkSampler GetSampler_LinearClamp() { return linearClampSampler; }
    static const VkSampler GetSampler_PointClamp() { return pointClampSampler; }
    static const VkSampler GetSampler_BlackBorder() { return blackBorderSampler; }
    static const VkSampler GetSampler_Cube() { return cubeSampler; }
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
    static VkSampler shadowSampler;
    static VkSampler ssaoClampSampler;
    static VkSampler linearClampSampler;
    static VkSampler pointClampSampler;
    static VkSampler blackBorderSampler;
    static VkSampler cubeSampler;
    // TODO: Add more sampler objects as needed...
};
