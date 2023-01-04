/************************************************************************************//*!
\file           ParticleEmitterAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Nov 4, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the C++ ParticleEmitterComponent ECS Component

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Ouroboros/Vulkan/ParticleEmitterComponent.h"

namespace oo
{
    SCRIPT_API_FUNCTION(ParticleEmitterComponent, Play)
    SCRIPT_API_FUNCTION(ParticleEmitterComponent, Stop)
    SCRIPT_API_FUNCTION(ParticleEmitterComponent, ResetSystem)

    SCRIPT_API_GET_SET_FUNC(ParticleEmitterComponent, IsPlaying, bool, GetPlaying, SetPlaying)
}