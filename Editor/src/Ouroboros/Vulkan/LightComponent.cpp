/************************************************************************************//*!
\file           LightComponent.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Sept 24, 2022
\brief          Defines the data required to allow users to interface with the backend
                lighting system provided by the graphics engine

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "LightComponent.h"

#include <rttr/registration>

#include "App/Editor/Properties/UI_metadata.h"
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<oo::LightComponent>("Light")
        .property("Color", &LightComponent::Color)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
        .property("Radius", &LightComponent::Radius)(metadata(UI_metadata::DRAG_SPEED, 0.1f))
        .property_readonly("Lighting ID", &LightComponent::Light_ID);
    }
}
