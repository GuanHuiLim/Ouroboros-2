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
                if (!as.GetChannel())
                    return;

                // Set 3D position
                auto tfPos = tf.GetGlobalPosition();
                FMOD_VECTOR fmPos = { .x = tfPos.x, .y = tfPos.y, .z = tfPos.z };
                as.GetChannel()->set3DAttributes(&fmPos, nullptr);

                // Check dirty flag
                if (as.IsDirty())
                {
                    // Update all
                    FMOD_ERR_HAND(as.GetChannel()->setMute(as.IsMuted()));
                    FMOD_ERR_HAND(as.GetChannel()->setMute(as.IsMuted()));
                    FMOD_ERR_HAND(as.GetChannel()->setLoopCount(as.IsLoop() ? -1 : 0));
                    FMOD_ERR_HAND(as.GetChannel()->setVolume(as.GetVolume()));
                    FMOD_ERR_HAND(as.GetChannel()->setPitch(as.GetPitch()));
                    as.ClearDirty();
                }
            });
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void AudioSystem::playAllOnAwake()
    {
        if (!scene)
            return;
        static Ecs::Query query = Ecs::make_query<AudioSourceComponent>();
        scene->GetWorld().for_each(query, [&](AudioSourceComponent& as, TransformComponent&)
        {
            if (as.IsPlayOnAwake())
                as.Play();
        });
    }

    void AudioSystem::stopAll()
    {
        if (!scene)
            return;
        static Ecs::Query query = Ecs::make_query<AudioSourceComponent>();
        scene->GetWorld().for_each(query, [&](AudioSourceComponent& as, TransformComponent&)
        {
            if (as.IsPlaying())
                as.Stop();
        });
    }

    void AudioSystem::onLoadScene(LoadSceneEvent* e)
    {
        bool isEditor = false;
        {
            oo::GetCurrentSceneEvent ev;
            oo::EventManager::Broadcast(&ev);
            isEditor = ev.IsEditor;
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
        if (!scene)
            return;

        bool isEditor = false;
        {
            oo::GetCurrentSceneEvent ev;
            oo::EventManager::Broadcast(&ev);
            isEditor = ev.IsEditor;
        }
        if (isEditor)
            return;

        auto go = scene->FindWithInstanceID(e->Id);
        if (go == nullptr)
            return;

        if (!go->HasComponent<AudioSourceComponent>())
            return;

        if (go->GetComponent<AudioSourceComponent>().IsPlayOnAwake())
            go->GetComponent<AudioSourceComponent>().Play();
    }

    void AudioSystem::onObjectDisabled(GameObjectComponent::OnDisableEvent* e)
    {
        if (!scene)
            return;

        bool isEditor = false;
        {
            oo::GetCurrentSceneEvent ev;
            oo::EventManager::Broadcast(&ev);
            isEditor = ev.IsEditor;
        }
        if (isEditor)
            return;

        auto go = scene->FindWithInstanceID(e->Id);
        if (go == nullptr)
            return;

        if (!go->HasComponent<AudioSourceComponent>())
            return;

        go->GetComponent<AudioSourceComponent>().Stop();
    }
}
