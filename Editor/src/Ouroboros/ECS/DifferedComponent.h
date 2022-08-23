/************************************************************************************//*!
\file           DifferedComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Aug 23, 2022
\brief          All Components holding onto this component will need to be considered
                differed and will not be updated for the current frame.
                This Component will be removed by the end of this frame by the system.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Archetypes_Ecs/src/A_Ecs.h>
#include <rttr/type>

namespace oo
{
    struct DifferedComponent
    {
        DifferedComponent();

        Ecs::EntityID entityID;

        RTTR_ENABLE();
    };
}
