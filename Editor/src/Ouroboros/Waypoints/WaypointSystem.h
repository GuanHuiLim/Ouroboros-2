#pragma once

#include "Ouroboros/ECS/ArchtypeECS/A_Ecs.h"
#include "WaypointSetComponent.h"
#include "WaypointNodeComponent.h"

#include "Ouroboros/ECS/GameObjectComponent.h"
#include "Ouroboros/Scene/Scene.h"

namespace oo
{
    class WaypointSystem : public Ecs::System
    {
    public:
        WaypointSystem(Scene* scene);
        virtual~WaypointSystem();

        virtual void Run(Ecs::ECSWorld* world) override {};

        void PostLoadSceneInit();
        void EditorUpdate();
        void RuntimeUpdate();

        inline static bool DebugDrawSetPath = true;
    private:
        void UpdateJustCreated();
        void UpdateDuplicated();
        void UpdateExisting();

        void SetUpWaypointSet();    // should be synounymous to reset.
        void UpdateWaypointNode();

        void OnEnableGameObject(GameObjectComponent::OnEnableEvent* e);
        void OnDisableGameObject(GameObjectComponent::OnDisableEvent* e);
    
    private:
        Scene* m_scene;
    };
}
