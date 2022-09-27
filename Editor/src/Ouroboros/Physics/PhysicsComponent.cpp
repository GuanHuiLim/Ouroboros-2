#include "pch.h"
#include "PhysicsComponent.h"

#include <rttr/registration>
namespace oo
{
    // FOR DEBUGGING PURPOSES.
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<PhysicsComponent>("Physics");
    }
}
