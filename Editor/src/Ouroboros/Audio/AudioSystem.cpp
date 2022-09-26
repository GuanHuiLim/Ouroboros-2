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

#include "Ouroboros/ECS/ECS.h"

namespace oo
{
    void AudioSystem::Run(Ecs::ECSWorld* world)
    {
        static constexpr const char* const AUDIO_UPDATE = "audio_update";
        TRACY_TRACK_PERFORMANCE(AUDIO_UPDATE);
        TRACY_PROFILE_SCOPE_NC(AUDIO_UPDATE, tracy::Color::Aquamarine1);

        // Iterate audio sources
        static Ecs::Query query = Ecs::make_query<AudioSourceComponent>();
        world->for_each(query, [&](AudioSourceComponent& comp, TransformComponent& tf)
        {
            // TODO: update
        });

        TRACY_PROFILE_SCOPE_END();
        TRACY_DISPLAY_PERFORMANCE_SELECTED(AUDIO_UPDATE);
    }
}
