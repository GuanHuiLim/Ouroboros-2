/************************************************************************************//*!
\file           ColliderComponents.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           September 2, 2021
\brief          Describes the various collider components that are supported.
                Take note not all colliders should be exposed. Some are only for physics
                to take control of.

Copyright (C) 2021 DigiPen Institute of Technology.
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

        /*registration::class_<BoxCollider>("Collider")
            .property("Is Trigger ", &BoxCollider::IsTrigger)
            .property("Offset", &BoxCollider::Offset);*/

        registration::class_<BoxColliderComponent>("Box Collider")
            .property("Is Trigger ", &BoxColliderComponent::IsTrigger)
            .property("Offset", &BoxColliderComponent::Offset)
            .property("Bounds", &BoxColliderComponent::Bounds)
            .property("Size", &BoxColliderComponent::Size);

        registration::class_<SphereColliderComponent>("Sphere Collider")
            .property("Is Trigger", &SphereColliderComponent::IsTrigger)
            .property("Offset", &SphereColliderComponent::Offset)
            .property("Bounds", &SphereColliderComponent::Bounds)
            .property("Radius", &SphereColliderComponent::Radius);
    }
}