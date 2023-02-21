/************************************************************************************//*!
\file           UITextComponent.cpp
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
#include "pch.h"
#include "UITextComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration.h>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::enumeration<UITextComponent::FontAlignment>("Font Alignment")
        (
            value("Top Left", UITextComponent::FontAlignment::Top_Left),
            value("Top Centre", UITextComponent::FontAlignment::Top_Centre),
            value("Top Right", UITextComponent::FontAlignment::Top_Right),

            value("Middle Left", UITextComponent::FontAlignment::Centre_Left),
            value("Middle Centre", UITextComponent::FontAlignment::Centre),
            value("Middle Right", UITextComponent::FontAlignment::Centre_Right),

            value("Bottom Left", UITextComponent::FontAlignment::Bottom_Left),
            value("Bottom Centre", UITextComponent::FontAlignment::Bottom_Centre),
            value("Bottom Right", UITextComponent::FontAlignment::Bottom_Right)
        );

        registration::class_<UITextComponent>("UI Text")
            .property("Text", &UITextComponent::Text)
            .property("Color",&UITextComponent::TextColor)
            .property("Font Size", &UITextComponent::FontSize)
            .property("Font Alignment", &UITextComponent::Alignment)
            .property("Vertical Line Space", &UITextComponent::VerticalLineSpace);
    }
}
