/************************************************************************************//*!
\file           GameObjectComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes component holding debugging information that should be accessible
                by all and all gameobjects should have one of this component which will be
                stripped during production.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
        oo::UUID GetUUID() const;
        oo::UUID GetParentUUID() const;
        std::vector<oo::UUID> GetChildUUIDs() const;
        std::size_t GetChildCount() const;

        RTTR_ENABLE();
    };
}
