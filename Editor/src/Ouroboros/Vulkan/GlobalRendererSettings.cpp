/************************************************************************************//*!
\file           GlobalRendererSettings.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Sept 24, 2022
\brief          Defines data relevant to the renderer at a global level.
                Data here will be serialized and can be tweak via the editor to produce
                differing results in the scene.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "GlobalRendererSettings.h"

#include <rttr/registration>
namespace oo
{
    RTTR_REGISTRATION
    {
        using namespace rttr;
        registration::class_<GlobalRendererSettings::SSAOSettings>("SSAO Settings")
            .property("Radius", &GlobalRendererSettings::SSAOSettings::Radius)
            .property("Bias", &GlobalRendererSettings::SSAOSettings::Bias);
        registration::class_<GlobalRendererSettings::LightingSettings>("Light Settings")
            .property("Ambient", &GlobalRendererSettings::LightingSettings::Ambient)
            .property("Max Bias", &GlobalRendererSettings::LightingSettings::MaxBias)
            .property("Bias Multiplier", &GlobalRendererSettings::LightingSettings::BiasMultiplier);
        registration::class_<GlobalRendererSettings>("Renderer Settings")
            .property("SSAO Configuration", &GlobalRendererSettings::SSAO)
            .property("Light Configuration", &GlobalRendererSettings::Lighting);
    }
}
