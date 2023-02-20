/************************************************************************************//*!
\file           LightComponent.h
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
#pragma once

#include <glm/glm.hpp>
#include <rttr/type>
#include "Color.h"
namespace oo
{
    enum class LightType
    {
        POINT,
        DIRECTIONAL,
    };

    class LightComponent
    {
    public:
        // shouldn't be changed by other systems.
        std::int32_t Light_ID = -1;

        Color Color = { };
        float Intensity = 1.f;
        //glm::vec4 Color = glm::vec4{ 1.f };
        float Radius = 1.f;
        //glm::vec4 Radius = glm::vec4{ 1.f }; // the true radius is vec4
        
        LightType LightType = LightType::POINT;

        bool ProduceShadows = false;
        RTTR_ENABLE();
    };
}
