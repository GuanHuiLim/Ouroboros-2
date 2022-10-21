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