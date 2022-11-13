/************************************************************************************//*!
\file           UIButtonComponent.h
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
#pragma once

#include <rttr/type>
#include <glm/glm.hpp>
#include "Utility/UUID.h"
#include "Ouroboros/EventSystem/Event.h"

namespace oo
{
    enum class UIButtonEventType
    {
        ON_POINTER_ENTER,
        ON_POINTER_EXIT,
        ON_PRESS,
        ON_CLICK,
        ON_RELEASE,
    };

    struct UIButtonEvent : public Event
    {
        UIButtonEventType Type;
        UUID buttonID;
    };

    class UIRaycastComponent final 
    {
    public:

        //bool IsInteractable = true;
        bool HasEntered = false;
        bool IsPressed = false;

        RTTR_ENABLE();
    };
}