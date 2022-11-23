/************************************************************************************//*!
\file           RendererSystem.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Sept 30, 2022
\brief          Renderer System is in charge of putting all rendering related components
                and performing the correct instructions in order to render the expected
                scene properly

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "MeshRendererComponent.h"
#include "LightComponent.h"
#include "CameraComponent.h"

#include "Archetypes_Ecs/src/A_Ecs.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"
#include "Ouroboros/ECS/GameObjectComponent.h"

#include "Ouroboros/Core/CameraController.h"

#include "Ouroboros/Core/Events/ApplicationEvent.h"
#include "Ouroboros/Scene/Scene.h"


//fwd declaration
struct EditorViewportResizeEvent;
struct PreviewWindowResizeEvent;
struct UpdateRendererSettings;

namespace oo
{

    class RendererSystem : public Ecs::System
    {
    private:
        //GraphicsWorld* m_graphicsWorld{nullptr};
        //Ecs::ECSWorld* m_world{nullptr};
        /*std::map<uint32_t, UUID> m_graphicsIdToUUID;
        std::map<UUID, uint32_t> m_uuidToGraphicsID;*/
    public:
        RendererSystem(GraphicsWorld* graphicsWorld, Scene* scene);
        virtual ~RendererSystem();

        void Init();

        virtual void Run(Ecs::ECSWorld* world) override;

        void UpdateCameras(Scene::go_ptr& mainCamera);
        void SaveEditorCamera();

        //UUID GetUUID(uint32_t graphicsID) const;
        inline static bool CameraDebugDraw = true;
        inline static bool LightsDebugDraw = true;

    private:
        void OnScreenResize(WindowResizeEvent* e);
        void OnEditorViewportResize(EditorViewportResizeEvent* e);
        //void OnPreviewWindowResize(PreviewWindowResizeEvent* e);

        void OnUpdateRendererSettings(UpdateRendererSettings*);

        void OnLightAssign(Ecs::ComponentEvent<LightComponent>* evnt);
        void OnLightRemove(Ecs::ComponentEvent<LightComponent>* evnt);

        void OnMeshAssign(Ecs::ComponentEvent<MeshRendererComponent>* evnt);
        void OnMeshRemove(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

        void RenderDebugDraws(Ecs::ECSWorld* world);
        void InitializeMesh(MeshRendererComponent& meshComp, TransformComponent& transformComp, GameObjectComponent& goc);
        void InitializeLight(LightComponent& lightComp, TransformComponent& transformComp);

        /*void OnEnableGameObject(GameObjectComponent::OnEnableEvent* e);
        void OnDisableGameObject(GameObjectComponent::OnDisableEvent* e);*/
    private:

        GraphicsWorld* m_graphicsWorld{ nullptr };
        Scene* m_scene;

        CameraController m_runtimeCC;
        Camera m_runtimeCamera;
        
        bool m_firstFrame = true; // potentially improvable if this can be run once per creation
    };
}
