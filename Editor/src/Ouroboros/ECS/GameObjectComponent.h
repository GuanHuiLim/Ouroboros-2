#pragma once

#include <string>
#include "Utility/UUID.h"
#include <scenenode.h>

namespace oo
{
    struct GameObjectComponent
    {
        bool Active = true;
        bool ActiveInHierarchy = true;
        UUID Id;

        // Should be string, but just string_view for now until ecs is fixed
        const std::string_view Name = "Default";

        scenenode::raw_pointer Node = nullptr;
    };
}