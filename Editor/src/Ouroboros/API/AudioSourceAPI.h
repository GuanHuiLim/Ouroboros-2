/************************************************************************************//*!
\file           AudioSourceAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Nov 4, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to interact with the C++ AudioSourceComponent ECS Component

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once
#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/Audio/AudioSourceComponent.h"
#include "Ouroboros/Audio/Audio.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* AudioSource Component Functions for C#                                      */
    /*-----------------------------------------------------------------------------*/

    SCRIPT_API_FUNCTION(AudioSourceComponent, Play)
    SCRIPT_API_FUNCTION(AudioSourceComponent, Stop)
    SCRIPT_API_FUNCTION(AudioSourceComponent, Pause)
    SCRIPT_API_FUNCTION(AudioSourceComponent, UnPause)

    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Muted, bool, IsMuted, SetMuted)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, PlayOnAwake, bool, IsPlayOnAwake, SetPlayOnAwake)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Loop, bool, IsLoop, SetLoop)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, LoopBegin, uint, GetLoopBegin, SetLoopBegin)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, LoopEnd, uint, GetLoopEnd, SetLoopEnd)

    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Volume, float, GetVolume, SetVolume)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Pitch, float, GetPitch, SetPitch)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Priority, int, GetPriority, SetPriority)

    SCRIPT_API_GET_FUNC(AudioSourceComponent, IsPlaying, bool, IsPlaying)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, PlaybackTime, float, GetPlaybackTime, SetPlaybackTime)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, PlaybackTimeSamples, uint, GetPlaybackTimeSamples, SetPlaybackTimeSamples)

    SCRIPT_API AssetID AudioSourceComponent_GetAudioClip(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        return obj->GetComponent<AudioSourceComponent>().GetAudioClip().GetID();
    }

    SCRIPT_API void AudioSourceComponent_SetAudioClip(Scene::ID_type sceneID, UUID uuid, AssetID assetID)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        if (assetID == 0)
        {
            obj->GetComponent<AudioSourceComponent>().SetAudioClip(Asset{});
            return;
        }
        Asset asset = Project::GetAssetManager()->Get(assetID);
        if (asset.GetID() == Asset::ID_NULL || asset.GetType() != AssetInfo::Type::Audio)
            ScriptEngine::ThrowNullException();
        obj->GetComponent<AudioSourceComponent>().SetAudioClip(asset);
    }

    /*-----------------------------------------------------------------------------*/
    /* AudioClip Functions for C#                                                  */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API SoundID AudioClip_GetSoundID(AssetID assetID)
    {
        Asset asset = Project::GetAssetManager()->Get(assetID);
        if (asset.GetID() == Asset::ID_NULL || asset.GetType() != AssetInfo::Type::Audio)
            ScriptEngine::ThrowNullException();
        return asset.GetData<SoundID>();
    }

    SCRIPT_API float AudioClip_GetLength(SoundID id)
    {
        unsigned int length = 0;
        audio::GetSound(id)->getLength(&length, FMOD_TIMEUNIT_MS);
        return static_cast<float>(length) / 1000;
    }

    SCRIPT_API uint AudioClip_GetSampleCount(SoundID id)
    {
        unsigned int length = 0;
        audio::GetSound(id)->getLength(&length, FMOD_TIMEUNIT_PCM);
        return length;
    }
}