#include "pch.h"
#include "GameObjectDebugComponent.h"
#include <rttr/registration>

namespace oo
{
    // FOR DEBUGGING PURPOSES.
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<GameObjectDebugComponent>("GameObject Debug")
        .property_readonly("My ID", &GameObjectDebugComponent::GetEntity)
        .property_readonly("Parent ID", &GameObjectDebugComponent::GetParentId)
        .property_readonly("My UUID", &GameObjectDebugComponent::GetUUID)
        .property_readonly("Parent UUID", &GameObjectDebugComponent::GetParentUUID)
        .property_readonly("List of Child UUIDs", &GameObjectDebugComponent::GetChildUUIDs)
        .property_readonly("No Of Childs", &GameObjectDebugComponent::GetChildCount);
    }
    
    GameObject::Entity GameObjectDebugComponent::GetEntity() const { return m_itself->GetEntity(); }
    GameObject::Entity GameObjectDebugComponent::GetParentId() const { return m_itself->GetParent().GetEntity(); }
    UUID GameObjectDebugComponent::GetUUID() const { return m_itself->GetInstanceID(); }
    UUID GameObjectDebugComponent::GetParentUUID() const { return m_itself->GetParentUUID(); }
    std::vector<UUID> GameObjectDebugComponent::GetChildUUIDs() const { return m_itself->GetChildrenUUID(); }
    std::size_t GameObjectDebugComponent::GetChildCount() const { return m_itself->GetChildren().size(); }
}
