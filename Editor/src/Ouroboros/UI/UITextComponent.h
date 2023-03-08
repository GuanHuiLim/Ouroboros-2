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
namespace oGFX 
{
    class Font;
}
namespace oo
{
    class UITextComponent final
    {
    public:
        enum class FontAlignment : int
        {
            Top_Left        = 1 << 0,
            Top_Right       = 1 << 1,
            Top_Centre      = 1 << 2,
            Centre_Left     = 1 << 3,
            Centre          = 1 << 4,
            Centre_Right    = 1 << 5,
            Bottom_Left     = 1 << 6,
            Bottom_Right    = 1 << 7,
            Bottom_Centre   = 1 << 8,
        };

        std::string Text = "Default text here";
        Color TextColor = { 1, 1, 1, 1 };
        float FontSize = 8;
        float VerticalLineSpace = 1;
        FontAlignment Alignment = FontAlignment::Centre;

        Asset FontFamily;
        oGFX::Font* font = nullptr;

        Asset GetTextFont() const;
        void SetTextFont(Asset NewFontFamily);

        RTTR_ENABLE();
    };
}