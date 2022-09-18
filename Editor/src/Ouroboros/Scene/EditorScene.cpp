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

//#include "Editor.h"
#include "Ouroboros/TracyProfiling/OO_TracyProfiler.h"
#include "Ouroboros/Physics/PhysicsSystem.h"

namespace oo
{
    EditorScene::EditorScene(std::string const& filepath)
        : Scene{ "Editor Scene" }
    {
        if (!filepath.empty())
            SetFilePath(filepath);
    }

    void EditorScene::Init()
    {
        Scene::Init();

        constexpr const char* const editor_scene_init = "Editor scene init";
        //constexpr const char* const registration = "registration";
        //constexpr const char* const loading_world = "loading world";
        {
            TRACY_PROFILE_SCOPE_NC(editor_scene_init, tracy::Color::Aqua);
            //Register All Systems
            {
                GetWorld().Add_System<oo::PhysicsSystem>()->Init();
                //bool wantDebug = true;

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
                //TRACY_PROFILE_SCOPE_END();
            }

            // if filepath is a valid file path
            if (std::filesystem::exists(GetFilePath()))
            {
                {
                    //TRACY_PROFILE_SCOPE_N(loading_world);
                    
                    // load from that file
                    LoadFromFile();

                    //TRACY_PROFILE_SCOPE_END();
                }
            }

            TRACY_PROFILE_SCOPE_END();
        }

    }

    void EditorScene::Update()
    {
        Scene::Update();

        GetWorld().Get_System<PhysicsSystem>()->RuntimeUpdate(timer::dt());

        //constexpr const char* const editor_scene_update = "Editor Scene Update";
        //constexpr const char* const physics_editor_update = "physics editor mode update";
        //constexpr const char* const transform_update = "transform update";
        //constexpr const char* const particles_update = "Particles update";
        //constexpr const char* const animator_update = "animator update";
        //constexpr const char* const ui_update = "ui update";
        //constexpr const char* const video_update = "Video update";
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

            //TRACY_PROFILE_SCOPE_END();
        }

    }

    void EditorScene::LateUpdate()
    {
        Scene::LateUpdate();
    }

    void EditorScene::Render()
    {
        Scene::Render();

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
    }

    void EditorScene::Exit()
    {
        Scene::Exit();

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
        SaveToFile();
    }

}