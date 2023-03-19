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

#include "App/Editor/UI/Object Editor/EditorViewport.h"
#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/ECS/GameObjectComponent.h"
#include "Ouroboros/Vulkan/GlobalRendererSettings.h"

#include "Ouroboros/ECS/GameObject.h"
namespace oo
{
    void RendererSystem::UpdateJustCreated()
    {
    }

    void RendererSystem::UpdateDuplicated()
    {
        // Update Newly Duplicated Lights
        static Ecs::Query duplicated_lights_query = Ecs::make_raw_query<LightComponent, TransformComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_lights_query, [&](LightComponent& lightComp, TransformComponent& transformComp, DuplicatedComponent& dupComp)
            {
                InitializeLight(lightComp, transformComp);
            });

        // Update Newly Duplicated Mesh
        static Ecs::Query duplicated_meshes_query = Ecs::make_raw_query<MeshRendererComponent, TransformComponent, GameObjectComponent, DuplicatedComponent>();
        m_world->for_each(duplicated_meshes_query, [&](MeshRendererComponent& meshComp, TransformComponent& transformComp, GameObjectComponent& goc, DuplicatedComponent& dupComp)
            {
                InitializeMesh(meshComp, transformComp, goc);
            });
    }

    void RendererSystem::UpdateExisting()
    {
        // update meshes
        static Ecs::Query mesh_query = Ecs::make_query<MeshRendererComponent, TransformComponent, GameObjectComponent>();
        m_world->for_each(mesh_query, [&](MeshRendererComponent& m_comp, TransformComponent& transformComp, GameObjectComponent& goc)
            {
                //do nothing if transform did not change
                auto& actualObject = m_graphicsWorld->GetObjectInstance(m_comp.GraphicsWorldID);
                actualObject.modelID = m_comp.ModelHandle;
                actualObject.bindlessGlobalTextureIndex_Albedo = m_comp.AlbedoID;
                actualObject.bindlessGlobalTextureIndex_Normal = m_comp.NormalID;
                actualObject.bindlessGlobalTextureIndex_Metallic    = m_comp.MetallicID;
                actualObject.bindlessGlobalTextureIndex_Roughness   = m_comp.RoughnessID;
                actualObject.submesh = m_comp.MeshInformation.submeshBits;
                
                actualObject.bindlessGlobalTextureIndex_Emissive = m_comp.EmissiveID;
                actualObject.emissiveColour = m_comp.EmissiveColor;

                if (transformComp.HasChangedThisFrame)
                    actualObject.localToWorld = transformComp.GlobalTransform;

                actualObject.SetShadowCaster(m_comp.CastShadows);
                actualObject.SetShadowEnabled(m_comp.CastShadows);
                actualObject.SetShadowReciever(m_comp.ReceiveShadows);
            });

        // Update Lights
        static Ecs::Query light_query = Ecs::make_query<LightComponent, TransformComponent>();
        m_world->for_each(light_query, [&](LightComponent& lightComp, TransformComponent& transformComp)
            {
                auto& graphics_light = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);

                if (transformComp.HasChangedThisFrame)
                    graphics_light.position = glm::vec4{ transformComp.GetGlobalPosition(), 0.f };

                graphics_light.color = glm::vec4{ lightComp.Color.r, lightComp.Color.g, lightComp.Color.b, lightComp.Intensity };
                graphics_light.radius = vec4{ lightComp.Radius, 0, 0, 0 };

                SetCastsShadows(graphics_light, lightComp.ProduceShadows);
            });
    }

    void RendererSystem::OnScreenResize(WindowResizeEvent* e)
    {
        auto w = e->GetHeight();
        auto h = e->GetWidth();
        auto ar = static_cast<float>(w) / h;
        EditorViewport::EditorCamera.SetAspectRatio(ar);
        m_graphicsWorld->cameras[1] = EditorViewport::EditorCamera;
        m_runtimeCamera.SetAspectRatio(ar);
        m_graphicsWorld->cameras[0] = m_runtimeCamera;
    }

    void RendererSystem::OnEditorViewportResize(EditorViewportResizeEvent* e)
    {
        auto w = e->X;
        auto h = e->Y;
        auto ar = static_cast<float>(w) / h;
        EditorViewport::EditorCamera.SetAspectRatio(ar);
        ASSERT_MSG(m_graphicsWorld == nullptr, "Graphics world shouldn't be null!");
        m_graphicsWorld->cameras[1] = EditorViewport::EditorCamera;
    }

    /*void RendererSystem::OnPreviewWindowResize(PreviewWindowResizeEvent* e)
    {
        auto w = e->X;
        auto h = e->Y;
        auto ar = static_cast<float>(w) / h;
        m_runtimeCamera.SetAspectRatio(ar);
        m_graphicsWorld->cameras[0] = m_runtimeCamera;
    }*/

    void RendererSystem::OnUpdateRendererSettings(UpdateRendererSettings*)
    {
        m_graphicsWorld->ssaoSettings.bias = RendererSettings::setting.SSAO.Bias;
        m_graphicsWorld->ssaoSettings.radius = RendererSettings::setting.SSAO.Radius;
        m_graphicsWorld->ssaoSettings.intensity = RendererSettings::setting.SSAO.intensity;

        m_graphicsWorld->lightSettings.ambient = RendererSettings::setting.Lighting.Ambient;
        m_graphicsWorld->lightSettings.maxBias = RendererSettings::setting.Lighting.MaxBias;
        m_graphicsWorld->lightSettings.biasMultiplier = RendererSettings::setting.Lighting.BiasMultiplier;

        m_graphicsWorld->bloomSettings.threshold = RendererSettings::setting.Bloom.Threshold;
        m_graphicsWorld->bloomSettings.softThreshold = RendererSettings::setting.Bloom.SoftThreshold;
        
        m_graphicsWorld->colourSettings.highlightThreshold = RendererSettings::setting.ColourCorrection.HighlightThreshold;
        m_graphicsWorld->colourSettings.shadowThreshold = RendererSettings::setting.ColourCorrection.ShadowThreshold;
        m_graphicsWorld->colourSettings.shadowColour = RendererSettings::setting.ColourCorrection.ShadowColour;
        m_graphicsWorld->colourSettings.midtonesColour = RendererSettings::setting.ColourCorrection.MidtonesColour;
        m_graphicsWorld->colourSettings.highlightColour = RendererSettings::setting.ColourCorrection.HighlightColour;

        m_graphicsWorld->vignetteSettings.colour = RendererSettings::setting.Vignette.Colour;
        m_graphicsWorld->vignetteSettings.innerRadius = RendererSettings::setting.Vignette.InnerRadius;
        m_graphicsWorld->vignetteSettings.outerRadius = RendererSettings::setting.Vignette.OuterRadius;

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
        auto& gameobject_component = m_world->get_component<GameObjectComponent>(evnt->entityID);
        InitializeMesh(meshComp, transform_component, gameobject_component);

        // TODO: HARDCODED DEFAULTS : CURRENTLY ASSIGNED CUBE, TO BE REMOVED LATER
        meshComp.ModelHandle = 0;
        meshComp.MeshInformation.submeshBits[0] = true;
    }

    void oo::RendererSystem::OnMeshRemove(Ecs::ComponentEvent<MeshRendererComponent>* evnt)
    {
        auto& comp = evnt->component;
        m_scene->RemovePickingID(comp.PickingID);
        m_graphicsWorld->DestroyObjectInstance(comp.GraphicsWorldID);
        //m_graphicsWorld->DestroyObjectInstance(comp.GraphicsWorldID);
        //// remove graphics id to uuid of gameobject
        //m_graphicsIdToUUID.erase(comp.GraphicsWorldID);
    }

    oo::RendererSystem::RendererSystem(GraphicsWorld* graphicsWorld, Scene* scene)
        : m_graphicsWorld { graphicsWorld }
        , m_scene { scene }
    {
        assert(graphicsWorld != nullptr);	// it should never be nullptr, who's calling this?
    }

    RendererSystem::~RendererSystem()
    {
        // unsubscribe or it'll crash
        EventManager::Unsubscribe<RendererSystem, EditorViewportResizeEvent>(this, &RendererSystem::OnEditorViewportResize);
        //EventManager::Unsubscribe<RendererSystem, PreviewWindowResizeEvent>(this, &RendererSystem::OnPreviewWindowResize);
        EventManager::Unsubscribe<RendererSystem, WindowResizeEvent>(this, &RendererSystem::OnScreenResize);
        EventManager::Unsubscribe<RendererSystem, GameObjectComponent::OnEnableEvent>(this, &RendererSystem::OnEnableGameObject);
        EventManager::Unsubscribe<RendererSystem, GameObjectComponent::OnDisableEvent>(this, &RendererSystem::OnDisableGameObject);
        EventManager::Unsubscribe<RendererSystem, UpdateRendererSettings>(this, &RendererSystem::OnUpdateRendererSettings);
    }

    void RendererSystem::Init()
    {
        // setup runtime camera
        m_runtimeCamera = [&]()
        {
            Camera camera;
            camera.m_CameraMovementType = Camera::CameraMovementType::firstperson;
#if OO_EXECUTABLE
            auto [width, height] = Application::Get().GetWindow().GetSize();
            camera.SetAspectRatio((float)width / (float)height);
#else 
            /*GetPreviewWindowSizeEvent e;
            EventManager::Broadcast<GetPreviewWindowSizeEvent>(&e);
            auto ar = e.Width / e.Height;
            camera.SetAspectRatio(ar);*/
            static constexpr float defaultAR = 16.0f / 9.0f;
            camera.SetAspectRatio(defaultAR);
#endif
            camera.movementSpeed = 5.0f;
            //camera.SetPosition({ 0, 8, 8 });
            //camera.Rotate({ 45, 180, 0 });
            return camera;
        }();

        m_runtimeCC.SetCamera(&m_runtimeCamera);

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

        //EventManager::Subscribe<RendererSystem, PreviewWindowResizeEvent>(this, &RendererSystem::OnPreviewWindowResize);
        EventManager::Subscribe<RendererSystem, EditorViewportResizeEvent>(this, &RendererSystem::OnEditorViewportResize);
        EventManager::Subscribe<RendererSystem, WindowResizeEvent>(this, &RendererSystem::OnScreenResize);
        EventManager::Subscribe<RendererSystem, GameObjectComponent::OnEnableEvent>(this, &RendererSystem::OnEnableGameObject);
        EventManager::Subscribe<RendererSystem, GameObjectComponent::OnDisableEvent>(this, &RendererSystem::OnDisableGameObject);
        
        EventManager::Subscribe<RendererSystem, UpdateRendererSettings>(this, &RendererSystem::OnUpdateRendererSettings);
        // launch the event manually myself once.
        UpdateRendererSettings e;
        oo::EventManager::Broadcast<UpdateRendererSettings>(&e);
    }

    void RendererSystem::PostSceneLoadInit()
    {
        // update meshes positions!
        static Ecs::Query mesh_query = Ecs::make_raw_query<MeshRendererComponent, TransformComponent, GameObjectComponent>();
        m_world->for_each(mesh_query, [&](MeshRendererComponent& m_comp, TransformComponent& transformComp, GameObjectComponent& goc)
            {
                auto& actualObject = m_graphicsWorld->GetObjectInstance(m_comp.GraphicsWorldID);
                actualObject.localToWorld = transformComp.GlobalTransform;
            });
        
        // Update Lights positions immediately!
        static Ecs::Query light_query = Ecs::make_raw_query<LightComponent, TransformComponent>();
        m_world->for_each(light_query, [&](LightComponent& lightComp, TransformComponent& transformComp)
            {
                auto& graphics_light = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);
                graphics_light.position = glm::vec4{ transformComp.GetGlobalPosition(), 0.f };
            });
    }

    void RendererSystem::SaveEditorCamera()
    {
        // Save camera information back to appropriate place
        oo::GetCurrentSceneStateEvent e;
        oo::EventManager::Broadcast(&e);
        switch (e.state)
        {
        case oo::SCENE_STATE::RUNNING:
            EditorViewport::EditorCamera = m_graphicsWorld->cameras[1];
            break;
        }
        //EditorController::EditorCamera  = Application::Get().GetWindow().GetVulkanContext()->getRenderer()->camera;
    }

    /*UUID RendererSystem::GetUUID(uint32_t graphicsID) const
    {
        if (m_graphicsIdToUUID.contains(graphicsID))
            return m_graphicsIdToUUID.at(graphicsID);
        
        return UUID::Invalid;
    }*/

    void oo::RendererSystem::Run(Ecs::ECSWorld* world)
    {
        UpdateJustCreated();
        UpdateDuplicated();
        UpdateExisting();

        // draw debug stuff
        RenderDebugDraws();
    }

    // additional function that runs during runtime scene only.
    void RendererSystem::UpdateCameras(Scene::go_ptr& mainCamera)
    {
        // Update Camera(s)
        // TODO : for the time being only updates 1 global Editor Camera and only occurs in runtime mode.

        Camera* camera = m_runtimeCC.GetCamera();
        static Ecs::Query camera_query = Ecs::make_query<GameObjectComponent, CameraComponent, TransformComponent>();
        m_world->for_each(camera_query, [&](GameObjectComponent& goc, CameraComponent& cameraComp, TransformComponent& transformComp)
        {
            /*if (!transformComp.HasChangedThisFrame)
                return;*/
            // set this to be the main scene camera!
            if (cameraComp.MainCamera)
            {
                mainCamera = m_scene->FindWithInstanceID(goc.Id);
                // TODO : for the time being this will always be hard coded to be 0.
                cameraComp.GraphicsWorldIndex = 0;
            }

            camera->SetPosition(transformComp.GetGlobalPosition());
            camera->SetRotation(transformComp.GetGlobalRotationQuat());
            switch (cameraComp.AspectRatio)
            {
            case CameraAspectRatio::FOUR_BY_THREE:
                camera->SetAspectRatio(4.0f/3.0f);
                break;
            case CameraAspectRatio::SIXTEEN_BY_NINE:
                camera->SetAspectRatio(16.0f/9.0f);
                break;
            case CameraAspectRatio::SIXTEEN_BY_TEN:
                camera->SetAspectRatio(16.0f/10.0f);
                break;
            }
        });
        m_runtimeCC.Update(oo::timer::dt(), false);

        m_graphicsWorld->cameras[0] = *camera;
        m_graphicsWorld->cameras[1] = EditorViewport::EditorCamera;
    }
    
    void RendererSystem::RenderDebugDraws()
    {
        if (LightsDebugDraw)
        {
            // Draw Debug Lights
            static Ecs::Query light_query = Ecs::make_query<LightComponent, TransformComponent>();
            m_world->for_each(light_query, [&](LightComponent& lightComp, TransformComponent& transformComp)
            {
                auto& graphics_light = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);
                // lighting debug draw
                oGFX::Sphere sphere;
                sphere.center = vec3{ graphics_light.position };
                sphere.radius = 0.1f;
                oGFX::DebugDraw::AddSphere(sphere, graphics_light.color);
            });
        }

        if (CameraDebugDraw)
        {
            // draws camera frustum if enabled.
            static Ecs::Query camera_query = Ecs::make_query<CameraComponent, TransformComponent>();
            m_world->for_each(camera_query, [&](CameraComponent& cameraComp, TransformComponent& transformComp)
            {
                Camera camera;
                camera.SetPosition(transformComp.GetGlobalPosition());
                camera.SetRotation(transformComp.GetGlobalRotationQuat());              
                switch (cameraComp.AspectRatio)
                {
                    case CameraAspectRatio::FOUR_BY_THREE: camera.SetAspectRatio(4.0f/3.0f);
                    break;
                    case CameraAspectRatio::SIXTEEN_BY_NINE: camera.SetAspectRatio(16.0f/9.0f);
                    break;
                    case CameraAspectRatio::SIXTEEN_BY_TEN: camera.SetAspectRatio(16.0f/10.0f);
                    break;                
                }
                oGFX::DebugDraw::DrawCameraFrustrum(camera, oGFX::Colors::ORANGE);
            });
        }

    }

    void RendererSystem::InitializeMesh(MeshRendererComponent& meshComp, TransformComponent& transformComp, GameObjectComponent& goc)
    {
        meshComp.GraphicsWorldID = m_graphicsWorld->CreateObjectInstance();
        meshComp.PickingID = m_scene->GeneratePickingID(goc.Id);
        //meshComp.GraphicsWorldID = m_graphicsWorld->CreateObjectInstance();
        
        //update graphics world side
        auto& graphics_object = m_graphicsWorld->GetObjectInstance(meshComp.GraphicsWorldID);
        graphics_object.entityID = meshComp.PickingID;
        graphics_object.localToWorld = transformComp.GetGlobalMatrix();

        //// map graphics id to uuid of gameobject
        //m_graphicsIdToUUID.insert({ meshComp.GraphicsWorldID, goc.Id });
        //m_uuidToGraphicsID.insert({ goc.Id, meshComp.GraphicsWorldID });
    }

    void RendererSystem::InitializeLight(LightComponent& lightComp, TransformComponent& transformComp)
    {
        lightComp.Light_ID = m_graphicsWorld->CreateLightInstance();
        //update graphics world side to prevent wrong initial placement
        auto& graphics_object = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);
        graphics_object.position = glm::vec4{ transformComp.GetGlobalPosition(), 0.f };
    }

    void RendererSystem::OnEnableGameObject(GameObjectComponent::OnEnableEvent* e)
    {
        // check for lights
        auto go = m_scene->FindWithInstanceID(e->Id);
        if (go->HasComponent<LightComponent>())
        {
            auto& lightComp = go->GetComponent<LightComponent>();
            auto& actualLight = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);
            SetLightEnabled(actualLight, true);
        }
    }

    void RendererSystem::OnDisableGameObject(GameObjectComponent::OnDisableEvent* e)
    {
        auto go = m_scene->FindWithInstanceID(e->Id);
        if (go->HasComponent<LightComponent>())
        {
            auto& lightComp = go->GetComponent<LightComponent>();
            auto& actualLight = m_graphicsWorld->GetLightInstance(lightComp.Light_ID);
            SetLightEnabled(actualLight, false);
        }
    }

}

