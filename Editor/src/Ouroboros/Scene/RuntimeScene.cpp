/************************************************************************************//*!
\file           RuntimeScene.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          RuntimeScene describes the scene when players are simulating the scene
                with all systems running reflective of the final build.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "RuntimeScene.h"

//#include "Editor.h"
//#include "Waypoint/WaypointSystem.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"

#include "Ouroboros/Scripting/ScriptSystem.h"
#include "Ouroboros/Input/InputSystem.h"
//#include "Ouroboros/Vulkan/RendererSystem.h"

#include "Ouroboros/Physics/PhysicsSystem.h"

namespace oo
{
    RuntimeScene::RuntimeScene(std::string const& filepath)
        : Scene{ "Runtime Scene" }
    {
        if (!filepath.empty())
            SetFilePath(filepath);
    }

    void RuntimeScene::Init()
    {
        Scene::Init();

        constexpr const char* const registration = "registration";
        {
            TRACY_PROFILE_SCOPE(registration);

            GetWorld().Add_System<InputSystem>()->Initialize();

            GetWorld().Add_System<PhysicsSystem>();

            //Register All Systems
            //GetWorld().Add_System<ScriptSystem>(*this);
            /*auto meshObj = oo::Mesh::CreateCubeMeshObject(this, GetGraphicsWorld());
            meshObj->GetComponent<TransformComponent>().SetScale({ 5.f,5.f,5.f });*/
            //GetWorld().RegisterSystem<PrefabComponentSystem>();
            //GetWorld().RegisterSystem<EditorComponentSystem>();

            //GetWorld().RegisterSystem<oo::Renderer2DSystem>(*oo::EditorCamera::g_editorCam);
            //GetWorld().RegisterSystem<oo::ParticleRenderingSystem>();
            //GetWorld().RegisterSystem<oo::QuadtreeSystem>();
            //GetWorld().RegisterSystem<oo::AnimatorSystem>();
            //GetWorld().RegisterSystem<oo::UIRenderingSystem>();
            //GetWorld().RegisterSystem<oo::PhysicsSystem>();
            //GetWorld().RegisterSystem<oo::UISystem>();
            //GetWorld().RegisterSystem<oo::VideoSystem>();

            //GetWorld().RegisterSystem<oo::AudioSystem>();
            //GetWorld().RegisterSystem<WaypointSystem>();
            //GetWorld().RegisterSystem<oo::AnimatorControllersystem>();

            //auto scriptSystem = GetWorld().RegisterSystem<oo::ScriptSystem>();
            //scriptSystem->SetCallbackInvokes();

            TRACY_PROFILE_SCOPE_END();
        }

        constexpr const char* const loading_world = "loading world";
        {
            TRACY_PROFILE_SCOPE(loading_world);
            LoadFromFile();
            TRACY_PROFILE_SCOPE_END();
        }

        StartSimulation();
    }

    void RuntimeScene::Update()
    {
        if (m_stepMode && m_framesLeft == 0) return;
        --m_framesLeft;

        Scene::Update();

        //GetWorld().Get_System<ScriptSystem>()->InvokeForAllEnabled("Update");

        constexpr const char* const runtime_scene_update = "Runtime Scene Update";
        {
            TRACY_PROFILE_SCOPE(runtime_scene_update);

            constexpr const char* const input_update = "Input Update";
            {
                 TRACY_PROFILE_SCOPE(input_update);
                 GetWorld().Get_System<InputSystem>()->Run(&GetWorld());
                 TRACY_PROFILE_SCOPE_END();
            }

            GetWorld().Get_System<PhysicsSystem>()->RuntimeUpdate(timer::dt());

            //Update All Systems
            //constexpr const char* const scripts_update = "Scripts Update";
            //{
            //    TRACY_PROFILE_SCOPE(scripts_update);
            //    GetWorld().Get_System<ScriptSystem>()->InvokeForAllEnabled("Update");
            //    //auto ss = GetWorld().GetSystem<oo::ScriptSystem>();
            //    //ss->InvokeFunctionAll("Update");
            //    TRACY_PROFILE_SCOPE_END();
            //}
            //constexpr const char* const ui_update = "UI Update";
            {
               /* TRACY_PROFILE_SCOPE(ui_update);
                GetWorld().GetSystem<oo::UISystem>()->RuntimeUpdate();
                TRACY_PROFILE_SCOPE_END();*/
            }
            //constexpr const char* const transforms_first_parse = "transforms first parse update";
            {
                //TRACY_PROFILE_SCOPE(transforms_first_parse);
                ////Transforms gets first-parsed updated after scripts
                //GetWorld().GetSystem<oo::TransformSystem>()->UpdateTransform();
                //TRACY_PROFILE_SCOPE_END();
            }
            //constexpr const char* const physics_update = "physics update";
            {
                /*TRACY_PROFILE_SCOPE(physics_update);
                GetWorld().GetSystem<oo::PhysicsSystem>()->Update(dt);
                TRACY_PROFILE_SCOPE_END();*/
            }
            //constexpr const char* const scripts_lateupdate = "Scripts LateUpdate";
            {
                /*TRACY_PROFILE_SCOPE(scripts_lateupdate);
                auto ss = GetWorld().GetSystem<oo::ScriptSystem>();
                ss->InvokeFunctionAll("LateUpdate");
                TRACY_PROFILE_SCOPE_END();*/
            }
            //constexpr const char* const transform_update = "tranform update";
            {
                /*TRACY_PROFILE_SCOPE(transform_update);
                GetWorld().GetSystem<oo::TransformSystem>()->Update();
                TRACY_PROFILE_SCOPE_END();*/
            }
            //constexpr const char* const animator_update = "animator update";
            {
                /*TRACY_PROFILE_SCOPE(animator_update);
                GetWorld().GetSystem<oo::AnimatorSystem>()->Update(dt);
                TRACY_PROFILE_SCOPE_END();*/
            }
            //constexpr const char* const audio_update = "audio update";
            {
                /*TRACY_PROFILE_SCOPE(audio_update);
                GetWorld().GetSystem<oo::AudioSystem>()->Update();
                TRACY_PROFILE_SCOPE_END();*/
            }
            //constexpr const char* const waypoint_update = "waypoint update";
            {
                /*TRACY_PROFILE_SCOPE(waypoint_update);
                GetWorld().GetSystem<WaypointSystem>()->Update(dt);
                TRACY_PROFILE_SCOPE_END();*/
            }
            //constexpr const char* const animator_controller_update = "animator controller update";
            {
                /*TRACY_PROFILE_SCOPE(animator_controller_update);
                GetWorld().GetSystem<oo::AnimatorControllersystem>()->Update(dt);
                TRACY_PROFILE_SCOPE_END();*/
            }

            //constexpr const char* const particles_update = "Particles update";
            {
                /*TRACY_TRACK_PERFORMANCE(particles_update);
                GetWorld().GetSystem<oo::ParticleRenderingSystem>()->Update(dt);*/
            }

            //constexpr const char* const video_update = "Video update";
            {
                /*TRACY_TRACK_PERFORMANCE(video_update);
                GetWorld().GetSystem<oo::VideoSystem>()->Update(dt);*/
            }
            //TRACY_PROFILE_SCOPE_END();
        }

    }

    void RuntimeScene::LateUpdate()
    {
        Scene::LateUpdate();
    }

    void RuntimeScene::Render()
    {
        Scene::Render();
        //constexpr const char* const text_rendering = "Text Rendering";
        {
            /*TRACY_PROFILE_SCOPE(text_rendering);
            GetWorld().GetSystem<oo::UIRenderingSystem>()->Render();
            TRACY_PROFILE_SCOPE_END();*/
        }

        //constexpr const char* const particle_rendering = "Particle Rendering";
        {
            /*TRACY_TRACK_PERFORMANCE(particle_rendering);
            GetWorld().GetSystem<oo::ParticleRenderingSystem>()->Render();*/
        }

        //constexpr const char* const standard_rendering = "Standard Rendering";
        {
            /*TRACY_PROFILE_SCOPE(standard_rendering);
            GetWorld().GetSystem<oo::Renderer2DSystem>()->Render();
            TRACY_PROFILE_SCOPE_END();*/
        }
    }

    void RuntimeScene::Exit()
    {
        Scene::Exit();

        StopSimulation();
    }

    void RuntimeScene::ProcessFrame(int count)
    {
        ASSERT_MSG(count < 0, "frames shouldnt be lesser at than 1!!");

        /*ENGINE_ASSERT_MSG(m_isPause == true , "you should only be able to get here once paused!!!");
        m_isPause = false;
        m_stepMode = true;*/

        m_framesLeft = count;
    }

    //void RuntimeScene::StopStepMode()
    //{
    //    //ENGINE_ASSERT_MSG(m_stepMode == true, "stepmode should be true at this point!");
    //    if(m_stepMode)
    //        m_stepMode = false;
    //}

    void RuntimeScene::StartSimulation()
    {
        constexpr const char* const start_simulation = "Start Simulation";
        {

            TRACY_PROFILE_SCOPE(start_simulation);

            GetWorld().Get_System<ScriptSystem>()->StartPlay();

            /*GetWorld().GetSystem<oo::ScriptSystem>()->StartPlay();
            GetWorld().GetSystem<oo::TransformSystem>()->UpdateTransform();
            GetWorld().GetSystem<oo::PhysicsSystem>()->Init();
            GetWorld().GetSystem<oo::ParticleRenderingSystem>()->Init();
            GetWorld().GetSystem<oo::AudioSystem>()->Init();*/

            TRACY_PROFILE_SCOPE_END();
        }
    }

    void RuntimeScene::StopSimulation()
    {
        constexpr const char* const stop_simulation = "Stop Simulation";
        {
            TRACY_PROFILE_SCOPE(stop_simulation);

            GetWorld().Get_System<oo::ScriptSystem>()->StopPlay();
            //GetWorld().GetSystem<oo::ScriptSystem>()->StopPlay();

            //{
            //    // Reset Timescale
            //    oo::Timestep::TimeScale = 1.0;
            //}

            TRACY_PROFILE_SCOPE_END();
        }
    }

    void RuntimeScene::PauseSimulation()
    {
        m_isPause = true;
        m_stepMode = true;
        m_framesLeft = 0;

        //GetWorld().GetSystem<oo::AudioSystem>()->Pause(m_isPause);
    }

    void RuntimeScene::ResumeSimulation()
    {
        m_isPause = false;
        m_stepMode = false;
        m_framesLeft = 0;

        //GetWorld().GetSystem<oo::AudioSystem>()->Pause(m_isPause);
    }

}
