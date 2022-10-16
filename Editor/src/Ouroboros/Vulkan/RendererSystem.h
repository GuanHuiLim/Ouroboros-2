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
#include "RendererComponent.h"
#include "LightComponent.h"
#include "Archetypes_Ecs/src/A_Ecs.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Transform/TransformComponent.h"

namespace oo
{
    class MeshRendererSystem : public Ecs::System
    {
    private:
        GraphicsWorld* m_graphicsWorld{nullptr};
        //Ecs::ECSWorld* m_world{nullptr};
    public:
        MeshRendererSystem(GraphicsWorld* graphicsWorld);

        void Init();

        void OnLightAssign(Ecs::ComponentEvent<LightComponent>* evnt);
        void OnLightRemove(Ecs::ComponentEvent<LightComponent>* evnt);

        void OnMeshAssign(Ecs::ComponentEvent<MeshRendererComponent>* evnt);
        void OnMeshRemove(Ecs::ComponentEvent<MeshRendererComponent>* evnt);

        //void Init(Ecs::ECSWorld* world, GraphicsWorld* graphicsWorld);

        virtual void Run(Ecs::ECSWorld* world) override;
    };
}
