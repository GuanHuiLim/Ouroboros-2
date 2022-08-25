/************************************************************************************//*!
\file           GameObjectComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes component holding basic information that should be accessible
                by all and all gameobjects should have one of these components by default.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>
#include "Utility/UUID.h"
#include <scenegraph/include/scenenode.h>

#include <rttr/type>
#include <Ouroboros/EventSystem/Event.h>
#include <Archetypes_Ecs/src/A_Ecs.h>

namespace oo
{
    class GameObjectComponent
    {
    public:
        bool Active = true;
        bool ActiveInHierarchy = true;
        UUID Id;
        std::string Name = "Default Name Long enough for no short string optimization";
        scenenode::weak_pointer Node = {};

        struct OnEnableEvent : public Event
        {
        };

        struct OnDisableEvent : public Event
        {
        };

        RTTR_ENABLE();
    };
}