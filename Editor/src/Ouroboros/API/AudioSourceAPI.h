#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Audio/AudioSourceComponent.h"

namespace oo
{
    SCRIPT_API_FUNCTION(AudioSourceComponent, Play)
    SCRIPT_API_FUNCTION(AudioSourceComponent, Stop)
    SCRIPT_API_FUNCTION(AudioSourceComponent, Pause)
    SCRIPT_API_FUNCTION(AudioSourceComponent, UnPause)

    SCRIPT_API_GET_SET(AudioSourceComponent, Muted, bool, IsMuted, SetMuted)
    SCRIPT_API_GET_SET(AudioSourceComponent, PlayOnAwake, bool, IsPlayOnAwake, SetPlayOnAwake)
    SCRIPT_API_GET_SET(AudioSourceComponent, Loop, bool, IsLoop, SetLoop)

    SCRIPT_API_GET_SET(AudioSourceComponent, Volume, float, GetVolume, SetVolume)
    SCRIPT_API_GET_SET(AudioSourceComponent, Pitch, float, GetPitch, SetPitch)

    SCRIPT_API_GET(AudioSourceComponent, IsPlaying, bool, IsPlaying)
    SCRIPT_API_GET(AudioSourceComponent, GetPlaybackTime, float, GetPlaybackTime)
}