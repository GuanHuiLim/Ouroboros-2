/************************************************************************************//*!
\file           Manifold.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 2, 2021
\brief          Manifold describes the useful information from collision
                used for physics-resolution

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "PhysicsFwd.h"

namespace oo
{
    /************************************************************************************//*!
        Describes vital information for each contact collision point
    *//*************************************************************************************/
    struct Contact2D final
    {
        vec2 Position           = vec2{ 0 };
        vec2 Normal             = vec2{ 0 };    // Normal Should be From A to B
        float PenetrationDepth  = 0.f;
    };

    /************************************************************************************//*!
        Describes the most important information that will be used to solve various
        physics constrains
    *//*************************************************************************************/
    struct Manifold2D final
    {
        //Entity EnttA{ std::numeric_limits<Entity>::max() };
        //Entity EnttB{ std::numeric_limits<Entity>::max() };

        //Rigidbody2D* RigidA = nullptr;
        //Rigidbody2D* RigidB = nullptr;

        //// Contact information
        //int ContactPoints = 0;
        //Contact2D Contacts[2];  // 2 is all you need in 2 dimensions, 4 in 3D

        //bool HasCollision = false;
    };

}
