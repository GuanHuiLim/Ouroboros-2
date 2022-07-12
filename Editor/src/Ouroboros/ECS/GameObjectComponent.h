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
        std::string Name = "Default";

        scenenode::weak_pointer Node = {};
    };
}