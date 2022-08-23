/************************************************************************************//*!
\file           DifferedSystem.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Aug 23, 2022
\brief          Describes a system that will remove all differed Components at the 
                VERY end of frame to indicate object has been removed.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Archetypes_Ecs/src/A_Ecs.h>
#include "DifferedComponent.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

namespace oo
{
    class DifferedSystem : public Ecs::System
    {
    private:
        Scene* m_scene = nullptr;

    public:
        void Link(Scene* scene) { m_scene = scene; }

        // Removes all differed component from the system
        virtual void Run(Ecs::ECSWorld* world) override
        {
            static constexpr const char* const differed_component_removal = "differed_component_removal";
            {
                TRACY_TRACK_PERFORMANCE(differed_component_removal);
                TRACY_PROFILE_SCOPE_NC(differed_component_removal, tracy::Color::Gold2);

                std::vector<UUID> uuids;

                Ecs::Query query;
                query.with<GameObjectComponent, DifferedComponent>().build();
                world->for_each(query, [&](GameObjectComponent& gocomp, DifferedComponent& differedComp)
                    {
                        LOG_INFO("Should be removing differed Component from entity {0}", gocomp.Id);
                        uuids.emplace_back(gocomp.Id);
                    });

                for (auto& uuid : uuids)
                {
                    auto go = m_scene->FindWithInstanceID(uuid);
                    go->RemoveComponent<DifferedComponent>();
                }

                TRACY_PROFILE_SCOPE_END();
            }

            TRACY_DISPLAY_PERFORMANCE_SELECTED(differed_component_removal);
        }
    };
}
