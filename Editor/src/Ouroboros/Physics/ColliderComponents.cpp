/************************************************************************************//*!
\file           ColliderComponents.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 2, 2022
\brief          Describes the various collider components that are supported.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "ColliderComponents.h"

#include <rttr/registration>

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::class_<BoxColliderComponent>("Box Collider")
            .property("Is Trigger ", &BoxColliderComponent::IsTrigger)
            .property("Offset", &BoxColliderComponent::Offset)
            //.property("Local Bounds", &BoxColliderComponent::Bounds)
            .property("Half Extents", &BoxColliderComponent::HalfExtents)
            .property("Size", &BoxColliderComponent::Size);

        registration::class_<SphereColliderComponent>("Sphere Collider")
            .property("Is Trigger", &SphereColliderComponent::IsTrigger)
            .property("Offset", &SphereColliderComponent::Offset)
            .property("Local Bounds", &SphereColliderComponent::Bounds)
            .property("Radius", &SphereColliderComponent::Radius);

        registration::class_<CapsuleColliderComponent>("Capsule Collider")
            .property("Is Trigger", &CapsuleColliderComponent::IsTrigger)
            .property("Offset", &CapsuleColliderComponent::Offset)
            .property("Radius", &CapsuleColliderComponent::Radius)
            .property("Half Height", &CapsuleColliderComponent::HalfHeight);
    }
}