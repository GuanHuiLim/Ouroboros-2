/************************************************************************************//*!
\file           UICanvasComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Nov 09, 2022
\brief          Defines a UICanvas Component, which marks GameObjects with a RectTransform
                as the controller for all UI elements parented under it

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "UICanvasComponent.h"

#include <rttr/registration>
#include "App/Editor/Properties/UI_metadata.h"

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::enumeration<UICanvasComponent::RenderMode>("Render Mode")
        (
            value("Overlay", UICanvasComponent::RenderMode::Overlay),
            value("World Space", UICanvasComponent::RenderMode::WorldSpace)
            //value("Canvas Space", UICanvasComponent::RenderMode::CanvasSpace)
        );

        registration::class_<UICanvasComponent>("UI Canvas")
            .property("Render Mode", &UICanvasComponent::RenderingMode)
            //.property("Layer", &UICanvasComponent::Layer)(metadata(UI_metadata::DRAG_SPEED, 0.01f))
            .property("Scale With Screen", &UICanvasComponent::ScaleWithScreenSize)
            .property("Render On Top", &UICanvasComponent::RenderOnTop)
            ;
    }

}
