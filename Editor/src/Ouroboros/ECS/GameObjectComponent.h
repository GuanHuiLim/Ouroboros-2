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
        const std::string_view Name = "Default Name Long enough for no short string optimization";

        scenenode::weak_pointer Node = {};//= nullptr;

        //components must have all 5 : default ctor, copy and move ctor/assignment

        /*GameObjectComponent() = default;
        GameObjectComponent(GameObjectComponent const& copyconstruct) = default;
        GameObjectComponent& operator=(GameObjectComponent const& copyassign) = default;*/
        
        /*GameObjectComponent(GameObjectComponent&& other) noexcept
            : Name {std::move(other.Name)}
            , Node {std::move(other.Node)}
            , Active {std::move(other.Active)}
            , ActiveInHierarchy {std::move(other.ActiveInHierarchy)}
            , Id {std::move(Id)}
        {
        }

        GameObjectComponent& operator=(GameObjectComponent&& moveassign) noexcept
        {
            *this = std::move(moveassign);
            return *this;
        }*/
        
        /*GameObjectComponent(GameObjectComponent&& moveconstruct) noexcept = default;
        GameObjectComponent& operator=(GameObjectComponent&& moveassign) noexcept = default;*/

    };
}