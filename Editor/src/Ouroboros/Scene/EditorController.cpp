/************************************************************************************//*!
\file           EditorController.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          EditorController controls the flow between the two main scenes, 
                Editor scene and runtime scene

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "EditorController.h"

//#include "Project/EditorProject.h"
#include "Ouroboros/Scripting/ScriptSystem.h"
//#include "RuntimeController.h"
//#include "Ouroboros/Platform/Windows/WindowsWindow.h"
//#include "Ouroboros/Core/Timestep.h"

#include "Ouroboros/EventSystem/EventManager.h"

#include <filesystem>

namespace oo
{
    EditorController::EditorController(SceneManager& sceneManager, RuntimeController& runtimeController)
        : m_sceneManager { sceneManager }
        , m_runtimeController { runtimeController }
    {
        EventManager::Subscribe<EditorController, Scene::OnInitEvent>(this, &EditorController::OnRuntimeSceneChange);
        EventManager::Subscribe<EditorController, LoadProjectEvent>(this, &EditorController::OnLoadProjectEvent);
        EventManager::Subscribe<EditorController, OpenFileEvent>(this, &EditorController::OnOpenFileEvent);
    }

    void EditorController::OnLoadProjectEvent(LoadProjectEvent* loadProjEvent)
    {
        std::filesystem::path filename = loadProjEvent->m_startScene;
        SceneInfo sceneData{ filename.stem().string(), loadProjEvent->m_startScene };
        Init(sceneData);
    }

    void EditorController::OnOpenFileEvent(OpenFileEvent* openFileEvent)
    {
        if (openFileEvent->m_type == OpenFileEvent::FileType::PREFAB ||
            openFileEvent->m_type == OpenFileEvent::FileType::SCENE)
        {
            LoadScene(openFileEvent->m_filepath.string());
        }
    }

    void EditorController::Init(SceneInfo startfile)
    {   
        // check current scene is already loaded
        auto currentScene = m_editorScene.lock();
        if (currentScene != nullptr)
        {
            m_editorScene.reset();
            m_sceneManager.RemoveScene(currentScene->GetID());
        }

        auto [success, editor_key, editorScene] = m_sceneManager.CreateNewScene<EditorScene>(startfile.SceneName, startfile.LoadPath);
        ASSERT_MSG(!success, "Scene couldnt be loaded, is the file path passed in correct?");
        
        m_editorScene = editorScene;
        m_sceneManager.ChangeScene(m_editorScene);
    }

    void EditorController::Simulate()
    {
        // if in editor mode
        if (m_activeState == STATE::EDITING)
        {
            // check for errors in scripts
            if (oo::ScriptSystem::DisplayErrors())
            {
                LOG_ERROR("Fix Compile Time Errors before entering play mode");
                return;
            }
            
            OnSimulateEvent onSimulateEvent;
            EventManager::Broadcast(&onSimulateEvent);

            // set active scene to runtime scene
            m_activeState = STATE::RUNNING;

            //Force save when you press play [ not sure if intended ]
            m_editorScene.lock()->Save();

            m_temporaryAdd = !m_runtimeController.HasScene(m_editorScene.lock()->GetID());
            // add selected path as load path
            if (m_temporaryAdd)
                m_runtimeController.AddLoadPath(m_editorScene.lock()->GetSceneName(), m_editorScene.lock()->GetFilePath());

            // Generate all the scenes on run
            m_runtimeController.GenerateScenes();

            // Change runtime scene to currently selected
            m_runtimeController.ChangeRuntimeScene(m_editorScene.lock()->GetSceneName());
        }
        // if in runtime 
        else if (m_activeState == STATE::RUNNING)
        {
            // check if its paused
            if (m_runtimeScene.lock()->IsPaused())
            {
                m_runtimeScene.lock()->ResumeSimulation();
            }
            //else // is either in step mode or all good.
            //{
            //    m_runtimeScene.lock()->StopStepMode();
            //}
        }

    }

    void EditorController::Pause()
    {
        // pause only applies when in runtime scene
        if (m_activeState == STATE::EDITING) return;

        OnPauseEvent onPauseEvent;
        EventManager::Broadcast(&onPauseEvent);

        if (m_runtimeScene.lock()->IsStepMode())
        {
            m_runtimeScene.lock()->ProcessFrame(1);
        }
        else if (!m_runtimeScene.lock()->IsPaused()) // check if runtime scene is currently paused.
        {
            m_runtimeScene.lock()->PauseSimulation();
        }
        //else // scene is already paused, proceed to step mode.
        //{
        //    m_runtimeScene.lock()->ProcessFrame(1);
        //}
    }

    void EditorController::LoadScene(const std::string& newPath)
    {
        LOG_TRACE("Loading new Scene at path {0}", newPath);

        if (m_activeState == STATE::RUNNING) return;

        // Remove all old scenes when loading a new one
        m_runtimeController.RemoveScenes();

        // remove the current scene
        if (m_temporaryAdd)
        {
            m_temporaryAdd = false;
            m_runtimeController.RemoveLoadPath(m_editorScene.lock()->GetSceneName());
        }

        // set editor to new file path
        m_editorScene.lock()->SetNewPath(newPath);
        // reload active scene [ this scene ]
        m_sceneManager.ReloadActiveScene();

        // reset runtime Scene to be nullptr
        m_runtimeScene.lock() = nullptr;
    }

    bool EditorController::GetActiveScenePaused() const
    {
        return m_activeState == STATE::RUNNING && m_runtimeScene.lock()->IsPaused();
    }

    bool EditorController::GetActiveSceneStepMode() const
    {
        return m_activeState == STATE::RUNNING && m_runtimeScene.lock()->IsStepMode();
    }

    void EditorController::Stop()
    {
        // stop only applies when in runtime scene
        if (m_activeState == STATE::EDITING) return;

        OnStopEvent onStopEvent;
        EventManager::Broadcast(&onStopEvent);

        // Reset the important things that the user can modify 
        //{
        //    // Show cursor
        //    reinterpret_cast<oo::WindowsWindow&>(oo::Application::Get().GetWindow()).ShowCursor(true);
        //    // Reset Timescale
        //    oo::Timestep::TimeScale = 1.0;
        //}

        m_runtimeController.RemoveScenes();
        
        // remove the current scene if it was added temporarily
        if (m_temporaryAdd)
        {
            m_temporaryAdd = false;
            m_runtimeController.RemoveLoadPath(m_editorScene.lock()->GetSceneName());
        }

        m_activeState = STATE::EDITING;
        m_sceneManager.ChangeScene(m_editorScene);
    }

    void EditorController::OnRuntimeSceneChange(Scene::OnInitEvent*)
    {
        // if the scene is active, it has to be a runtime scene!
        if (m_activeState == STATE::RUNNING)
            m_runtimeScene = std::static_pointer_cast<RuntimeScene>(m_sceneManager.GetActiveScene().lock());
    }

}