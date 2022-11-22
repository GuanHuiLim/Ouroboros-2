/************************************************************************************//*!
\file           UIImageComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Nov 09, 2022
\brief          Defines a UIImage Component, which allows GameObjects with a RectTransform
                to draw an image to its attached parent UICanvas

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/Vulkan/Color.h"
#include "Ouroboros/Asset/Asset.h"
#include "Ouroboros/Asset/AssetManager.h"
#include <rttr/type>
namespace oo
{
    class UIImageComponent final
    {
    public:
        Asset TextureHandle;
        //std::shared_ptr<Texture> m_texture;
        Color Tint = { 1.0f, 1.0f, 1.0f, 1.0f };
        //bool RaycastTarget = true;

        RTTR_ENABLE();
    };
}