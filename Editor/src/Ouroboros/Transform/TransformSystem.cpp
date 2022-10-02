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

#include <Ouroboros/Scene/Scene.h>
#include "ouroboros/ECS/DeferredComponent.h"
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/ECS/ECS.h"
#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>
//#include <JobSystem/src/JobSystem.h>

namespace oo
{
    TransformSystem::TransformSystem(Scene* scene)
        : m_scene{ scene }
    {
    }

    void TransformSystem::Run(Ecs::ECSWorld* world)
    {
        TRACY_PROFILE_SCOPE_NC(transform_main_update, tracy::Color::Gold2);

        // Typical System updates using query//
        /*
        Remember to #include "Ouroboros/ECS/ECS.h" for this wrapper functionality

        Option 1 : Makes with Deferred Component auto excluded
            static Ecs::Query query = Ecs::make_query<GameObjectComponent>();
            world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { // do function here});

        Option 2 : Makes with Deferred Component excluded
            static Ecs::Query query = Ecs::make_query_including_deferred<GameObjectComponent>();
            world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { // do function here });

        NOTE: this might be extended in the future to include specific components or have
        various combinations. But as much as possible make_query should work for the most part.
        */

        // Reset all has changed to false regardless of their previous state.
        // Note: this should only occure once per frame. Otherwise wonky behaviour.
        static Ecs::Query query = Ecs::make_query<TransformComponent>();
        world->for_each(query, [&](TransformComponent& tf) { tf.HasChangedThisFrame = false; });

        // TODO
        // update local transformations : can be parallelized.
        UpdateLocalTransforms();

        // Transform System updates via the scenegraph because the order matters
        auto const& graph = m_scene->GetGraph();
        scenegraph::shared_pointer root_node = graph.get_root();
        UpdateTree(root_node, false);

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateSubTree(GameObject go, bool includeItself)
    {
        TRACY_PROFILE_SCOPE_NC(transform_subtree_update, tracy::Color::Gold3);

        UpdateLocalTransforms();
        UpdateTree(go.GetSceneNode().lock(), includeItself);
        
        TRACY_PROFILE_SCOPE_END();
    }


    void TransformSystem::UpdateLocalTransforms()
    {
        TRACY_PROFILE_SCOPE_NC(transform_local_transform_update, tracy::Color::Gold4);

        // Update their local transform
        static Ecs::Query query = Ecs::make_query_including_deferred<TransformComponent>();
        m_world->for_each(query, [&](TransformComponent& tf)
            {
                // TODO: this part of the code doesn't need to be serial.
                // Update local and global transform immediately
                if (tf.LocalMatrixDirty)
                {
                    tf.CalculateLocalTransform();
                }
            });

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateTree(scenenode::shared_pointer node, bool updateRoot)
    {
        // Transform System updates via the scenegraph because the order matters
        
        TRACY_PROFILE_SCOPE_NC(pre_transform_collect, tracy::Color::Gold3);

        scenegraph::shared_pointer root_node = node;
        std::stack<scenenode::shared_pointer> s;
        scenenode::shared_pointer curr = root_node;

        // update itself or not
        if (updateRoot)
        {
            // Find root gameobject
            auto const go = m_scene->FindWithInstanceID(node->get_handle());

            UpdateTransform(go, go->Transform());
            //// Skip gameobjects that has the deferred component
            //if (go->HasComponent<DeferredComponent>() == false)
            //{
            //}
        }

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

                //// Skip gameobjects that has the deferred component
                //if (go->HasComponent<DeferredComponent>())
                //    continue;

                UpdateTransform(go, go->Transform());
            }
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateTransform(std::shared_ptr<GameObject> const& go, TransformComponent& tf)
    {
        TRACY_PROFILE_SCOPE_NC(per_transform_update, tracy::Color::Gold4);
        
        /// all parents need to be sure to be updated first.
        if (tf.GlobalMatrixDirty)
        {
            tf.CalculateGlobalTransform();
            glm::mat4 parentGlobal = go->GetParent().Transform().GlobalTransform;
            glm::mat4 parent_inverse = glm::affineInverse(parentGlobal) * tf.GlobalTransform.Matrix;
            tf.SetLocalTransform(parent_inverse);   // decompose to this values.
            tf.LocalEulerAngles = glm::degrees(quaternion::to_euler(tf.LocalTransform.Orientation));// update fake values outside!
        }
        else 
        {
            ASSERT_MSG(m_scene->IsValid(go->GetParentUUID()) == false, "Assumes we always have proper parent");

            // Check for valid parent
            auto& parentTf = go->GetParent().Transform();
            // Check if transform has changed locally or if parent has changed [optimization step]
            if (tf.HasChangedThisFrame || parentTf.HasChangedThisFrame)
            {
                tf.HasChangedThisFrame = true;
                tf.GlobalTransform.Matrix = go->GetParent().Transform().GlobalTransform.Matrix * tf.LocalTransform.Matrix;
                Transform3D::DecomposeValues(tf.GlobalTransform.Matrix, tf.GlobalTransform.Position, tf.GlobalTransform.Orientation.value, tf.GlobalTransform.Scale);
            }
        }

        TRACY_PROFILE_SCOPE_END();
    }


}

