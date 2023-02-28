#include "pch.h"
#include "WaypointNodeComponent.h"

#include <rttr/registration.h>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::enumeration<WaypointNodeComponent::MovementType>("Movement Type")
        (
            value("Linear", WaypointNodeComponent::MovementType::LINEAR),
            value("Bezier", WaypointNodeComponent::MovementType::BEZIER)
        );

        registration::class_<WaypointNodeComponent>("Waypoint Node")
            .property("Movement Type", &WaypointNodeComponent::MoveType)
            .property("Wait Time Before Moving", &WaypointNodeComponent::WaitTimeBeforeMoving)
            .property("Time to next Node", &WaypointNodeComponent::TimeToNextNode)
            .property_readonly("Waited For", &WaypointNodeComponent::WaitedFor)
            .property_readonly("Moved For", &WaypointNodeComponent::MovedFor)
            .property_readonly("Next Node UUID", &WaypointNodeComponent::NextNodeUUID)
            ;
    }
}
