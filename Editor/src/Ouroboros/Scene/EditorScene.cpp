/************************************************************************************//*!
\file           EditorScene.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          EditorScene describes the scene when its meant for editing.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "EditorScene.h"
#include <filesystem>
#include "OO_Vulkan/src/DebugDraw.h"

//#include "Editor.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Ouroboros/Physics/PhysicsSystem.h"
#include "Ouroboros/Vulkan/RendererSystem.h"
#include "Ouroboros/Vulkan/SkinRendererSystem.h"
#include "Ouroboros/Animation/AnimationSystem.h"
#include "Ouroboros/Transform/TransformSystem.h"
#include "Ouroboros/Audio/AudioSystem.h"
#include "Ouroboros/Core/Application.h"
#include "Ouroboros/UI/UISystem.h"

//optick
#include "optick.h"

namespace oo
{
    EditorScene::EditorScene(std::string const& name, std::string const& filepath)
        : Scene{ name }
    {
        if (!filepath.empty())
            SetFilePath(filepath);
    }

    void EditorScene::Init()
    {
        TRACY_PROFILE_SCOPE_NC(editor_scene_init, tracy::Color::Aqua);
        OPTICK_EVENT();
        
        // Reset some global window related settings here ... might be a bit scuffed and needa shift elsewhere
        {
            // Unlock the mouse if it was locked in play mode
            Application::Get().GetWindow().SetMouseLockState(false);
            oo::timer::set_timescale(1.0);
        }

        Scene::Init();

        //Register All Systems
        {
            TRACY_PROFILE_SCOPE_N(editor_registration);
            GetWorld().Add_System<Anim::AnimationSystem>()->Init(&GetWorld(), this);
            GetWorld().Add_System<oo::PhysicsSystem>()->Init(this);
            GetWorld().Add_System<oo::UISystem>(this);
            //bool wantDebug = true;

            //GetWorld().Get_System<Anim::AnimationSystem>()->CreateAnimationTestObject();
            //TRACY_PROFILE_SCOPE_N(registration);
            /*GetWorld().RegisterSystem<PrefabComponentSystem>();
            GetWorld().RegisterSystem<EditorComponentSystem>();

            GetWorld().RegisterSystem<oo::Renderer2DSystem>(*oo::EditorCamera::g_editorCam, wantDebug);
            GetWorld().RegisterSystem<oo::ParticleRenderingSystem>();
            GetWorld().RegisterSystem<oo::QuadtreeSystem>();
            GetWorld().RegisterSystem<oo::AnimatorSystem>();
            GetWorld().RegisterSystem<oo::UIRenderingSystem>(wantDebug);
            GetWorld().RegisterSystem<oo::VideoSystem>();
            auto scriptSystem = GetWorld().RegisterSystem<oo::ScriptSystem>();
            GetWorld().RegisterSystem<oo::PhysicsSystem>();
            GetWorld().RegisterSystem<oo::UISystem>();
            GetWorld().RegisterSystem<oo::AudioSystem>();

            scriptSystem->SetCallbackInvokes();*/
            TRACY_PROFILE_SCOPE_END();
        }

        // Some system setup code
        {
            // set default debug draws
            GetWorld().Get_System<PhysicsSystem>()->ColliderDebugDraw = true;
            GetWorld().Get_System<RendererSystem>()->CameraDebugDraw = true;
            GetWorld().Get_System<RendererSystem>()->LightsDebugDraw = true;
        }

        // if filepath is a valid file path
        if (std::filesystem::exists(GetFilePath()))
        {
            TRACY_PROFILE_SCOPE_N(loading_world);

            LoadFromFile();

            TRACY_PROFILE_SCOPE_END();
        }
        GetWorld().Get_System<SkinMeshRendererSystem>()->PostLoadScene();

        TRACY_PROFILE_SCOPE_END();
    }

    void EditorScene::Update()
    {
        TRACY_PROFILE_SCOPE_NC(editor_scene_update, tracy::Color::Azure);
        OPTICK_EVENT();

        GetWorld().Get_System<oo::TransformSystem>()->Run(&GetWorld());
        GetWorld().Get_System<oo::AudioSystem>()->Run(&GetWorld());
        GetWorld().Get_System<PhysicsSystem>()->EditorUpdate(timer::dt());
        GetWorld().Get_System<oo::UISystem>()->EditorUpdate();

        {
            //TRACY_PROFILE_SCOPE_NC(editor_scene_update, tracy::Color::Azure);

            {
                /*TRACY_PROFILE_SCOPE_N(physics_editor_update);
                GetWorld().GetSystem<oo::PhysicsSystem>()->EditorModeUpdate(dt);
                TRACY_PROFILE_SCOPE_END();*/
            }

            {
                /*TRACY_PROFILE_SCOPE_N(transform_update);
                GetWorld().GetSystem<oo::TransformSystem>()->Update();
                TRACY_PROFILE_SCOPE_END();*/
            }

            {
                /*TRACY_PROFILE_SCOPE_N(animator_update);
                GetWorld().GetSystem<oo::AnimatorSystem>()->Update(dt);
                TRACY_PROFILE_SCOPE_END();*/
            }

            {
                /*TRACY_PROFILE_SCOPE_N(ui_update);
                GetWorld().GetSystem<oo::UISystem>()->EditorUpdate();
                TRACY_PROFILE_SCOPE_END();*/
            }

            {
                /*ZoneScopedN(particles_update);
                GetWorld().GetSystem<oo::ParticleRenderingSystem>()->Update(dt);*/
            }

            {
                /*TRACY_TRACK_PERFORMANCE(video_update);
                GetWorld().GetSystem<oo::VideoSystem>()->Update(dt);*/
            }

        }
        
        TRACY_PROFILE_SCOPE_END();
    }

    void EditorScene::LateUpdate()
    {
        TRACY_PROFILE_SCOPE_NC(editor_scene_late_update, tracy::Color::Azure2);
        Scene::LateUpdate();
        TRACY_PROFILE_SCOPE_END();
    }

    void EditorScene::Render()
    {
        TRACY_PROFILE_SCOPE_NC(editor_scene_rendering, tracy::Color::Cyan);

        constexpr float gridSize = 100.0f;
        constexpr float gridIncrement = 1.0f / 5.0f;

        DebugDraw::DrawYGrid(gridSize, gridIncrement);
        DebugDraw::DrawYGrid(gridSize, 1.0f, oGFX::Colors::RED);

        Scene::Render();
        
        GetWorld().Get_System<oo::PhysicsSystem>()->RenderDebugColliders();

        //constexpr const char* const rendering = "Overall Rendering";
        //constexpr const char* const text_rendering = "Text Rendering";
        //constexpr const char* const renderer2d_rendering = "Renderer2D Rendering";
        {
            //TRACY_PROFILE_SCOPE_NC(rendering, tracy::Color::Cyan);

            {
                /*TRACY_PROFILE_SCOPE_N(text_rendering);
                GetWorld().GetSystem<oo::UIRenderingSystem>()->Render();
                TRACY_PROFILE_SCOPE_END();*/
            }

            {
                /*TRACY_PROFILE_SCOPE_N(renderer2d_rendering);
                GetWorld().GetSystem<oo::ParticleRenderingSystem>()->Render();
                GetWorld().GetSystem<oo::Renderer2DSystem>()->Render();
                TRACY_PROFILE_SCOPE_END();*/
            }
            //TRACY_PROFILE_SCOPE_END();
        }
        //TRACY_DISPLAY_PERFORMANCE_SELECTED(rendering);
        
        TRACY_PROFILE_SCOPE_END();
    }

    void EditorScene::Exit()
    {
        TRACY_PROFILE_SCOPE_NC(editor_scene_exit, tracy::Color::Cyan2);
        Scene::Exit();
        TRACY_PROFILE_SCOPE_END();

        // DOES not autosave by default anymore
        //SaveToFile();
    }

    void EditorScene::SetNewPath(std::string const& filepath)
    {
        SetFilePath(filepath);
        //ReloadScene();
    }

    void EditorScene::Save()
    {
        TRACY_PROFILE_SCOPE_NC(editor_save_to_file, tracy::Color::Cyan3);
        SaveToFile();
        TRACY_PROFILE_SCOPE_END();
    }

}