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