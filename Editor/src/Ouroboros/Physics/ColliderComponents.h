/************************************************************************************//*!
\file           ColliderComponents.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 2, 2022
\brief          Describes the various collider components that are supported.

Copyright (C) 20212DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Shapes.h"
#include <rttr/type>

namespace oo
{
    /********************************************************************************//*!
     @brief    Defines the Various Types of Colliders Supported in the engine
    ************************************************************************************/
    enum class ColliderType
    {
        SPHERE,
        CUBE,
        CONVEX
    };

    /*-----------------------------------------------------------------------------*/
    /* Base collider component that all colliders inherit from                     */
    /*-----------------------------------------------------------------------------*/
    struct ColliderBase 
    {
        bool IsTrigger = false;
        vec3 Offset = { 0.0, 0.0, 0.0 };

        ColliderType Collider;

        RTTR_ENABLE();
    };

    /*-----------------------------------------------------------------------------*/
    /* Describes a Sphere Collider Component                                       */
    /*-----------------------------------------------------------------------------*/
    struct SphereColliderComponent final : public ColliderBase
    {
        Sphere Bounds = { { 0.f, 0.f, 0.f }, { 0.5f } };
        float Radius = 0.5f;

        Sphere GlobalBounds;

        RTTR_ENABLE(ColliderBase);
    };

    /*-----------------------------------------------------------------------------*/
    /* Describes a Box Collider Component                                          */
    /*-----------------------------------------------------------------------------*/
    struct BoxColliderComponent final : public ColliderBase
    {
        AABB Bounds = { { -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f } };
        vec3 Size = { 1.f, 1.f, 1.f };
        
        AABB GlobalBounds;

        RTTR_ENABLE(ColliderBase);
    };

}