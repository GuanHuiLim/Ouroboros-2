/************************************************************************************//*!
\file           TransformSystem.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420
\par            email: c.tecklee\@digipen.edu
\date           Jun 31, 2022
\brief          Describes the main system that will work on updating the transform
                components in the current world.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Transform/TransformComponent.h>

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>

namespace oo
{
    class TransformSystem final : public Ecs::System
    {
    private:
        Scene* m_scene = nullptr;

    public:
        
        //TransformSystem(Scene* scene) : m_scene{ scene } {};
        TransformSystem() = default;
        virtual ~TransformSystem() = default;
        
        void Link(Scene* scene) { m_scene = scene; }

        void UpdateAllTransforms()
        {
            /*std::shared_ptr<oo::GameObject> rootGo = m_activeScene.GetRoot();
            rootGo->GetChildrenUUID();*/
            
            //for (auto& editorComp : ecs)
            //{
            //    //work on the components directly.
            //}
            
            // 2 graphs
            // current graph    - that is used for reading
            // next graph       - that should be edited

            //readonlygraph = m_activeScene.GetGraph();

            //m_activeScene.reinsert_stuff(target, goal);

            //readonlygraph = newgraph;
            
            /*Ecs::Query some_query;
            some_query.with<Transform3D>().build();
            
            scenegraph sg = m_activeScene.GetGraph();
            
            auto root_node = sg.get_root();
            std::stack<scenenode::const_raw_pointer> s;
            std::vector<scenenode const> nodes;
            std::vector<scenegraph::handle_type const> handles;
            scenenode::const_raw_pointer curr = root_node.get();
            while (!s.empty())
            {
                s.pop();
                for(auto iter = curr->get_direct_child().rbegin(); iter != curr->get_direct_child().rend(); ++iter)
                {
                    scenenode::shared_pointer child = *iter;
                    s.emplace(child.get());
                    nodes.emplace_back(child);
                    handles.emplace_back(child->get_handle());
                }
                curr = s.top();
            }*/

            ////root_node->add_child();
            //auto target = root_node->get_direct_child()[9]; //9 scene.h
            //auto goal   = root_node->get_direct_child()[0]; //0 editorcontroller.cpp
            //root_node->reinsert(target, goal);
            //
            //for (auto& child : root_node->get_direct_child())
            //{
            //    child->get_direct_child()[0];
            //}

            //std::shared_ptr<oo::GameObject> rootGo = m_activeScene.FindWithInstanceID(root_node->get_handle());
            //rootGo->GetComponent<Transform3D>();
            
        }

        void UpdateTransform(GameObjectComponent& gocomp, Transform3D& tf)
        {
            // Reset all has changed to false regardless of their previous state.
            tf.m_transform.m_hasChanged = false;

            // Update local and global transform immediately
            if (tf.IsDirty())
            {
                tf.m_transform.CalculateLocalTransform();
                tf.m_transform.m_globalTransform = tf.m_transform.m_localTransform;
            }
            
            // Find current gameobject
            std::shared_ptr<GameObject> go = m_scene->FindWithInstanceID(gocomp.Id);
            
            // Check for valid parent
            if (m_scene->IsValid(go->GetParentUUID()))
            {
                // Check if parent has changed locally or if hierarchy above has changed [optimization step]
                if (go->GetParent().Transform().HasChanged())
                {
                    tf.m_transform.m_hasChanged = true;
                    tf.m_transform.m_globalTransform = go->GetParent().Transform().GetGlobalMatrix() * tf.m_transform.m_localTransform;
                }
            }
        }

        virtual void Run(Ecs::ECSWorld* world)
        {
            constexpr const char* const transform_update = "transform_update";
            {
                ZoneScopedNC(transform_update, tracy::Color::Gold2);
                TRACY_TRACK_PERFORMANCE(transform_update);

                Ecs::Query query;
                query.with<GameObjectComponent, Transform3D>().build();
                world->for_each(query, [&] (GameObjectComponent gocomp, Transform3D& tf) { UpdateTransform(gocomp,tf); });
            }

            TRACY_DISPLAY_PERFORMANCE_SELECTED(transform_update);
        }

    };
}
