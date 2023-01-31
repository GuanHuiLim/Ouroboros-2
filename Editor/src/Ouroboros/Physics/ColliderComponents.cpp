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
            .property("Half Extents", &BoxColliderComponent::HalfExtents)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Size", &BoxColliderComponent::Size)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Global Half Extents", &BoxColliderComponent::GlobalHalfExtents);

        registration::class_<SphereColliderComponent>("Sphere Collider")
            .property("Radius", &SphereColliderComponent::Radius)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Global Radius", &SphereColliderComponent::GlobalRadius);

        registration::class_<CapsuleColliderComponent>("Capsule Collider")
            .property("Radius", &CapsuleColliderComponent::Radius)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property("Half Height", &CapsuleColliderComponent::HalfHeight)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
            .property_readonly("Global Radius", &CapsuleColliderComponent::GlobalRadius)
            .property_readonly("Global Half Height", &CapsuleColliderComponent::GlobalHalfHeight)
            ;

        registration::class_<MeshColliderComponent>("Mesh Collider")
            .property("Reset", &MeshColliderComponent::Reset)
            .property_readonly("vertices", &MeshColliderComponent::Vertices);
    }
}