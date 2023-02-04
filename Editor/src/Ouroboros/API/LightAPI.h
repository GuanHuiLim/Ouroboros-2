/************************************************************************************//*!
\file           LightAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Nov 4, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the C++ LightComponent ECS Component

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Ouroboros/Scripting/ScriptValue.h"
#include "Ouroboros/Vulkan/LightComponent.h"

namespace oo
{
    SCRIPT_API_GET_SET(LightComponent, Color, Color, Color)
    SCRIPT_API_GET_SET(LightComponent, Intensity, float, Intensity)
    SCRIPT_API_GET_SET(LightComponent, Radius, float, Radius)
    SCRIPT_API_GET_SET(LightComponent, ProduceShadows, bool, ProduceShadows)
}
