/************************************************************************************//*!
\file           PhysicsComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 18, 2022
\brief          Backend component to help link up with physX and perform actual 
                instructions from the system

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <rttr/type>
#include <Physics/Source/phy.h>

namespace oo
{
    // invisible class
    struct PhysicsComponent
    {
        PhysicsObject object{};

        RTTR_ENABLE();
    };
}
