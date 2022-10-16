/************************************************************************************//*!
\file           GameObject.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Spet 23, 2022
\brief          Extends the ECS feature set that requires extra information 
                only found within the editor.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Archetypes_ECS/src/A_Ecs.h>
#include "DeferredComponent.h"
#include "DuplicatedComponent.h"

namespace Ecs
{
    //Extending queries
    template<typename ...T>
    Query make_query()
    {
        return [&]()
        {
            Ecs::Query query;
            query.with<T...>().exclude<oo::DeferredComponent, oo::DuplicatedComponent>().build();
            return query;
        }();
    }

    template<typename ...T>
    Query make_raw_query()
    {
        return [&]()
        {
            Ecs::Query query;
            query.with<T...>().build();
            return query;
        }();
    }
}
