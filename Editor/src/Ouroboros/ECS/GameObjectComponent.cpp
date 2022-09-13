/************************************************************************************//*!
\file           GameObjectComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Describes component holding basic information that should be accessible
                by all and all gameobjects should have one of these components by default.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "GameObjectComponent.h"

#include <rttr/registration>
#include <Ouroboros/EventSystem/EventManager.h>
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
	registration::class_<oo::GameObjectComponent>("GameObject")
		.property("Active", &GameObjectComponent::Active)
		.property("Name", &GameObjectComponent::Name)
		.property("UUID", &GameObjectComponent::Id)
		//.property("Layer", &GameObjectComponent::GetLayer, &GameObjectComponent::SetLayer)
		.property("Active In Hierarchy", &GameObjectComponent::ActiveInHierarchy)
		.property("TestAsset", &GameObjectComponent::TestAsset);
    }
}
