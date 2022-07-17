#pragma once

#include <string>
#include "Utility/UUID.h"
#include <scenegraph/include/scenenode.h>

namespace oo
{
    struct GameObjectComponent
    {
        bool Active = true;
        bool ActiveInHierarchy = true;
        UUID Id;

        // Should be string, but just string_view for now until ecs is fixed
        std::string Name = "Default Name Long enough for no short string optimization";

        scenenode::weak_pointer Node = {};//= nullptr;
    };
}