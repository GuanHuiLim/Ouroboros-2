#include "pch.h"
#include "RendererSystem.h"
#include <rttr/registration>

namespace oo
{
    void oo::MeshRendererSystem::OnLightAssign(Ecs::ComponentEvent<LightingComponent>* evnt)
    {
        assert(m_world != nullptr); // it should never be nullptr, was the Init funciton called?

        auto& comp = evnt->component;
        comp.Light_ID = m_graphicsWorld->CreateLightInstance();

        //update graphics world side to prevent wrong initial placement
        auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
        auto& graphics_object = m_graphicsWorld->GetLightInstance(comp.Light_ID);
        graphics_object.position = glm::vec4{ transform_component.GetGlobalPosition(), 0.f };
    }

    void oo::MeshRendererSystem::OnLightRemove(Ecs::ComponentEvent<LightingComponent>* evnt)
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
        graphics_object.localToWorld = transform_component.GetGlobalMatrix();
        
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

        // Mesh Renderer
        m_world->SubscribeOnAddComponent<MeshRendererSystem, MeshRendererComponent>(
            this, &MeshRendererSystem::OnMeshAssign);

        m_world->SubscribeOnRemoveComponent<MeshRendererSystem, MeshRendererComponent>(
            this, &MeshRendererSystem::OnMeshRemove);

        //Lights
        m_world->SubscribeOnAddComponent<MeshRendererSystem, LightingComponent>(
            this, &MeshRendererSystem::OnLightAssign);

        m_world->SubscribeOnRemoveComponent<MeshRendererSystem, LightingComponent>(
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

            if (transformComp.HasChanged())
                actualObject.localToWorld = transformComp.GetGlobalMatrix();
            });

        // Update Lights
        static Ecs::Query light_query = []() 
        {
            Ecs::Query query;
            return query.with<LightingComponent, TransformComponent>().build();
        }();

        world->for_each(light_query, [&](LightingComponent& lightComp, TransformComponent& transformComp)
        {
            auto& graphics_light = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);

            //if (transformComp.HasChanged())
            graphics_light.position = glm::vec4{ transformComp.GetGlobalPosition(), 0.f };
            graphics_light.color = lightComp.Color;
            graphics_light.radius = lightComp.Radius;
        });
    }
}

