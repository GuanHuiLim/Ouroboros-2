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
        registration::class_<UITextComponent>("UI Text")
            .property("Text", &UITextComponent::text)
            .property("Color",&UITextComponent::color);
    }
}
