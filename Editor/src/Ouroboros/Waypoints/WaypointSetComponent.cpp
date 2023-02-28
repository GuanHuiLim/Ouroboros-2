#include "pch.h"
#include "WaypointSetComponent.h"

#include <rttr/registration.h>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::class_<WaypointSetComponent>("Waypoint Set")
            .property("Target Name", &WaypointSetComponent::TargetName)
            .property("Total Elapsed Time", &WaypointSetComponent::TotalTimeElapsed)
            .property_readonly("Current Node UUID", &WaypointSetComponent::CurrentNodeUUID)
            .property_readonly("First Node UUID", &WaypointSetComponent::FirstNodeUUID)
            ;
    }
}
