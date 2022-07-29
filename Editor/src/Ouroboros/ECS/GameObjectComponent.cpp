#include "pch.h"
#include "GameObjectComponent.h"

#include <rttr/registration>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<oo::GameObjectComponent>("GameObject")
            .property("Active", &GameObjectComponent::Active)
            .property("Name", &GameObjectComponent::Name)
            .property("UUID", &GameObjectComponent::Id)
            //.property("Layer", &GameObjectComponent::GetLayer, &GameObjectComponent::SetLayer)
            .property("Active In Hierarchy", &GameObjectComponent::ActiveInHierarchy);
    }

    GameObjectComponent::GameObjectComponent()
        : Active{ true }
        , ActiveInHierarchy{ true }
        , Id { }
        , Name { "Default Name Long enough for no short string optimization" }
        , Node { }
    {
    }
}
