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
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<oo::LightingComponent>("Light")
        .property("Color", &LightingComponent::Color)
        .property("Radius", &LightingComponent::Radius)
        .property_readonly("Lighting ID", &LightingComponent::Light_ID);
    }
}
