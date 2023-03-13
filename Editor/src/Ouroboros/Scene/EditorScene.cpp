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
            Application::Get().GetWindow().ShowCursor(true);
            oo::timer::set_timescale(1.0);
        }

        Scene::Init();

        //Register All Systems
        {
            TRACY_PROFILE_SCOPE_N(editor_registration);
            GetWorld().Add_System<Anim::AnimationSystem>()->Init(&GetWorld(), this);
            GetWorld().Add_System<oo::PhysicsSystem>()->Init(this);
            GetWorld().Add_System<oo::UISystem>(GetGraphicsWorld(), this)->Init();
            TRACY_PROFILE_SCOPE_END();
        }

        // Some system setup code
        {
            // set default debug draws
            GetWorld().Get_System<UISystem>()->UIDebugDraw = true;
            GetWorld().Get_System<UISystem>()->UIDebugPrint = false;
            GetWorld().Get_System<UISystem>()->UIDebugRaycast = false;
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

        // Functions to run after the file and scene has been loaded.
        {
            TRACY_PROFILE_SCOPE_N(post_load_scene_init);

            GetWorld().Get_System<TransformSystem>()->PostLoadSceneInit();
            GetWorld().Get_System<PhysicsSystem>()->PostLoadSceneInit();
            GetWorld().Get_System<SkinMeshRendererSystem>()->PostLoadScene();
            GetWorld().Get_System<RendererSystem>()->PostSceneLoadInit();
            GetWorld().Get_System<UISystem>()->PostSceneLoadInit();
            
            TRACY_PROFILE_SCOPE_END();
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void EditorScene::Update()
    {
        TRACY_PROFILE_SCOPE_NC(editor_scene_update, tracy::Color::Azure);
        OPTICK_EVENT();

        // jobs in the same phase should not depend on one another's run order.
        jobsystem::job phase_one{};
        jobsystem::submit(phase_one, [&]() {
            GetWorld().Get_System<oo::TransformSystem>()->Run(&GetWorld());
            });
        jobsystem::launch_and_wait(phase_one);

        jobsystem::job phase_two{};
        jobsystem::submit(phase_two, [&]() {
            GetWorld().Get_System<oo::AudioSystem>()->Run(&GetWorld());
            });
        jobsystem::submit(phase_two, [&]() {
            GetWorld().Get_System<PhysicsSystem>()->EditorUpdate(timer::dt());
            });
        jobsystem::submit(phase_two, [&]() {
            GetWorld().Get_System<oo::UISystem>()->EditorUpdate();
            });
        
        jobsystem::launch_and_wait(phase_two);
        
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

        oGFX::DebugDraw::DrawYGrid(gridSize, gridIncrement);
        oGFX::DebugDraw::DrawYGrid(gridSize, 1.0f, oGFX::Colors::RED);

        Scene::Render();
        
        //debug draw for physics
        GetWorld().Get_System<oo::PhysicsSystem>()->RenderDebugColliders();
        
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