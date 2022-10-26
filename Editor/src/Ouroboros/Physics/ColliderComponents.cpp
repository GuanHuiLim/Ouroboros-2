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

#include "App/Editor/Properties/UI_metadata.h"

namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;

        registration::class_<BoxColliderComponent>("Box Collider")
            //.property("Is Trigger ", &BoxColliderComponent::IsTrigger)
            //.property("Offset", &BoxColliderComponent::Offset)
            //.property("Local Bounds", &BoxColliderComponent::Bounds)
            .property("Half Extents", &BoxColliderComponent::HalfExtents)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Size", &BoxColliderComponent::Size)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Global Half Extents", &BoxColliderComponent::GlobalHalfExtents);

        registration::class_<SphereColliderComponent>("Sphere Collider")
            //.property("Is Trigger", &SphereColliderComponent::IsTrigger)
            //.property("Offset", &SphereColliderComponent::Offset)
            .property("Local Bounds", &SphereColliderComponent::Bounds)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Radius", &SphereColliderComponent::Radius)(metadata(UI_metadata::DRAG_SPEED, 0.1f));

        registration::class_<CapsuleColliderComponent>("Capsule Collider")
            //.property("Is Trigger", &CapsuleColliderComponent::IsTrigger)
            //.property("Offset", &CapsuleColliderComponent::Offset)
            .property("Radius", &CapsuleColliderComponent::Radius)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Half Height", &CapsuleColliderComponent::HalfHeight)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Global Radius", &CapsuleColliderComponent::GlobalRadius)
            .property_readonly("Global Half Height", &CapsuleColliderComponent::GlobalHalfHeight)
            ;
    }
}