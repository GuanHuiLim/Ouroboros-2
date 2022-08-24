/************************************************************************************//*!
\file           TransformSystem.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Aug 23, 2022
\brief          Describes the main system that will work on updating the transform
                components in the current world.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "TransformSystem.h"

#include "ouroboros/ECS/DeferredComponent.h"

#include <JobSystem/src/JobSystem.h>

namespace oo
{
    void TransformSystem::UpdateTransform(std::shared_ptr<GameObject> go, GameObjectComponent& gocomp, Transform3D& tf)
    {
        // Reset all has changed to false regardless of their previous state.
        tf.m_transform.m_hasChanged = false;

        // Update local and global transform immediately
        if (tf.IsDirty())
        {
            tf.m_transform.CalculateLocalTransform();
            //tf.m_transform.m_globalTransform = tf.m_transform.m_localTransform;
        }

        // Check for valid parent
        if (m_scene->IsValid(go->GetParentUUID()))
        {
            // Check if parent has changed locally or if hierarchy above has changed [optimization step]
            if (tf.m_transform.HasChanged() || go->GetParent().Transform().HasChanged())
            {
                tf.m_transform.m_hasChanged = true;
                tf.m_transform.m_globalTransform = go->GetParent().Transform().GetGlobalMatrix() * tf.m_transform.m_localTransform;
            }
        }
    }

    void TransformSystem::Run(Ecs::ECSWorld* world)
    {
        static constexpr const char* const transform_update = "transform_update";
        {
            TRACY_TRACK_PERFORMANCE(transform_update);
            TRACY_PROFILE_SCOPE_NC(transform_update, tracy::Color::Gold2);

            // Typical System updates

            // Option 1
            //static Ecs::Query query = []()
            //{
            //    Ecs::Query query;
            //    query.with<GameObjectComponent, Transform3D>().exclude<DeferredComponent>().build();
            //    return query;
            //}();
            //world->for_each(query, [&](GameObjectComponent& gocomp, Transform3D& tf) { /*UpdateTransform(gocomp, tf); */ });

            // Option 2
            //Ecs::Query query;
            //query.with<GameObjectComponent, Transform3D>().exclude<DeferredComponent>().build();
            //world->for_each(query, [&](GameObjectComponent& gocomp, Transform3D& tf) { UpdateTransform(gocomp, tf); });
            
            // Transform System updates via the scenegraph because the order matters

            auto& const scenegraph = m_scene->GetGraph();
            scenegraph.get_childs(scenegraph.get_root());
            for (auto& node : scenegraph.hierarchy_traversal_nodes())
            {
                // Find current gameobject
                auto const go = m_scene->FindWithInstanceID(node.get_handle());
                
                // Skip gameobjects that has the deferred component
                if (go->HasComponent<DeferredComponent>() == true)
                    continue;

                UpdateTransform(go, go->GetComponent<GameObjectComponent>(), go->Transform());
            }

            
            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_DISPLAY_PERFORMANCE_SELECTED(transform_update);
    }

}

