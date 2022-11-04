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

#include "Ouroboros/Core/Application.h"
#include "VulkanContext.h"
#include "Ouroboros/Core/Timer.h"
#include "Ouroboros/Core/Application.h"

#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "Ouroboros/Core/Input.h"

#include "Ouroboros/Scene/EditorController.h"

namespace oo
{
    Camera EditorController::EditorCamera = [&]()
    {
        Camera camera;
        camera.m_CameraMovementType = Camera::CameraMovementType::firstperson;
        camera.movementSpeed = 5.0f;
        //camera.SetPosition({ 0, 8, 8 });
        //camera.Rotate({ 45, 180, 0 });
        return camera;
    }();

    void RendererSystem::OnScreenResize(WindowResizeEvent* e)
    {
        auto w = e->GetHeight();
        auto h = e->GetWidth();
        auto ar = static_cast<float>(w) / h;
        EditorController::EditorCamera.SetAspectRatio(ar);
        m_runtimeCamera.SetAspectRatio(ar);
    }

    void oo::RendererSystem::OnLightAssign(Ecs::ComponentEvent<LightComponent>* evnt)
    {
        assert(m_world != nullptr); // it should never be nullptr, was the Init funciton called?
        auto& lightComp = evnt->component;
        auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
        InitializeLight(lightComp, transform_component);
    }

    void oo::RendererSystem::OnLightRemove(Ecs::ComponentEvent<LightComponent>* evnt)
    {
        auto& comp = evnt->component;
        m_graphicsWorld->DestroyLightInstance(comp.Light_ID);
    }

    void oo::RendererSystem::OnMeshAssign(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
    {
        assert(m_world != nullptr); // it should never be nullptr, was the Init funciton called?

        auto& meshComp = evnt->component;
        auto& transform_component = m_world->get_component<TransformComponent>(evnt->entityID);
        InitializeMesh(meshComp, transform_component);

        //HARDCODED DEFAULTS : CURRENTLY ASSIGNED CUBE, TO BE REMOVED LATER
        meshComp.model_handle = 0;
        meshComp.meshInfo.submeshBits[0] = true;
    }

    void oo::RendererSystem::OnMeshRemove(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
    {
        auto& comp = evnt->component;
        m_graphicsWorld->DestroyObjectInstance(comp.graphicsWorld_ID);
    }

    oo::RendererSystem::RendererSystem(GraphicsWorld* graphicsWorld)
        : m_graphicsWorld { graphicsWorld }
    {
        assert(graphicsWorld != nullptr);	// it should never be nullptr, who's calling this?
    }

    void RendererSystem::Init()
    {
        // setup cameras
        auto [width, height] = Application::Get().GetWindow().GetSize();
        // set camera
        oo::GetCurrentSceneStateEvent e;
        oo::EventManager::Broadcast(&e);
        switch (e.state)
        {
        case oo::SCENE_STATE::EDITING:
            EditorController::EditorCamera.SetAspectRatio((float)width / (float)height);
            m_graphicsWorld->cameras[0] = EditorController::EditorCamera;
            break;
        case oo::SCENE_STATE::RUNNING:

            m_runtimeCamera = [&]()
            {
                Camera camera;
                camera.m_CameraMovementType = Camera::CameraMovementType::firstperson;
                camera.SetAspectRatio((float)width / (float)height);
                camera.movementSpeed = 5.0f;
                //camera.SetPosition({ 0, 8, 8 });
                //camera.Rotate({ 45, 180, 0 });
                return camera;
            }();

            m_graphicsWorld->cameras[0] = m_runtimeCamera;
            break;
        }
        auto& camera = m_graphicsWorld->cameras[0];
        m_cc.SetCamera(&camera);

        // Mesh Renderer
        m_world->SubscribeOnAddComponent<RendererSystem, MeshRendererComponent>(
            this, &RendererSystem::OnMeshAssign);

        m_world->SubscribeOnRemoveComponent<RendererSystem, MeshRendererComponent>(
            this, &RendererSystem::OnMeshRemove);

        //Lights
        m_world->SubscribeOnAddComponent<RendererSystem, LightComponent>(
            this, &RendererSystem::OnLightAssign);

        m_world->SubscribeOnRemoveComponent<RendererSystem, LightComponent>(
            this, &RendererSystem::OnLightRemove);

        EventManager::Subscribe<RendererSystem, WindowResizeEvent>(this, &RendererSystem::OnScreenResize);
    }

    void RendererSystem::SaveEditorCamera()
    {
        // Save camera information back to appropriate place
        oo::GetCurrentSceneStateEvent e;
        oo::EventManager::Broadcast(&e);
        switch (e.state)
        {
        case oo::SCENE_STATE::RUNNING:
            EditorController::EditorCamera = m_graphicsWorld->cameras[0];
            break;
        }
        //EditorController::EditorCamera  = Application::Get().GetWindow().GetVulkanContext()->getRenderer()->camera;
    }

    void oo::RendererSystem::Run(Ecs::ECSWorld* world)
    {
        // Update Newly Duplicated Lights
        static Ecs::Query duplicated_lights_query = Ecs::make_raw_query<LightComponent, TransformComponent, DuplicatedComponent>();
        world->for_each(duplicated_lights_query, [&](LightComponent& lightComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
        {
            InitializeLight(lightComp, transformComp);
        });

        // Update Newly Duplicated Mesh
        static Ecs::Query duplicated_meshes_query = Ecs::make_raw_query<MeshRendererComponent, TransformComponent, DuplicatedComponent>();
        world->for_each(duplicated_meshes_query, [&](MeshRendererComponent& meshComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
        { 
            InitializeMesh(meshComp, transformComp);
        });

        // update meshes
        static Ecs::Query mesh_query = Ecs::make_query<MeshRendererComponent, TransformComponent>();
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
            
            // update transform if this is the first frame of rendering
            if (m_firstFrame)
            {
                actualObject.localToWorld = transformComp.GlobalTransform;
                m_firstFrame = false;
            }
        });


        // Update Lights
        static Ecs::Query light_query = Ecs::make_query<LightComponent, TransformComponent>();
        world->for_each(light_query, [&](LightComponent& lightComp, TransformComponent& transformComp)
        {
            auto& graphics_light = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);

            //if (transformComp.HasChanged())
            graphics_light.position = glm::vec4{ transformComp.GetGlobalPosition(), 0.f };
            graphics_light.color = lightComp.Color;
            graphics_light.radius = lightComp.Radius;
        });

        // draw debug stuff
        RenderDebugDraws(world);
    }

    void RendererSystem::UpdateCamerasEditorMode()
    {
        m_cc.Update(oo::timer::dt());
        auto pos = m_cc.GetCamera()->m_position;
        LOG_TRACE("Editor Camera Position {0} {1} {2}", pos.x, pos.y, pos.z);
    }

    // additional function that runs during runtime scene only.
    void RendererSystem::UpdateCamerasRuntime()
    {
        static bool using_editor_camera = false;
        if (oo::input::IsKeyPressed(KEY_F8))
        {
            using_editor_camera = !using_editor_camera;

            if (using_editor_camera)
            {
                m_runtimeCamera = m_graphicsWorld->cameras[0];
                m_graphicsWorld->cameras[0] = EditorController::EditorCamera;
            }
            else
            {
                m_graphicsWorld->cameras[0] = m_runtimeCamera;
            }
            m_cc.SetCamera(&m_graphicsWorld->cameras[0]);
        }

        if (!using_editor_camera)
        {
            // TODO: debug draw the camera's view in editormode
            //DebugDraw::AddLine();

            // Update Camera(s)
            // TODO : for the time being only updates 1 global Editor Camera and only occurs in runtime mode.

            Camera* camera = m_cc.GetCamera();
            static Ecs::Query camera_query = Ecs::make_query<CameraComponent, TransformComponent>();
            m_world->for_each(camera_query, [&](CameraComponent& cameraComp, TransformComponent& transformComp)
            {
                camera->SetPosition(transformComp.GetGlobalPosition());
                camera->SetRotation(transformComp.GetGlobalRotationQuat());
            });
        }

        m_cc.Update(oo::timer::dt(), using_editor_camera);
    }
    
    void RendererSystem::RenderDebugDraws(Ecs::ECSWorld* world)
    {
        // Draw Debug Lights
        static Ecs::Query light_query = Ecs::make_query<LightComponent, TransformComponent>();
        world->for_each(light_query, [&](LightComponent& lightComp, TransformComponent& transformComp)
        {
            auto& graphics_light = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);
            // lighting debug draw
            Sphere sphere;
            sphere.center = vec3{ graphics_light.position };
            sphere.radius = 0.1f;
            DebugDraw::AddSphere(sphere, graphics_light.color);
        });
    }

    void RendererSystem::InitializeMesh(MeshRendererComponent& meshComp, TransformComponent& transformComp)
    {
        meshComp.graphicsWorld_ID = m_graphicsWorld->CreateObjectInstance();

        //update graphics world side
        auto& graphics_object = m_graphicsWorld->GetObjectInstance(meshComp.graphicsWorld_ID);
        graphics_object.localToWorld = transformComp.GetGlobalMatrix();
    }

    void RendererSystem::InitializeLight(LightComponent& lightComp, TransformComponent& transformComp)
    {
        lightComp.Light_ID = m_graphicsWorld->CreateLightInstance();
        //update graphics world side to prevent wrong initial placement
        auto& graphics_object = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);
        graphics_object.position = glm::vec4{ transformComp.GetGlobalPosition(), 0.f };
    }

}

