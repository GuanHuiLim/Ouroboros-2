/************************************************************************************//*!
\file           EditorController.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2021
\brief          EditorController controls the flow between the two main scenes, 
                Editor scene and runtime scene

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "EditorController.h"

//#include "Project/EditorProject.h"
//#include "Ouroboros/Scripting/ScriptSystem.h"
//#include "RuntimeController.h"
//#include "Ouroboros/Platform/Windows/WindowsWindow.h"
//#include "Ouroboros/Core/Timestep.h"

#include "Ouroboros/EventSystem/EventManager.h"
namespace oo
{

    void EditorController::Init()
    {
        auto [success, editor_key, editorScene] = m_sceneManager.CreateNewScene<EditorScene>("Test Scene", "please fill up with a valid filepath");
        ASSERT(!success);

        m_editorScene = editorScene;
        EventManager::Subscribe<EditorController, Scene::OnInitEvent>(this, &EditorController::OnRuntimeSceneChange);

        m_sceneManager.ChangeScene(m_editorScene);

        //m_editorScene = std::make_shared<EditorScene>("");
        //SceneManager::ChangeScene(m_editorScene);
        //SceneManager::GetInstance().OnActiveSceneChange += [] { OnRuntimeSceneChange(); };
    }

    void EditorController::Simulate()
    {
        // if in editor mode
        if (m_activeState == STATE::EDITING)
        {
            //// check for errors in scripts
            //if (oo::ScriptSystem::CheckErrors(true))
            //{
            //    //LOG_ERROR("Fix Compile Time Errors before entering play mode");
            //    return;
            //}

            // set active scene to runtime scene
            m_activeState = STATE::RUNNING;

            ////Force save when you press play [ not sure if intended ]
            //m_editorScene->Save();
            m_editorScene->Save();

            m_temporaryAdd = !m_runtimeController.HasScene(m_editorScene->GetSceneName());
            // add selected path as load path
            if (m_temporaryAdd)
                m_runtimeController.AddLoadPath(m_editorScene->GetSceneName(), m_editorScene->GetFilePath());

            // Generate all the scenes on run
            //RuntimeController::GenerateScenes();
            m_runtimeController.GenerateScenes();

            // Change runtime scene to currently selected
            //RuntimeController::ChangeRuntimeScene(m_editorScene->GetSceneName());
            m_runtimeController.ChangeRuntimeScene(m_editorScene->GetSceneName());
        }
        // if in runtime 
        else if (m_activeState == STATE::RUNNING)
        {
            // check if its paused
            if (m_runtimeScene->IsPaused())
            {
                m_runtimeScene->ResumeSimulation();
            }
            //else // is either in step mode or all good.
            //{
            //    m_runtimeScene->StopStepMode();
            //}
        }

    }

    void EditorController::Pause()
    {
        // pause only applies when in runtime scene
        if (m_activeState == STATE::EDITING) return;

        if (m_runtimeScene->IsStepMode())
        {
            m_runtimeScene->ProcessFrame(1);
        }
        else if (!m_runtimeScene->IsPaused()) // check if runtime scene is currently paused.
        {
            m_runtimeScene->PauseSimulation();
        }
        //else // scene is already paused, proceed to step mode.
        //{
        //    m_runtimeScene->ProcessFrame(1);
        //}

    }

    void EditorController::LoadScene(const std::string& newPath)
    {
        LOG_TRACE("Loading new Scene at path {0}", newPath);
        if (m_activeState == STATE::RUNNING) return;

        // Remove all old scenes when loading a new one
        //RuntimeController::RemoveScenes();
        m_runtimeController.RemoveScenes();
        // remove the current scene
        if (m_temporaryAdd)
        {
            m_temporaryAdd = false;
            //RuntimeController::RemoveLoadPath(m_editorScene->GetSceneName())
            m_runtimeController.RemoveLoadPath(m_editorScene->GetSceneName());
        }

        // change editor scene
        //m_editorScene->ReloadWorldWithPath(newPath);
        m_editorScene->ReloadSceneWithPath(newPath);

        // reset runtime Scene to be nullptr
        m_runtimeScene = nullptr;
    }

    bool EditorController::GetActiveScenePaused() const
    {
        return m_activeState == STATE::RUNNING && m_runtimeScene->IsPaused();
    }

    bool EditorController::GetActiveSceneStepMode() const
    {
        return m_activeState == STATE::RUNNING && m_runtimeScene->IsStepMode();
    }

    void EditorController::Stop()
    {
        // stop only applies when in runtime scene
        if (m_activeState == STATE::EDITING) return;

        // Reset the important things that the user can modify 
        //{
        //    // Show cursor
        //    reinterpret_cast<oo::WindowsWindow&>(oo::Application::Get().GetWindow()).ShowCursor(true);
        //    // Reset Timescale
        //    oo::Timestep::TimeScale = 1.0;
        //}

        //RuntimeController::RemoveScenes();
        m_runtimeController.RemoveScenes();
        // remove the current scene
        if (m_temporaryAdd)
        {
            m_temporaryAdd = false;
            m_runtimeController.RemoveLoadPath(m_editorScene->GetSceneName());
        }

        m_activeState = STATE::EDITING;
        m_sceneManager.ChangeScene(m_editorScene);
    }

    void EditorController::OnRuntimeSceneChange(Scene::OnInitEvent*)
    {
        // if the scene is active, it has to be a runtime scene!
        if (m_activeState == STATE::RUNNING)
            m_runtimeScene = std::static_pointer_cast<RuntimeScene>(m_sceneManager.GetActiveScene());

        /*if (m_activeScene == STATE::RUNNING)
            m_runtimeScene = std::static_pointer_cast<RuntimeScene>(SceneManager::GetActiveScenePointer());*/
    }
}