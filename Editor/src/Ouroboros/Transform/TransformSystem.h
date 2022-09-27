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
#pragma once

#include <Ouroboros/Scene/Scene.h>
#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Transform/TransformComponent.h>

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>

namespace oo
{
    class TransformSystem final : public Ecs::System
    {
    public:
        
        TransformSystem(Scene* scene) 
            : m_scene{ scene } 
        {
        }

        virtual ~TransformSystem() = default;

        virtual void Run(Ecs::ECSWorld* world) override;

        void UpdateSubTree(GameObject go);

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
            some_query.with<TransformComponent>().build();
            
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
            //rootGo->GetComponent<TransformComponent>();
            
        }

    private:
        void UpdateTransform(std::shared_ptr<GameObject> const& go, TransformComponent& tf);

        void UpdateLocalTransforms();
        void UpdateTree(scenenode::shared_pointer node, bool updateRoot);
    private:
        Scene* m_scene = nullptr;

    };
}
