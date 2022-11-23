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
#include "Ouroboros/Animation/AnimationSystem.h"
//#include "Ouroboros/Vulkan/RendererSystem.h"

#include "Ouroboros/Physics/PhysicsSystem.h"
#include "Ouroboros/Vulkan/RendererSystem.h"
#include "Ouroboros/Vulkan/SkinRendererSystem.h"
#include "Ouroboros/Transform/TransformSystem.h"
#include "Ouroboros/Audio/AudioSystem.h"
#include "Ouroboros/UI/UISystem.h"

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
        TRACY_PROFILE_SCOPE(runtime_scene_init);

        Scene::Init();

        {
            TRACY_PROFILE_SCOPE(runtime_registration);

            GetWorld().Add_System<InputSystem>()->Initialize();
            GetWorld().Add_System<Anim::AnimationSystem>()->Init(&GetWorld(), this);

            GetWorld().Add_System<PhysicsSystem>()->Init(this);
            GetWorld().Add_System<oo::UISystem>(this);

            GetWorld().Get_System<Anim::AnimationSystem>()->CreateAnimationTestObject();

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

        // Some system setup code
        {
            // set default debug draws
            GetWorld().Get_System<PhysicsSystem>()->ColliderDebugDraw = false;
            GetWorld().Get_System<RendererSystem>()->CameraDebugDraw = false;
            GetWorld().Get_System<RendererSystem>()->LightsDebugDraw = false;
        }

        {
            TRACY_PROFILE_SCOPE(runtime_load_from_file);
            LoadFromFile();
            TRACY_PROFILE_SCOPE_END();
        }
        //post load file processes
        GetWorld().Get_System<Anim::AnimationSystem>()->BindPhase();
        GetWorld().Get_System<SkinMeshRendererSystem>()->PostLoadScene(*this);

        StartSimulation();

        TRACY_PROFILE_SCOPE_END();
    }

    void RuntimeScene::Update()
    {
        if (m_stepMode && m_framesLeft == 0) return;
        --m_framesLeft;

        TRACY_PROFILE_SCOPE(runtime_scene_update);


        GetWorld().Get_System<oo::TransformSystem>()->Run(&GetWorld());
        GetWorld().Get_System<oo::AudioSystem>()->Run(&GetWorld());
        
        {
            TRACY_PROFILE_SCOPE(input_update);
            GetWorld().Get_System<InputSystem>()->Run(&GetWorld());
            TRACY_PROFILE_SCOPE_END();
        }

        {
            TRACY_PROFILE_SCOPE(scripts_update);
            GetWorld().Get_System<ScriptSystem>()->InvokeForAllEnabled("Update");
            TRACY_PROFILE_SCOPE_END();
        }

        {
            TRACY_PROFILE_SCOPE(scripts_update);
            GetWorld().Get_System<ScriptSystem>()->InvokeForAllEnabled("TickCoroutines");
            TRACY_PROFILE_SCOPE_END();
        }

        {
            TRACY_PROFILE_SCOPE(physics_runtime_update);
            GetWorld().Get_System<PhysicsSystem>()->RuntimeUpdate(timer::dt());
            TRACY_PROFILE_SCOPE_END();
        }
        
        {
            TRACY_PROFILE_SCOPE(UI_runtime_update);
            GetWorld().Get_System<oo::UISystem>()->RuntimeUpdate();
            TRACY_PROFILE_SCOPE_END();
        }

        /*{
            TRACY_PROFILE_SCOPE(physics_runtime_update);
            GetWorld().Get_System<TransformSystem>()->Run(&GetWorld());
            TRACY_PROFILE_SCOPE_END();
        }*/

        {
            TRACY_PROFILE_SCOPE(animation_update);
            GetWorld().Run_System<oo::Anim::AnimationSystem>();
            TRACY_PROFILE_SCOPE_END();
        }

            
        TRACY_PROFILE_SCOPE_END();

    }

    void RuntimeScene::LateUpdate()
    {
        TRACY_PROFILE_SCOPE(runtime_scene_late_update);
        Scene::LateUpdate();
        {
            TRACY_PROFILE_SCOPE(scripts_late_update);
            GetWorld().Get_System<ScriptSystem>()->InvokeForAllEnabled("LateUpdate");
            TRACY_PROFILE_SCOPE_END();
        }
        TRACY_PROFILE_SCOPE_END();
    }

    void RuntimeScene::Render()
    {
        TRACY_PROFILE_SCOPE(runtime_scene_rendering);
        Scene::Render();
        GetWorld().Get_System<oo::PhysicsSystem>()->RenderDebugColliders();
        TRACY_PROFILE_SCOPE_END();
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
        TRACY_PROFILE_SCOPE(runtime_scene_exit);
        Scene::Exit();

        StopSimulation();
        TRACY_PROFILE_SCOPE_END();
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
        TRACY_PROFILE_SCOPE(start_simulation);

        GetWorld().Get_System<oo::TransformSystem>()->UpdateSubTree(*GetRoot(), false);
        GetWorld().Get_System<ScriptSystem>()->StartPlay();

        /*GetWorld().GetSystem<oo::ScriptSystem>()->StartPlay();
        GetWorld().GetSystem<oo::TransformSystem>()->UpdateTransform();
        GetWorld().GetSystem<oo::PhysicsSystem>()->Init();
        GetWorld().GetSystem<oo::ParticleRenderingSystem>()->Init();
        GetWorld().GetSystem<oo::AudioSystem>()->Init();*/

        TRACY_PROFILE_SCOPE_END();
    }

    void RuntimeScene::StopSimulation()
    {
        TRACY_PROFILE_SCOPE(stop_simulation);

        GetWorld().Get_System<oo::ScriptSystem>()->StopPlay();
        //GetWorld().GetSystem<oo::ScriptSystem>()->StopPlay();

        TRACY_PROFILE_SCOPE_END();
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
