#pragma once

#include "WaypointNodeComponent.h"
#include <string>

#include <rttr/type>
namespace oo
{
    class WaypointSetComponent
    {
    public:
        std::string TargetName = "Name of Gameobject you want this to move.";

        // Variables to set in editor
        float TotalTimeElapsed = 0.f;

        // this should only be touched by waypoint system, used to check if its the last node.
        UUID CurrentNodeUUID = UUID::Invalid;
        UUID FirstNodeUUID = UUID::Invalid;

        RTTR_ENABLE();
    };
}
