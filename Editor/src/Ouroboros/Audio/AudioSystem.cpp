/************************************************************************************//*!
\file           AudioSystem.cpp
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Sep 26, 2022
\brief          Contains the definition for the Audio System.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

#include "AudioSystem.h"

#include "Ouroboros/Audio/Audio.h"
#include "Ouroboros/Audio/AudioListenerComponent.h"
#include "Ouroboros/Audio/AudioSourceComponent.h"
#include "Ouroboros/ECS/ECS.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/EventSystem/EventTypes.h"

namespace oo
{
    AudioSystem::AudioSystem(Scene* scene)
        : scene{ scene }
    {
        EventManager::Subscribe<AudioSystem, LoadSceneEvent>(this, &AudioSystem::onLoadScene);
        EventManager::Subscribe<AudioSystem, UnloadSceneEvent>(this, &AudioSystem::onUnloadScene);
        EventManager::Subscribe<AudioSystem, GameObjectComponent::OnEnableEvent>(this, &AudioSystem::onObjectEnabled);
        EventManager::Subscribe<AudioSystem, GameObjectComponent::OnDisableEvent>(this, &AudioSystem::onObjectDisabled);
    }

    AudioSystem::~AudioSystem()
    {
        EventManager::Unsubscribe<AudioSystem, LoadSceneEvent>(this, &AudioSystem::onLoadScene);
        EventManager::Unsubscribe<AudioSystem, UnloadSceneEvent>(this, &AudioSystem::onUnloadScene);
        EventManager::Unsubscribe<AudioSystem, GameObjectComponent::OnEnableEvent>(this, &AudioSystem::onObjectEnabled);
        EventManager::Unsubscribe<AudioSystem, GameObjectComponent::OnDisableEvent>(this, &AudioSystem::onObjectDisabled);
    }

    void AudioSystem::Run(Ecs::ECSWorld* world)
    {
        static constexpr const char* const AUDIO_UPDATE = "audio_update";
        TRACY_TRACK_PERFORMANCE(AUDIO_UPDATE);
        TRACY_PROFILE_SCOPE_NC(AUDIO_UPDATE, tracy::Color::Aquamarine1);

        // Iterate audio listeners
        {
            static Ecs::Query query = Ecs::make_query<AudioListenerComponent>();
            bool has = false;
            bool warned = false;
            world->for_each(query, [&](AudioListenerComponent& al, TransformComponent& tf)
            {
                if (!has)
                {
                    auto tfPos = tf.GetGlobalPosition();
                    FMOD_VECTOR fmPos = { .x = tfPos.x, .y = tfPos.y, .z = tfPos.z };
                    // TODO: forward and up vectors
                    audio::GetSystem()->set3DListenerAttributes(0, &fmPos, nullptr, nullptr, nullptr);
                    has = true;
                }
                else if (!warned)
                {
                    LOG_WARN("Should not have more than one Audio Listener in a scene!");
                    warned = true;
                }
            });
        }

        // Iterate audio sources
        {
            static Ecs::Query query = Ecs::make_query<AudioSourceComponent>();
            world->for_each(query, [&](AudioSourceComponent& as, TransformComponent& tf)
            {
                auto tfPos = tf.GetGlobalPosition();
                FMOD_VECTOR fmPos = { .x = tfPos.x, .y = tfPos.y, .z = tfPos.z };
                as.GetChannel()->set3DAttributes(&fmPos, nullptr);
            });
        }

        TRACY_PROFILE_SCOPE_END();
        TRACY_DISPLAY_PERFORMANCE_SELECTED(AUDIO_UPDATE);
    }

    void AudioSystem::playAllOnAwake()
    {
        static Ecs::Query query = Ecs::make_query<AudioSourceComponent>();
        scene->GetWorld().for_each(query, [&](AudioSourceComponent& as, TransformComponent&)
        {
            if (as.IsPlayOnAwake())
                as.Play();
        });
        LOG_INFO("scene loaded");
    }

    void AudioSystem::stopAll()
    {
        static Ecs::Query query = Ecs::make_query<AudioSourceComponent>();
        scene->GetWorld().for_each(query, [&](AudioSourceComponent& as, TransformComponent&)
        {
            if (as.IsPlaying())
                as.Stop();
        });
        LOG_INFO("scene unloaded");
    }

    void AudioSystem::onLoadScene(LoadSceneEvent* e)
    {
        // fuck u
        bool isEditor = false;
        {
            oo::GetCurrentSceneEvent e;
            oo::EventManager::Broadcast(&e);
            isEditor = e.IsEditor;
        }
        if (isEditor)
            return;

        playAllOnAwake();
    }

    void AudioSystem::onUnloadScene(UnloadSceneEvent* e)
    {
        stopAll();
    }

    void AudioSystem::onObjectEnabled(GameObjectComponent::OnEnableEvent* e)
    {
        // fuck u
        bool isEditor = false;
        {
            oo::GetCurrentSceneEvent e;
            oo::EventManager::Broadcast(&e);
            isEditor = e.IsEditor;
        }
        if (isEditor)
            return;

        playAllOnAwake();
    }

    void AudioSystem::onObjectDisabled(GameObjectComponent::OnDisableEvent* e)
    {
        // fuck u
        bool isEditor = false;
        {
            oo::GetCurrentSceneEvent e;
            oo::EventManager::Broadcast(&e);
            isEditor = e.IsEditor;
        }
        if (isEditor)
            return;

        stopAll();
    }
}
