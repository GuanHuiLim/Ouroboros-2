/************************************************************************************//*!
\file           UIImageComponent.cpp
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
#include "pch.h"
#include "UIImageComponent.h"
#include "App/Editor/Properties/UI_metadata.h"
#include <rttr/registration.h>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<UIImageComponent>("UI Image")
        .property("Texture", &UIImageComponent::TextureHandle)(metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Texture)))
        .property("Colour Tint", &UIImageComponent::Tint)
        //.property("Raycast Target", &UIImageComponent::RaycastTarget)
        ;
    }
}
