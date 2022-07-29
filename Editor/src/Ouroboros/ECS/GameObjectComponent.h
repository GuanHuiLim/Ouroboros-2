#pragma once

#include <string>
#include "Utility/UUID.h"
#include <scenegraph/include/scenenode.h>

#include <rttr/type>

namespace oo
{
    class GameObjectComponent
    {
    public:
        bool Active = true;
        bool ActiveInHierarchy = true;
        UUID Id;

        // Should be string, but just string_view for now until ecs is fixed
        std::string Name = "Default Name Long enough for no short string optimization";

        scenenode::weak_pointer Node = {};//= nullptr;

        GameObjectComponent();

        RTTR_ENABLE();
    };
}