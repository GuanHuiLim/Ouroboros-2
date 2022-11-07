/************************************************************************************//*!
\file           Color.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Sept 30, 2022
\brief          Defines a color struct that wraps glm vec4

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "pch.h"
namespace oo
{
    class Color
    {
    public:
        float r = 1.f, g = 1.f, b = 1.f, a = 1.f;
        //glm::vec4 Color = glm::vec4{ 1.f };
        operator glm::vec4() const { return glm::vec4{ r, g, b, a }; }
    };
}
