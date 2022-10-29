/************************************************************************************//*!
\file          PhysicsEvents.h
\project       Ouroboros
\author        Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par           email: c.tecklee\@digipen.edu
\date          Oct 21, 2022
\brief         Describes all physics events that other systems might be interested 
               to hook up to.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/EventSystem/Event.h"
#include "Utility/UUID.h"

namespace oo
{
    //physics tick event.
    struct PhysicsTickEvent : public Event
    {
        double DeltaTime;
    };

    // Determine State of Trigger
    enum class TriggerState
    {
        NONE,
        ENTER,
        STAY,
        EXIT
    };

    struct PhysicsTriggerEvent : public Event
    {
        UUID TriggerID = UUID{ 0 };
        UUID OtherID = UUID{ 0 };
        TriggerState State = TriggerState::NONE;
    };
}