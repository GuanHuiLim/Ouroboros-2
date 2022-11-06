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

namespace oo
{
    SCRIPT_API_FUNCTION(AudioSourceComponent, Play)
    SCRIPT_API_FUNCTION(AudioSourceComponent, Stop)
    SCRIPT_API_FUNCTION(AudioSourceComponent, Pause)
    SCRIPT_API_FUNCTION(AudioSourceComponent, UnPause)

    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Muted, bool, IsMuted, SetMuted)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, PlayOnAwake, bool, IsPlayOnAwake, SetPlayOnAwake)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Loop, bool, IsLoop, SetLoop)

    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Volume, float, GetVolume, SetVolume)
    SCRIPT_API_GET_SET_FUNC(AudioSourceComponent, Pitch, float, GetPitch, SetPitch)

    SCRIPT_API_GET_FUNC(AudioSourceComponent, IsPlaying, bool, IsPlaying)
    SCRIPT_API_GET_FUNC(AudioSourceComponent, GetPlaybackTime, float, GetPlaybackTime)

    SCRIPT_API AssetID AudioSourceComponent_GetAudioClip(Scene::ID_type sceneID, UUID uuid)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        return obj->GetComponent<AudioSourceComponent>().GetAudioClip().GetID();
    }

    SCRIPT_API void AudioSourceComponent_SetAudioClip(Scene::ID_type sceneID, UUID uuid, AssetID assetID)
    {
        std::shared_ptr<GameObject> obj = ScriptManager::GetObjectFromScene(sceneID, uuid);
        Asset asset = Project::GetAssetManager()->Get(assetID);
        if (asset.GetID() == Asset::ID_NULL || asset.GetType() != AssetInfo::Type::Audio)
            ScriptEngine::ThrowNullException();
        obj->GetComponent<AudioSourceComponent>().SetAudioClip(asset);
    }
}