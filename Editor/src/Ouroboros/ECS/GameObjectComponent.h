#pragma once

#include <string>
#include "Utility/UUID.h"
#include <scenegraph/include/scenenode.h>

#include <rttr/type>
#include <Ouroboros/EventSystem/Event.h>

namespace oo
{
    class GameObjectComponent
    {
    public:
        bool Active = true;
        bool ActiveInHierarchy = true;
        UUID Id;
        std::string Name = "Default Name Long enough for no short string optimization";
        scenenode::weak_pointer Node = {};

    public:
        
        struct OnEnableEvent : public Event
        {
        };

        struct OnDisableEvent : public Event
        {
        };

    public:
        void SetHierarchyActive(bool active);

        RTTR_ENABLE();
    };
}