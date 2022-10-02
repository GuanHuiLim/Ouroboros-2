/************************************************************************************//*!
\file           RendererSystem.cpp
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
#include "pch.h"
#include "RendererSystem.h"
#include <rttr/registration>

#include <OO_Vulkan/src/DebugDraw.h>

namespace oo
{
    void oo::MeshRendererSystem::OnLightAssign(Ecs::ComponentEvent<LightComponent>* evnt)
    {
        assert(m_world != nullptr); // it should never be nullptr, was the Init funciton called?

        auto& comp = evnt->component;
        comp.Light_ID = m_graphicsWorld->CreateLightInstance();

        //update graphics world side to prevent wrong initial placement
        auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
        auto& graphics_object = m_graphicsWorld->GetLightInstance(comp.Light_ID);
        graphics_object.position = glm::vec4{ transform_component.GetGlobalPosition(), 0.f };
    }

    void oo::MeshRendererSystem::OnLightRemove(Ecs::ComponentEvent<LightComponent>* evnt)
    {
        auto& comp = evnt->component;
        m_graphicsWorld->DestroyLightInstance(comp.Light_ID);
    }

    void oo::MeshRendererSystem::OnMeshAssign(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
    {
        assert(m_world != nullptr); // it should never be nullptr, was the Init funciton called?

        auto& comp = evnt->component;
        comp.graphicsWorld_ID = m_graphicsWorld->CreateObjectInstance();
        //HARDCODED AS CUBE, TO BE REMOVED LATER
        comp.model_handle = 0;
        
        //update graphics world side
        auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
        auto& graphics_object = m_graphicsWorld->GetObjectInstance(comp.graphicsWorld_ID);
        graphics_object.localToWorld = transform_component.GlobalTransform;
        
    }

    void oo::MeshRendererSystem::OnMeshRemove(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
    {
        auto& comp = evnt->component;
        m_graphicsWorld->DestroyObjectInstance(comp.graphicsWorld_ID);
    }

    oo::MeshRendererSystem::MeshRendererSystem(GraphicsWorld* graphicsWorld)
    {
        assert(graphicsWorld != nullptr);	// it should never be nullptr, who's calling this?

        this->m_graphicsWorld = graphicsWorld;
    }

    void MeshRendererSystem::Init()
    {
        // Mesh Renderer
        m_world->SubscribeOnAddComponent<MeshRendererSystem, MeshRendererComponent>(
            this, &MeshRendererSystem::OnMeshAssign);

        m_world->SubscribeOnRemoveComponent<MeshRendererSystem, MeshRendererComponent>(
            this, &MeshRendererSystem::OnMeshRemove);

        //Lights
        m_world->SubscribeOnAddComponent<MeshRendererSystem, LightComponent>(
            this, &MeshRendererSystem::OnLightAssign);

        m_world->SubscribeOnRemoveComponent<MeshRendererSystem, LightComponent>(
            this, &MeshRendererSystem::OnLightRemove);
    }

    void oo::MeshRendererSystem::Run(Ecs::ECSWorld* world)
    {
        static Ecs::Query mesh_query = []() 
        {
            Ecs::Query query;
            return query.with<MeshRendererComponent, TransformComponent>().build();
        }();

        world->for_each(mesh_query, [&](MeshRendererComponent& m_comp, TransformComponent& transformComp) 
        {
            //do nothing if transform did not change
            auto& actualObject = m_graphicsWorld->GetObjectInstance(m_comp.graphicsWorld_ID);
            actualObject.modelID = m_comp.model_handle;
            actualObject.bindlessGlobalTextureIndex_Albedo = m_comp.albedoID;
            actualObject.bindlessGlobalTextureIndex_Normal= m_comp.normalID;
            actualObject.submesh = m_comp.meshInfo.submeshBits;

            if (transformComp.HasChangedThisFrame)
                actualObject.localToWorld = transformComp.GlobalTransform;
            });

        // Update Lights
        static Ecs::Query light_query = []() 
        {
            Ecs::Query query;
            return query.with<LightComponent, TransformComponent>().build();
        }();

        world->for_each(light_query, [&](LightComponent& lightComp, TransformComponent& transformComp)
        {
            auto& graphics_light = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);

            //if (transformComp.HasChanged())
            graphics_light.position = glm::vec4{ transformComp.GetGlobalPosition(), 0.f };
            graphics_light.color = lightComp.Color;
            graphics_light.radius = lightComp.Radius;
            
            // lighting debug draw
            Sphere sphere;
            sphere.center = vec3{ graphics_light.position }; 
            sphere.radius = 0.1f;
            DebugDraw::AddSphere(sphere, graphics_light.color);
        });

    }
}

