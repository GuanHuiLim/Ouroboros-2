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
    /* Describes a Sphere Collider Component                                       */
    /*-----------------------------------------------------------------------------*/
    struct SphereColliderComponent final /*: public ColliderBase*/
    {
        float Radius = 0.5f;

        float GlobalRadius = 0.5f;
        RTTR_ENABLE();
    };

    /*-----------------------------------------------------------------------------*/
    /* Describes a Box Collider Component                                          */
    /*-----------------------------------------------------------------------------*/
    struct BoxColliderComponent final /*: public ColliderBase*/
    {
        vec3 HalfExtents = { 0.5f, 0.5f, 0.5f };
        vec3 Size = { 1.f, 1.f, 1.f };
        
        vec3 GlobalHalfExtents = { 0.5f, 0.5f, 0.5f };
        RTTR_ENABLE();
    };

    struct CapsuleColliderComponent final /*: public ColliderBase*/
    {
        float Radius        = 0.5f;
        float HalfHeight    = 0.5f;

        float GlobalRadius = 0.5f;
        float GlobalHalfHeight = 0.5f;
        RTTR_ENABLE();
    };

    struct MeshColliderComponent final
    {
        bool Reset = false;
        std::vector<vec3> Vertices;
        RTTR_ENABLE();
    };

}