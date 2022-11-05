/************************************************************************************//*!
\file           GameObjectDisabledComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Aug 23, 2022
\brief          All Components holding onto this component has been disabled.
                See GameObject SetActive for more information.
                This Component will be removed only be removed or added when you change
                the active state of the gameobject component.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "pch.h"
#include "GameObjectDisabledComponent.h"
#include <rttr/registration>

namespace oo
{
    // FOR DEBUGGING PURPOSES.
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<GameObjectDisabledComponent>("This GameObject is Disabled in the hierarchy");
    }
}
