/************************************************************************************//*!
\file           CameraComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Sept 24, 2022
\brief          Defines the data required to allow users to interface with the backend
                camera from the graphics world

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <glm/glm.hpp>
#include <rttr/type>
namespace oo
{
    enum class CameraAspectRatio
    {
        SIXTEEN_BY_NINE,
        SIXTEEN_BY_TEN,
        FOUR_BY_THREE,
    };

    class CameraComponent
    {
    public:
        CameraAspectRatio AspectRatio;
        RTTR_ENABLE();
    };
}
