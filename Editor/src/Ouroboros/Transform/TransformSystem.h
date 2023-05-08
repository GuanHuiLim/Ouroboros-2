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

#include <Ouroboros/ECS/GameObject.h>
#include <Ouroboros/Transform/TransformComponent.h>

namespace oo
{
    class Scene;

    class TransformSystem final : public Ecs::System
    {
    public:
        
        TransformSystem(Scene* scene);
        virtual ~TransformSystem();

        void PostLoadSceneInit();

        virtual void Run(Ecs::ECSWorld* world) override;
        void UpdateSubTree(GameObject go, bool includeItself = true);
        void UpdateEntireTree();

        /*void StartOfFrame();
        void EndOfFrame();*/

    private:
        void UpdateLocalTransform(TransformComponent& tf);
        void UpdateLocalTransforms();
        void UpdateRootTree();  // optimized version without pre processing.
        void UpdateTree(scenenode::shared_pointer node, bool updateRoot);
        void UpdateTransform(std::shared_ptr<GameObject> const& go);

        void OnEnableGameObject(GameObjectComponent::OnEnableEvent* e);
        //void OnDisableGameObject(GameObjectComponent::OnDisableEvent* e);

        void StartOfFramePreprocessing();

    private:
        Scene* m_scene = nullptr;

        bool m_firstFrame = true;

        // im not expecting anything that's nested beyond 32 depth. its possible but freaking unlikely
        static constexpr std::size_t MaxDepth = 32;
        std::array<std::vector<scenegraph::shared_pointer>, MaxDepth> launch_groups;
        
        // extra info stored for preprocessing
        std::stack<scenenode::shared_pointer> s;

    };
}
