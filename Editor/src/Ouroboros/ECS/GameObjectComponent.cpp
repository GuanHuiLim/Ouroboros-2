#include "pch.h"
#include "GameObjectComponent.h"

#include <rttr/registration>
#include <Ouroboros/EventSystem/EventManager.h>
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
    
    void oo::GameObjectComponent::SetHierarchyActive(bool active)
    {
        if (ActiveInHierarchy != active)
        {
            if(ActiveInHierarchy)
            {
                // if was active, call the disable event
                OnDisableEvent onDisableEvent;
                oo::EventManager::Broadcast(&onDisableEvent);
                LOG_CORE_INFO("GameObjectComponent OnDisable Invoke");
            }
            else
            {
                // if was inactive, call the enable event
                OnEnableEvent onEnableEvent;
                oo::EventManager::Broadcast(&onEnableEvent);
                LOG_CORE_INFO("GameObjectComponent OnEnable Invoke");
            }
            ActiveInHierarchy = active;
        }
    }
}
