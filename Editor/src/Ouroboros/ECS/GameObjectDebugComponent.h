#pragma once

#include "GameObject.h"
#include <rttr/type>
namespace oo
{
    class GameObjectDebugComponent
    {
    private:
        GameObject* m_itself = nullptr;

    public :
        GameObjectDebugComponent() = default;

        GameObjectDebugComponent(GameObject*go)
            : m_itself { go }
        {
        }

    public:
        GameObject::Entity GetEntity() const;
        GameObject::Entity GetParentId() const;
        UUID GetUUID() const;
        UUID GetParentUUID() const;
        std::vector<UUID> GetChildUUIDs() const;
        std::size_t GetChildCount() const;

        RTTR_ENABLE();
    };
}
