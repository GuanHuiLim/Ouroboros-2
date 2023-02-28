#pragma once

#include <rttr/type>

namespace oo
{
    class WaypointNodeComponent 
    {
    public:
        enum class MovementType
        {
            LINEAR,
            BEZIER,
        };

        MovementType MoveType = MovementType::LINEAR;

        // Variables to set in editor
        float WaitTimeBeforeMoving = 0.f;
        float TimeToNextNode = 1.f;

        // variables hidden from editor, for systems to update
        float WaitedFor = 0.f;
        float MovedFor = 0.f;

        // this should only be touched by waypoint system, used to check if its the last node.
        UUID NextNodeUUID = UUID::Invalid;

        RTTR_ENABLE();
    };
}
