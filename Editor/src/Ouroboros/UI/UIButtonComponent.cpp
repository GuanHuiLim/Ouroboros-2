/************************************************************************************//*!
\file           UIButtonComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Nov 09, 2022
\brief          Defines a UIButton Component, which allows GameObjects with a RectTransform
                to become interactable. This is done by attaching actions to mouse events, like
                on enter, on click etc., which will be invoked when the corresponding mouse
                event is triggered

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "UIButtonComponent.h"
#include <rttr/registration.h>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<UIButtonComponent>("UI Button")
        .property("Is Interactable", &UIButtonComponent::IsInteractable)
        .property_readonly("Has Entered", &UIButtonComponent::HasEntered)
        .property_readonly("Is Pressed", &UIButtonComponent::IsPressed);
    }
}