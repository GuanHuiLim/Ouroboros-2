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

#include <JobSystem/src/final/jobs.h>

#include "Ouroboros/EventSystem/EventManager.h"

namespace oo
{
    TransformSystem::TransformSystem(Scene* scene)
        : m_scene{ scene }
    {
        EventManager::Subscribe<TransformSystem, GameObjectComponent::OnEnableEvent>(this, &TransformSystem::OnEnableGameObject);
        //EventManager::Subscribe<TransformSystem, GameObjectComponent::OnDisableEvent>(this, &TransformSystem::OnDisableGameObject);
    }

    TransformSystem::~TransformSystem()
    {
        EventManager::Unsubscribe<TransformSystem, GameObjectComponent::OnEnableEvent>(this, &TransformSystem::OnEnableGameObject);
        //EventManager::Unsubscribe<TransformSystem, GameObjectComponent::OnDisableEvent>(this, &TransformSystem::OnDisableGameObject);
    }

    void TransformSystem::PostLoadSceneInit()
    {
        //StartOfFramePreprocessing();
        UpdateEntireTree();
    }

    void TransformSystem::Run(Ecs::ECSWorld* world)
    {
        TRACY_PROFILE_SCOPE_NC(transform_main_update, tracy::Color::Gold2);
        OPTICK_EVENT();

        // Typical System updates using query//
        /*
        Remember to #include "Ouroboros/ECS/ECS.h" for this wrapper functionality

        Option 1 : Makes with Deferred Component auto excluded
            static Ecs::Query query = Ecs::make_query<GameObjectComponent>();
            world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { // do function here});

        Option 2 : Makes with Deferred Component excluded
            static Ecs::Query query = Ecs::make_raw_query<GameObjectComponent>();
            world->for_each(query, [&](GameObjectComponent& gocomp, TransformComponent& tf) { // do function here });

        NOTE: this might be extended in the future to include specific components or have
        various combinations. But as much as possible make_query should work for the most part.
        */

        if (m_firstFrame)
        {
            m_firstFrame = false;
        }
        else
        {
            // Reset all has changed to false regardless of their previous state.
            // Note: this should only occure once per frame. Otherwise wonky behaviour.
            static Ecs::Query query = Ecs::make_query<TransformComponent>();
            world->for_each(query, [&](TransformComponent& tf)
            { 
                tf.HasChangedThisFrame = false; 
            });
        }

        UpdateEntireTree();

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateSubTree(GameObject go, bool includeItself)
    {
        TRACY_PROFILE_SCOPE_NC(transform_subtree_update, tracy::Color::Gold3);
        OPTICK_EVENT();

        UpdateLocalTransform(go.GetComponent<TransformComponent>());
        UpdateTree(go.GetSceneNode().lock(), includeItself);
        
        //UpdateEntireTree();

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateEntireTree()
    {
        TRACY_PROFILE_SCOPE_NC(transform_update_entire_tree, tracy::Color::Gold3);
        OPTICK_EVENT();

        GenerateLaunchGroups();

        UpdateLocalTransforms();

        UpdateLaunchGroups();

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateLocalTransform(TransformComponent& tf)
    {
        TRACY_PROFILE_SCOPE_NC(transform_single_local_update, tracy::Color::Gold4);
        OPTICK_EVENT();

        if (tf.LocalMatrixDirty)
        {
            //std::string debugName = goc.Name;
            tf.CalculateLocalTransform();
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateLocalTransforms()
    {
        TRACY_PROFILE_SCOPE_NC(transform_local_transforms_update, tracy::Color::Gold4);
        OPTICK_EVENT();

        // Update their local transform
        static Ecs::Query query = Ecs::make_raw_query</*GameObjectComponent, */TransformComponent>();
        m_world->parallel_for_each(query, [&](/*GameObjectComponent& goc,*/ TransformComponent& tf)
            {
                // TODO: this part of the code doesn't need to be serial.
                // Update local and global transform immediately
                if (tf.LocalMatrixDirty)
                {
                    //std::string debugName = goc.Name;
                    tf.CalculateLocalTransform();
                }
            });

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateLaunchGroups()
    {
        // Transform System updates via the scenegraph because the order matters

        TRACY_PROFILE_SCOPE_NC(transform_update_launch_groups, tracy::Color::Gold3);
        OPTICK_EVENT();

        // Step 2. processing.
        for (auto& group : launch_groups)
        {
            if (group.size() <= 0)
                continue;

            TRACY_PROFILE_SCOPE_NC(per_batch_processing, tracy::Color::Goldenrod);
            OPTICK_EVENT("per_batch_processing");

            std::for_each(std::execution::par_unseq, std::begin(group), std::end(group), [&](auto const& elem)
                {
                    // Find current gameobject
                    auto const go = m_scene->FindWithInstanceID(elem->get_handle());
                    UpdateTransform(go);
                });


            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_PROFILE_SCOPE_END();
    }


    void TransformSystem::UpdateTree(scenenode::shared_pointer node, bool updateRoot)
    {
        // Transform System updates via the scenegraph because the order matters

        TRACY_PROFILE_SCOPE_NC(transform_update_tree, tracy::Color::Gold3);
        OPTICK_EVENT();

        //UpdateEntireTree();

        scenegraph::shared_pointer root_node = node;
        std::stack<scenenode::shared_pointer> s;
        scenenode::shared_pointer curr = root_node; 
        std::array<std::vector<scenegraph::shared_pointer>, MaxDepth> launch_groups;

        // update itself or not
        if (updateRoot)
        {
            // Find root gameobject
            auto const go = m_scene->FindWithInstanceID(node->get_handle());
            UpdateTransform(go);
        }

        /* Multithread Method of updating */
        
        // Step 1. Extra Pre-Processing Overhead
        {
            TRACY_PROFILE_SCOPE_NC(pre_process_overhead, tracy::Color::Gold3);
            OPTICK_EVENT("pre_process_overhead");
            s.emplace(curr);
            while (!s.empty())
            {
                curr = s.top();
                s.pop();

                {
                    TRACY_PROFILE_SCOPE_NC(pre_process_inner_for_loop, tracy::Color::Gold4);
                    OPTICK_EVENT("pre_process_inner_for_loop");
                    for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
                    {
                        scenenode::shared_pointer child = *iter;
                        if (child->has_child())
                            s.emplace(child);
                    }

                    TRACY_PROFILE_SCOPE_END();
                }

                auto childs = curr->get_direct_child();
                auto child_depth = curr->get_depth() + 1;
                launch_groups[child_depth].insert(launch_groups[child_depth].end(), childs.begin(), childs.end());
                assert(child_depth != 0);
            }

            TRACY_PROFILE_SCOPE_END();
        }

        // Step 2. processing.
        for (auto& group : launch_groups)
        {
            if (group.size() <= 0) 
                continue;

            TRACY_PROFILE_SCOPE_NC(per_batch_processing, tracy::Color::Goldenrod);
            OPTICK_EVENT("per_batch_processing");

            std::for_each(std::execution::seq, std::begin(group), std::end(group), [&](auto const& elem)
                {
                    // Find current gameobject
                    auto const go = m_scene->FindWithInstanceID(elem->get_handle());
                    UpdateTransform(go);
                });
            

            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::UpdateTransform(std::shared_ptr<GameObject> const& go)
    {
        TRACY_PROFILE_SCOPE_NC(per_transform_update, tracy::Color::Gold4);
        OPTICK_EVENT();

        // Check for valid parent
        {
            TRACY_PROFILE_SCOPE_NC(transform_assert_check_duration, tracy::Color::Gold4);
            OPTICK_EVENT("transform_assert_check_duration");
            ASSERT_MSG(m_scene->IsValid(go->GetParentUUID()) == false, "Assumes we always have proper parent");
            TRACY_PROFILE_SCOPE_END();
        }

        auto& tf = go->Transform();
        auto& parentTf = m_scene->FindWithInstanceID(go->GetParentUUID())->Transform();

        /// all parents need to be sure to be updated first.
        if (tf.GlobalMatrixDirty)
        {
            TRACY_PROFILE_SCOPE_NC(update_global_matrix, tracy::Color::Gold4);
            OPTICK_EVENT("update_global_matrix");
            
            tf.CalculateGlobalTransform();
            glm::mat4 parentGlobal = parentTf.GlobalTransform;
            glm::mat4 parent_inverse = glm::affineInverse(parentGlobal) * tf.GlobalTransform.Matrix;
            tf.SetLocalTransform(parent_inverse);   // decompose to this values.
            tf.LocalEulerAngles = glm::degrees(quaternion::to_euler(tf.LocalTransform.Orientation));// update fake values outside!
            
            TRACY_PROFILE_SCOPE_END();
        }
        // Check if transform has changed locally or if parent has changed [optimization step]
        else if (tf.HasChangedThisFrame || parentTf.HasChangedThisFrame)
        {
            TRACY_PROFILE_SCOPE_NC(update_transform_global_values, tracy::Color::Gold4);
            OPTICK_EVENT("update_transform_global_values");
                
            tf.HasChangedThisFrame = true;
            tf.GlobalTransform.Matrix = parentTf.GlobalTransform.Matrix * tf.LocalTransform.Matrix;
            Transform3D::DecomposeValues(tf.GlobalTransform.Matrix, tf.GlobalTransform.Position, tf.GlobalTransform.Orientation.value, tf.GlobalTransform.Scale);
                
            TRACY_PROFILE_SCOPE_END();
        }
        
        TRACY_PROFILE_SCOPE_END();
    }

    void TransformSystem::OnEnableGameObject(GameObjectComponent::OnEnableEvent* e)
    { 
        // check for lights
        auto go = m_scene->FindWithInstanceID(e->Id);
        // assumption: Everything should have transform!
        go->Transform().LocalMatrixDirty = true;
        //auto& tf = go->Transform().LocalMatrixDirty = true;
        //tf.SetPosition(tf.GetPosition());
    }

    void TransformSystem::GenerateLaunchGroups()
    {
        TRACY_PROFILE_SCOPE_NC(transform_determine_dirty, tracy::Color::Gold3);
        OPTICK_PUSH("transform_determine_dirty");
        // gather a set of unique ids that are currently dirty rn.
        std::set<scenenode::handle_type> dirtyIDs{};
        static Ecs::Query query = Ecs::make_raw_query<GameObjectComponent, TransformComponent>();
        m_world->for_each(query, [&](GameObjectComponent& goc, TransformComponent& tf)
        {
            auto node = goc.Node.lock();
            if (tf.LocalMatrixDirty || tf.GlobalMatrixDirty)
            {
                dirtyIDs.emplace(node->get_handle());
            }
        });
        OPTICK_POP();
        TRACY_PROFILE_SCOPE_END();

        TRACY_PROFILE_SCOPE_NC(transform_assemble_launch_groups, tracy::Color::Gold3);
        OPTICK_PUSH("transform_assemble_launch_groups");
        // reset groups
        for (auto& group : launch_groups)
            group.clear();

        // go through scene graph and determine what needs to be launched and updated
        auto const& graph = m_scene->GetGraph();
        scenegraph::shared_pointer root_node = graph.get_root();
        scenenode::shared_pointer curr = root_node;

        std::stack<std::pair<scenegraph::shared_pointer, int>> stk{};
        auto firstLevelChilds = curr->get_direct_child();
        std::for_each(firstLevelChilds.begin(), firstLevelChilds.end(), [&](auto&& child)
        {
            stk.push({ child, 0 });
        });

        while (!stk.empty())
        {
            auto& [node, hierarchyLevel] = stk.top();
            stk.pop();
            
            bool isDirty = dirtyIDs.contains(node->get_handle());
            if (hierarchyLevel == 0 && isDirty)
            {
                launch_groups[hierarchyLevel].emplace_back(node);
            }

            auto childs = node->get_direct_child();
            auto childLevel = hierarchyLevel;

            if (hierarchyLevel > 0 || isDirty)
            {
                childLevel += 1;
                launch_groups[childLevel].insert(launch_groups[childLevel].end(), childs.begin(), childs.end());
            }
            
            std::for_each(childs.begin(), childs.end(), [&](auto&& child)
            {
                stk.push({ child, childLevel });
            });

        }
        OPTICK_POP();
        TRACY_PROFILE_SCOPE_END();
    }

    //void TransformSystem::StartOfFramePreprocessing()
    //{
    //    TRACY_PROFILE_SCOPE_NC(start_of_frame_preprocessing, tracy::Color::Gold4);

    //    auto const& graph = m_scene->GetGraph();
    //    scenegraph::shared_pointer root_node = graph.get_root();
    //    std::stack<scenenode::shared_pointer> s;
    //    scenenode::shared_pointer curr = root_node;

    //    // Step 1. Extra Pre-Processing Overhead
    //    {
    //        TRACY_PROFILE_SCOPE_NC(pre_process_overhead, tracy::Color::Gold3);

    //        for (auto& group : launch_groups)
    //            group.clear();

    //        s.emplace(curr);
    //        while (!s.empty())
    //        {
    //            curr = s.top();
    //            s.pop();

    //            {
    //                TRACY_PROFILE_SCOPE_NC(pre_process_inner_for_loop, tracy::Color::Gold4);
    //                /*std::for_each(std::execution::par, curr->rbegin(), curr->rend(), [&](auto const& elem)
    //                    {
    //                        scenenode::shared_pointer child = elem;
    //                        if (child->has_child())
    //                            s.emplace(child);
    //                    });*/
    //                for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
    //                {
    //                    scenenode::shared_pointer child = *iter;
    //                    if (child->has_child())
    //                        s.emplace(child);
    //                }

    //                TRACY_PROFILE_SCOPE_END();
    //            }

    //            auto childs = curr->get_direct_child();
    //            auto child_depth = curr->get_depth() + 1;
    //            //auto current_size = launch_groups[child_depth].size();
    //            //launch_groups[child_depth].resize(current_size + childs.size());
    //            //std::move(std::execution::par_unseq, std::begin(childs), std::end(childs), std::begin(launch_groups[child_depth]) + current_size);
    //            launch_groups[child_depth].insert(launch_groups[child_depth].end(), childs.begin(), childs.end());
    //            assert(child_depth != 0);
    //        }

    //        TRACY_PROFILE_SCOPE_END();
    //    }

    //    TRACY_PROFILE_SCOPE_END();
    //}

    /*void TransformSystem::OnDisableGameObject(GameObjectComponent::OnDisableEvent* e)
    {

    }*/

}

