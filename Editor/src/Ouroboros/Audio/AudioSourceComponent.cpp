/************************************************************************************//*!
\file           AudioSourceComponent.cpp
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Sep 25, 2022
\brief          Contains the definition for the Audio Source component.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

#include "AudioSourceComponent.h"

#include <rttr/registration>

#include "App/Editor/Properties/UI_metadata.h"
#include "Ouroboros/Audio/Audio.h"

namespace oo
{
    // TODO:
    // serialisation
    // play on awake
    // spatialisation (need transform component)
    // complete component integration
    //

    RTTR_REGISTRATION
    {
        using namespace rttr;
    registration::class_<AudioSourceComponent>("Audio Source Component")
        .property("Audio Clip", &AudioSourceComponent::GetAudioClip, &AudioSourceComponent::SetAudioClip)
        (metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Audio)))
        .property("Mute", &AudioSourceComponent::IsMuted, &AudioSourceComponent::SetMuted)
        .property("Play On Awake", &AudioSourceComponent::IsPlayOnAwake, &AudioSourceComponent::SetPlayOnAwake)
        .property("Loop", &AudioSourceComponent::IsLoop, &AudioSourceComponent::SetLoop)
        .property("Volume", &AudioSourceComponent::GetVolume, &AudioSourceComponent::SetVolume)
        (metadata(UI_metadata::DRAG_SPEED, 0.1f))
        .property("Pitch", &AudioSourceComponent::GetPitch, &AudioSourceComponent::SetPitch)
        (metadata(UI_metadata::DRAG_SPEED, 0.1f));
    };

    bool oo::AudioSourceComponent::IsPlaying() const
    {
        if (!channel)
            return false;
        bool result;
        FMOD_ERR_HAND(channel->isPlaying(&result));
        return result;
    }

    float oo::AudioSourceComponent::GetPlaybackTime() const
    {
        if (!channel)
            return 0;
        unsigned int result;
        channel->getPosition(&result, FMOD_TIMEUNIT_MS);
        return static_cast<float>(result) / 1000;
    }

    void oo::AudioSourceComponent::SetAudioClip(Asset a)
    {
        audioClip = a;
    }

    void oo::AudioSourceComponent::SetMuted(bool m)
    {
        muted = m;
        isDirty = true;
    }

    void oo::AudioSourceComponent::SetPlayOnAwake(bool p)
    {
        playOnAwake = p;
    }

    void oo::AudioSourceComponent::SetLoop(bool l)
    {
        loop = l;
        isDirty = true;
    }

    void oo::AudioSourceComponent::SetVolume(float v)
    {
        volume = v;
        isDirty = true;
    }

    void oo::AudioSourceComponent::SetPitch(float p)
    {
        pitch = p;
        isDirty = true;
    }

    void AudioSourceComponent::Play()
    {
        if (!audioClip.IsValid())
            return;
        Stop(); // always stop whatever sound it was playing before
        FMOD::Sound* sound = audio::GetSound(audioClip.GetData<SoundID>());
        FMOD_ERR_HAND(audio::GetSystem()->playSound(sound, nullptr, false, &channel));
        FMOD_ERR_HAND(channel->setMode(FMOD_3D));
    }

    void AudioSourceComponent::Stop()
    {
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->stop());
        channel = nullptr;
    }

    void AudioSourceComponent::Pause()
    {
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setPaused(true));
    }

    void AudioSourceComponent::UnPause()
    {
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setPaused(false));
    }

    void oo::AudioSourceComponent::ClearDirty()
    {
        isDirty = false;
    }
}
