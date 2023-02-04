/************************************************************************************//*!
\file           JustCreatedSystem.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Aug 23, 2022
\brief          Describes a system that will remove all just created Components at the
                VERY end of frame to indicate object has finished its 
                first frame of initialization.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include "JustCreatedComponent.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObjectComponent.h"
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/ECS/ECS.h"

namespace oo
{
    class JustCreatedSystem : public Ecs::System
    {
    private:
        Scene* m_scene = nullptr;

    public:
        JustCreatedSystem(Scene* scene) : m_scene{ scene } {}

        // Removes all Just Created component from the system
        virtual void Run(Ecs::ECSWorld* world) override
        {
            TRACY_PROFILE_SCOPE_NC(just_created_component_removal, tracy::Color::Gold2);

            std::vector<oo::UUID> uuids;

            // we collect all uuids first
            // we manually build query as we want Just Created component 
            static Ecs::Query query = Ecs::make_raw_query<GameObjectComponent, JustCreatedComponent>();
            world->for_each(query, [&](GameObjectComponent& gocomp, JustCreatedComponent& justCreatedComp)
                {
                    //LOG_INFO("Should be removing deferred Component from entity {0}", gocomp.Id);
                    uuids.emplace_back(gocomp.Id);

                    //auto go = m_scene->FindWithInstanceID(justCreatedComp.uuid);
                    //go->RemoveComponent<JustCreatedComponent>();
                });

            // than we start removing 
            // NOTE : because doing so while iterating will cause issues.
            for (auto& uuid : uuids)
            {
                auto go = m_scene->FindWithInstanceID(uuid);
                go->RemoveComponent<JustCreatedComponent>();
            }

            TRACY_PROFILE_SCOPE_END();
        }
    };
}
