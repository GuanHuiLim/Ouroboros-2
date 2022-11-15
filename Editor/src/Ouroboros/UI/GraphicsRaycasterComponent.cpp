#include "pch.h"
#include "GraphicsRaycasterComponent.h"
#include <rttr/registration>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::class_<GraphicsRaycasterComponent>("Graphics Raycaster")
            //.property("Enable", &GraphicsRaycasterComponent::Enabled)
            ;
    }
}
