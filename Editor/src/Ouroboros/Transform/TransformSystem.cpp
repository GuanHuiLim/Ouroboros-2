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
    void TransformSystem::UpdateTransform(std::shared_ptr<GameObject> const& go, TransformComponent& tf)
    {
        static constexpr const char* const per_transform_update = "per_transform_update";
        TRACY_TRACK_PERFORMANCE(per_transform_update);
        TRACY_PROFILE_SCOPE_NC(per_transform_update, tracy::Color::Gold4);

        // Reset all has changed to false regardless of their previous state.
        tf.m_transform.m_hasChanged = false;

        // Update local and global transform immediately
        if (tf.IsDirty())
        {
            tf.m_transform.CalculateLocalTransform();
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

        TRACY_PROFILE_SCOPE_END();
        TRACY_DISPLAY_PERFORMANCE_SELECTED(per_transform_update);
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
            //    query.with<GameObjectComponent, Transform3D>().include<DeferredComponent>().build();
            //    return query;
            //}();
            //world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { /*UpdateTransform(gocomp, tf); */ });

            // Option 2
            //Ecs::Query query;
            //query.with<GameObjectComponent, TransformComponent>().exclude<DeferredComponent>().build();
            //world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { UpdateTransform(gocomp, tf); });
            
            // Transform System updates via the scenegraph because the order matters
            {
                static constexpr const char* const pre_transform_collect = "pre_transform_collect";
                TRACY_TRACK_PERFORMANCE(pre_transform_collect);
                TRACY_PROFILE_SCOPE_NC(pre_transform_collect, tracy::Color::Gold3);

                //std::vector<Scene::go_ptr> objects_to_update;
                auto const&  graph = m_scene->GetGraph();
                //graph.get_childs(graph.get_root());

                scenegraph::shared_pointer root_node = graph.get_root();
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


                //for (auto& handle : scenegraph.hierarchy_traversal_handles())
                //{
                //    // Find current gameobject
                //    auto const go = m_scene->FindWithInstanceID(handle);
                //
                //    // Skip gameobjects that has the deferred component
                //    if (go->HasComponent<DeferredComponent>())
                //        continue;

                //    //objects_to_update.emplace_back(go);
                //    UpdateTransform(go, go->Transform());
                //}

                TRACY_PROFILE_SCOPE_END();
                TRACY_DISPLAY_PERFORMANCE_SELECTED(pre_transform_collect);

                /*for (auto& go : objects_to_update)
                {
                    static constexpr const char* const get_component = "get_component";
                    TRACY_TRACK_PERFORMANCE(get_component);
                    TRACY_PROFILE_SCOPE_NC(get_component, tracy::Color::Gold4);
                    GameObjectComponent& gocomp = go->GetComponent<GameObjectComponent>();
                    TRACY_PROFILE_SCOPE_END();
                    TRACY_DISPLAY_PERFORMANCE_SELECTED(get_component);

                    static constexpr const char* const get_transform = "get_transform";
                    TRACY_TRACK_PERFORMANCE(get_transform);
                    TRACY_PROFILE_SCOPE_NC(get_transform, tracy::Color::Gold4);
                    TransformComponent& tf = go->Transform();
                    TRACY_PROFILE_SCOPE_END();
                    TRACY_DISPLAY_PERFORMANCE_SELECTED(get_transform);
                    
                    static constexpr const char* const per_transform_update = "per_transform_update";
                    TRACY_TRACK_PERFORMANCE(per_transform_update);
                    TRACY_PROFILE_SCOPE_NC(per_transform_update, tracy::Color::Gold4);
                    UpdateTransform(go, gocomp, tf);
                    TRACY_PROFILE_SCOPE_END();
                    TRACY_DISPLAY_PERFORMANCE_SELECTED(per_transform_update);
                }*/
            }
            
            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_DISPLAY_PERFORMANCE_SELECTED(transform_update);
    }

}

