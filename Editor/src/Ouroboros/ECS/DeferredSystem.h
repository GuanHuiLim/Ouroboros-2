/************************************************************************************//*!
\file           DeferredComponent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Aug 23, 2022
\brief          Describes a system that will remove all deferred Components at the 
                VERY end of frame to indicate object has been removed.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Archetypes_Ecs/src/A_Ecs.h>
#include "DeferredComponent.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObjectComponent.h"
#include "Ouroboros/ECS/GameObject.h"

namespace oo
{
    class DeferredSystem : public Ecs::System
    {
    private:
        Scene* m_scene = nullptr;

    public:
        void Link(Scene* scene) { m_scene = scene; }

        // Removes all deferred component from the system
        virtual void Run(Ecs::ECSWorld* world) override
        {
            static constexpr const char* const deferred_component_removal = "deferred_component_removal";
            {
                TRACY_TRACK_PERFORMANCE(deferred_component_removal);
                TRACY_PROFILE_SCOPE_NC(deferred_component_removal, tracy::Color::Gold2);

                std::vector<UUID> uuids;

                Ecs::Query query;
                query.with<GameObjectComponent, DeferredComponent>().build();
                world->for_each(query, [&](GameObjectComponent& gocomp, DeferredComponent& deferredComp)
                    {
                        LOG_INFO("Should be removing deferred Component from entity {0}", gocomp.Id);
                        uuids.emplace_back(gocomp.Id);
                    });

                for (auto& uuid : uuids)
                {
                    auto go = m_scene->FindWithInstanceID(uuid);
                    go->RemoveComponent<DeferredComponent>();
                }

                TRACY_PROFILE_SCOPE_END();
            }

            TRACY_DISPLAY_PERFORMANCE_SELECTED(deferred_component_removal);
        }
    };
}
