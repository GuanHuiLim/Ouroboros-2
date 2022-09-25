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
    registration::class_<AudioSourceComponent>("Audio Source Component");
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

    void oo::AudioSourceComponent::SetMuted(bool m)
    {
        muted = m;
        if (channel)
        {
            FMOD_ERR_HAND(channel->setMute(m));
        }
    }

    void oo::AudioSourceComponent::SetPlayOnAwake(bool p)
    {
        playOnAwake = p;
    }

    void oo::AudioSourceComponent::SetLoop(bool l)
    {
        loop = l;
        if (channel)
        {
            FMOD_ERR_HAND(channel->setLoopCount(l ? -1 : 0));
        }
    }

    void oo::AudioSourceComponent::SetVolume(float v)
    {
        volume = v;
        if (channel)
        {
            FMOD_ERR_HAND(channel->setVolume(v));
        }
    }

    void oo::AudioSourceComponent::SetPitch(float p)
    {
        pitch = p;
        if (channel)
        {
            FMOD_ERR_HAND(channel->setPitch(p));
        }
    }

    void AudioSourceComponent::Play()
    {
        if (!audioClip.IsValid())
            return;
        FMOD::Sound* sound = audio::GetSound(audioClip.GetData<SoundID>());
        FMOD_ERR_HAND(audio::GetSystem()->playSound(sound, nullptr, false, &channel));
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
}
