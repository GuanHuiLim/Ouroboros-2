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
    registration::enumeration<AudioSourceGroup>("Audio Group")
        (
        value("None", AudioSourceGroup::None),
        value("SFX", AudioSourceGroup::SFXGeneral),
        value("SFX - Voice", AudioSourceGroup::SFXVoice),
        value("SFX - Environment", AudioSourceGroup::SFXEnvironment),
        value("Music", AudioSourceGroup::Music)
        );
    registration::class_<AudioSourceComponent>("Audio Source")
        .property("Audio Clip", &AudioSourceComponent::GetAudioClip, &AudioSourceComponent::SetAudioClip)
        (metadata(UI_metadata::ASSET_TYPE, static_cast<int>(AssetInfo::Type::Audio)))
        .property("Audio Group", &AudioSourceComponent::GetGroup, &AudioSourceComponent::SetGroup)
        .property("Mute", &AudioSourceComponent::IsMuted, &AudioSourceComponent::SetMuted)
        .property("Play On Awake", &AudioSourceComponent::IsPlayOnAwake, &AudioSourceComponent::SetPlayOnAwake)
        .property("Loop", &AudioSourceComponent::IsLoop, &AudioSourceComponent::SetLoop)
        .property("Volume", &AudioSourceComponent::GetVolume, &AudioSourceComponent::SetVolume)
        (metadata(UI_metadata::DRAG_SPEED, 0.01f))
        .property("Pitch", &AudioSourceComponent::GetPitch, &AudioSourceComponent::SetPitch)
        (metadata(UI_metadata::DRAG_SPEED, 0.01f))
        .property("Priority", &AudioSourceComponent::GetPriority, &AudioSourceComponent::SetPriority);
    };

    bool AudioSourceComponent::IsPlaying() const
    {
        if (!channel)
            return false;
        bool result;
        FMOD_ERR_HAND(channel->isPlaying(&result));
        return result;
    }

    float AudioSourceComponent::GetPlaybackTime() const
    {
        if (!channel)
            return 0;
        unsigned int result;
        FMOD_ERR_HAND(channel->getPosition(&result, FMOD_TIMEUNIT_MS));
        return static_cast<float>(result) / 1000;
    }

    unsigned int AudioSourceComponent::GetPlaybackTimeSamples() const
    {
        if (!channel)
            return 0;
        unsigned int result;
        FMOD_ERR_HAND(channel->getPosition(&result, FMOD_TIMEUNIT_PCM));
        return result;
    }

    void AudioSourceComponent::SetAudioClip(Asset a)
    {
        audioClip = a;
        if (!channel)
            return;
        Play();
    }

    void AudioSourceComponent::SetGroup(AudioSourceGroup g)
    {
        group = g;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setChannelGroup(audio::GetChannelGroup(g)));
    }

    void AudioSourceComponent::SetMuted(bool m)
    {
        muted = m;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setMute(m));
    }

    void AudioSourceComponent::SetPlayOnAwake(bool p)
    {
        playOnAwake = p;
    }

    void AudioSourceComponent::SetLoop(bool l)
    {
        loop = l;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setLoopCount(l ? -1 : 0));
    }

    void oo::AudioSourceComponent::SetLoopBegin(unsigned int t)
    {
        loopBegin = t;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setLoopPoints(t, FMOD_TIMEUNIT_MS, loopEnd, FMOD_TIMEUNIT_MS));
    }

    void oo::AudioSourceComponent::SetLoopEnd(unsigned int t)
    {
        loopEnd = t;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setLoopPoints(loopBegin, FMOD_TIMEUNIT_MS, t, FMOD_TIMEUNIT_MS));
    }

    void AudioSourceComponent::SetVolume(float v)
    {
        volume = v;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setVolume(v));
    }

    void AudioSourceComponent::SetPitch(float p)
    {
        pitch = p;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setPitch(p));
    }

    void AudioSourceComponent::SetPriority(int p)
    {
        if (p < 0)
            p = 0;
        if (p > 256)
            p = 256;
        priority = p;
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setPriority(p));
    }

    void AudioSourceComponent::SetPlaybackTime(float t)
    {
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setPosition(static_cast<unsigned int>(t * 1000), FMOD_TIMEUNIT_MS));
    }

    void AudioSourceComponent::SetPlaybackTimeSamples(unsigned int t)
    {
        if (!channel)
            return;
        FMOD_ERR_HAND(channel->setPosition(t, FMOD_TIMEUNIT_PCM));
    }

    void AudioSourceComponent::Set3DPosition(float x, float y, float z)
    {
        posX = x;
        posY = y;
        posZ = z;
    }

    void AudioSourceComponent::Play()
    {
        if (!audioClip.IsValid())
            return;
        Stop(); // always stop whatever sound it was playing before
        FMOD::Sound* sound = audio::GetSound(audioClip.GetData<SoundID>());
        FMOD_ERR_HAND(audio::GetSystem()->playSound(sound, nullptr, false, &channel));
        FMOD_ERR_HAND(channel->setMode(FMOD_3D));
        FMOD_VECTOR fmPos = { .x = posX, .y = posY, .z = posZ };
        FMOD_ERR_HAND(channel->set3DAttributes(&fmPos, nullptr));
        SetGroup(group);
        SetMuted(muted);
        SetLoop(loop);
        SetLoopBegin(loopBegin);
        SetLoopEnd(loopEnd);
        SetVolume(volume);
        SetPitch(pitch);
        SetPriority(priority);
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
