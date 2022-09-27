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

//#include <JobSystem/src/JobSystem.h>

#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/ECS/ECS.h"

namespace oo
{
    void TransformSystem::UpdateTransform(std::shared_ptr<GameObject> const& go, TransformComponent& tf)
    {
        static constexpr const char* const per_transform_update = "per_transform_update";
        TRACY_TRACK_PERFORMANCE(per_transform_update);
        TRACY_PROFILE_SCOPE_NC(per_transform_update, tracy::Color::Gold4);

        // Reset all has changed to false regardless of their previous state.
        tf.m_hasChanged = false;

        // Update local and global transform immediately
        if (tf.IsDirty())
        {
            tf.CalculateLocalTransform();
        }

        ASSERT_MSG(m_scene->IsValid(go->GetParentUUID()) == false,"Assumes we always have proper parent");

        auto& parentTf = go->GetParent().Transform();
        // Check if transform has changed locally or if parent has changed [optimization step]
        if (tf.HasChanged() || parentTf.HasChanged())
        {
            tf.SetGlobalTransform(parentTf.GetGlobalMatrix() * tf.GetLocalMatrix());
        }

        TRACY_PROFILE_SCOPE_END();
        TRACY_DISPLAY_PERFORMANCE_SELECTED(per_transform_update);
    }

    void TransformSystem::UpdateTree(scenenode::shared_pointer node)
    {
        // Transform System updates via the scenegraph because the order matters
        static constexpr const char* const pre_transform_collect = "tree_update";
        TRACY_TRACK_PERFORMANCE(pre_transform_collect);
        TRACY_PROFILE_SCOPE_NC(pre_transform_collect, tracy::Color::Gold3);

        scenegraph::shared_pointer root_node = node;
        std::stack<scenenode::shared_pointer> s;
        scenenode::shared_pointer curr = root_node;
        s.push(curr);
        while (!s.empty())
        {
            curr = s.top();
            s.pop();
            for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
            {
                scenenode::shared_pointer child = *iter;
                s.push(child);

                // Find current gameobject
                auto const go = m_scene->FindWithInstanceID(child->get_handle());

                // Skip gameobjects that has the deferred component
                if (go->HasComponent<DeferredComponent>())
                    continue;

                UpdateTransform(go, go->Transform());
            }
        }

        TRACY_PROFILE_SCOPE_END();
        TRACY_DISPLAY_PERFORMANCE_SELECTED(pre_transform_collect);
    }

    void TransformSystem::Run(Ecs::ECSWorld* world)
    {
        static constexpr const char* const transform_update = "transform_update";
        TRACY_TRACK_PERFORMANCE(transform_update);
        TRACY_PROFILE_SCOPE_NC(transform_update, tracy::Color::Gold2);

        // Typical System updates using query//
        /* 
        Remember to #include "Ouroboros/ECS/ECS.h" for this wrapper functionality

        Option 1 : Makes with Deferred Component auto excluded
            static Ecs::Query query = Ecs::make_query<GameObjectComponent>();
            world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { // do function here});
        
        Option 2 : Makes with Deferred Component excluded
            static Ecs::Query query = Ecs::make_query_including_differed<GameObjectComponent>();
            world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { // do function here });

        NOTE: this might be extended in the future to include specific components or have 
        various combinations. But as much as possible make_query should work for the most part.
        */

        // Transform System updates via the scenegraph because the order matters
        auto const&  graph = m_scene->GetGraph();
        scenegraph::shared_pointer root_node = graph.get_root();
        UpdateTree(root_node);

        TRACY_PROFILE_SCOPE_END();
        TRACY_DISPLAY_PERFORMANCE_SELECTED(transform_update);
    }

    void TransformSystem::UpdateSubTree(GameObject go)
    {
        UpdateTree(go.GetSceneNode().lock());
    }

}

