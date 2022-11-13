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
#include "pch.h"
#include "CameraComponent.h"

#include <rttr/registration>
#include "App/Editor/Properties/UI_metadata.h"
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::enumeration<CameraAspectRatio>("Aspect Ratio")
        (
            value("16:9", CameraAspectRatio::SIXTEEN_BY_NINE),
            value("16:10", CameraAspectRatio::SIXTEEN_BY_TEN),
            value("4:3", CameraAspectRatio::FOUR_BY_THREE)
        );

        registration::class_<oo::CameraComponent>("Camera")
            .property("Main Camera", &CameraComponent::MainCamera)
            .property_readonly("Graphics World Camera Index", &CameraComponent::GraphicsWorldIndex)
            .property("Aspect Ratio", &CameraComponent::AspectRatio)
            ;
    }
}
