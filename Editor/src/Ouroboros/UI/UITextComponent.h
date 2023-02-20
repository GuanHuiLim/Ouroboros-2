/************************************************************************************//*!
\file           UITextComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Feb 20, 2023
\brief          Defines a UIText Component, which allows GameObjects with a RectTransform
                to draw an image to its attached parent UICanvas

Copyright (C) 2023 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "Ouroboros/Vulkan/Color.h"
#include "Ouroboros/Asset/Asset.h"
#include "Ouroboros/Asset/AssetManager.h"
#include <rttr/type>

#include "Ouroboros/Vulkan/Color.h"

namespace oo
{
    class UITextComponent final
    {
    public:
        std::string text = "Default text here";
        Color color;

        RTTR_ENABLE();
    };
}