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

#include "Ouroboros/Core/CameraController.h"

#include "Ouroboros/Core/Events/ApplicationEvent.h"

//fwd declaration
struct EditorViewportResizeEvent;

namespace oo
{
    class RendererSystem : public Ecs::System
    {
    private:
        GraphicsWorld* m_graphicsWorld{nullptr};
        //Ecs::ECSWorld* m_world{nullptr};
    public:
        RendererSystem(GraphicsWorld* graphicsWorld);
        virtual ~RendererSystem();

        void Init();

        virtual void Run(Ecs::ECSWorld* world) override;

        void UpdateCamerasEditorMode();
        void UpdateCamerasRuntime();
        void SaveEditorCamera();

        inline static bool CameraDebugDraw = true;
    private:
        void OnScreenResize(WindowResizeEvent* e);
        void OnEditorViewportResize(EditorViewportResizeEvent* e);

        void OnLightAssign(Ecs::ComponentEvent<LightComponent>* evnt);
        void OnLightRemove(Ecs::ComponentEvent<LightComponent>* evnt);

        void OnMeshAssign(Ecs::ComponentEvent<MeshRendererComponent>* evnt);
        void OnMeshRemove(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

        void RenderDebugDraws(Ecs::ECSWorld* world);
        void InitializeMesh(MeshRendererComponent& meshComp, TransformComponent& transformComp);
        void InitializeLight(LightComponent& lightComp, TransformComponent& transformComp);
    
    private:
        CameraController m_cc;
        Camera m_runtimeCamera;
        
        bool m_firstFrame = true; // potentially improvable if this can be run once per creation
    };
}
