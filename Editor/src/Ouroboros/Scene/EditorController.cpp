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
#include "Ouroboros/Scripting/ScriptManager.h"
//#include "RuntimeController.h"
//#include "Ouroboros/Platform/Windows/WindowsWindow.h"
//#include "Ouroboros/Core/Timestep.h"

#include "Ouroboros/EventSystem/EventManager.h"

#include <filesystem>

namespace oo
{
    EditorController::EditorController(SceneManager& sceneManager, RuntimeController& runtimeController, SCENE_STATE& activeState)
        : m_sceneManager { sceneManager }
        , m_runtimeController { runtimeController }
        , m_activeState { activeState }
    {
        m_activeState = SCENE_STATE::EDITING;
        EventManager::Subscribe<EditorController, Scene::OnInitEvent>(this, &EditorController::OnRuntimeSceneChange);
        EventManager::Subscribe<EditorController, LoadProjectEvent>(this, &EditorController::OnLoadProjectEvent);
        EventManager::Subscribe<EditorController, OpenFileEvent>(this, &EditorController::OnOpenFileEvent);
    }

    void EditorController::OnLoadProjectEvent(LoadProjectEvent* loadProjEvent)
    {
        LOG_TRACE("Editor Controller Received and Resolving Load Project Event");

        std::filesystem::path filename = loadProjEvent->m_startScene;
        SceneInfo sceneData{ filename.stem().string(), loadProjEvent->m_startScene };
        Init(sceneData);
    }

    void EditorController::OnOpenFileEvent(OpenFileEvent* openFileEvent)
    {
        LOG_TRACE("Editor Controller Received and Resolving Open File Event");

        if (openFileEvent->m_type == OpenFileEvent::FileType::PREFAB ||
            openFileEvent->m_type == OpenFileEvent::FileType::SCENE)
        {
            LoadScene(openFileEvent->m_filepath.string());
        }
    }

    void EditorController::Init(SceneInfo startfile)
    {   
        // check current scene is already loaded
        auto currentScene = m_editorScene;
        if (currentScene != nullptr)
        {
            m_editorScene.reset();
            m_sceneManager.RemoveScene(currentScene->GetID());
        }

        m_editorSceneName = "Editor Scene: " + startfile.SceneName;
        m_editorSceneNameAtRuntime = m_editorSceneName + " at runtime";
        // initialize editor scene : make sure it uses the name of the file
        auto [success, editor_key, editorScene] = m_sceneManager.CreateNewScene<EditorScene>(m_editorSceneName, "Editor Scene", startfile.LoadPath);
        ASSERT_MSG(!success, "Scene couldnt be loaded, is the file path passed in correct?");
        m_editorScene = editorScene.lock();
        m_sceneManager.ChangeScene(m_editorScene);
    }

    void EditorController::Simulate()
    {
        // if in editor mode
        if (m_activeState == SCENE_STATE::EDITING)
        {
            // check for errors in scripts
            if (oo::ScriptManager::DisplayErrors())
            {
                LOG_ERROR("Fix Compile Time Errors before entering play mode");
                return;
            }
            
            OnSimulateEvent onSimulateEvent;
            EventManager::Broadcast(&onSimulateEvent);

            // set active scene to runtime scene
            m_activeState = SCENE_STATE::RUNNING;

            auto editor_scene = m_editorScene;
            ASSERT_MSG(editor_scene == nullptr, "editor scene shouldn't be null now!");

            //Force save when you press play [ not sure if intended ]
            editor_scene->Save();

            auto editor_filepath = editor_scene->GetFilePath();
            // determine if runtime controller have a scene with current loadpath added
           // m_temporaryAdd = !m_runtimeController.HasSceneWithName(editor_scene->GetSceneName());
            // add selected path as load path if it wasn't added
            //if (m_temporaryAdd)
            
            m_runtimeController.AddLoadPath(m_editorSceneNameAtRuntime, editor_filepath);

            // Generate all the scenes on run
            m_runtimeController.GenerateScenes();

            // Change runtime scene to currently selected
            m_runtimeController.ChangeRuntimeScene(m_editorSceneNameAtRuntime);
        }
        // if in runtime 
        else if (m_activeState == SCENE_STATE::RUNNING)
        {
            auto runtime_scene = m_runtimeController.GetRuntimeScene().lock();
            ASSERT_MSG(runtime_scene == nullptr, "runtime scene shouldn't be null now!");
            // check if its paused
            if (runtime_scene->IsPaused())
            {
                runtime_scene->ResumeSimulation();
                LOG_TRACE("Resuming simulation of runtime scene \"{0}\"", runtime_scene->GetSceneName());
            }
            //else // is either in step mode or all good.
            //{
            //    m_runtimeController.GetRuntimeScene().lock()->StopStepMode();
            //}
        }

    }

    void EditorController::Pause()
    {
        // pause only applies when in runtime scene
        if (m_activeState == SCENE_STATE::EDITING) return;

        OnPauseEvent onPauseEvent;
        EventManager::Broadcast(&onPauseEvent);

        auto runtime_scene = m_runtimeController.GetRuntimeScene().lock();
        ASSERT_MSG(runtime_scene == nullptr, "runtime scene shouldn't be null!");

        if (runtime_scene->IsStepMode())
        {
            runtime_scene->ProcessFrame(1);
            LOG_TRACE("Stepping through runtime scene \"{0}\"", runtime_scene->GetSceneName());
        }
        else if (!runtime_scene->IsPaused()) // check if runtime scene is currently paused.
        {
            runtime_scene->PauseSimulation();
            LOG_TRACE("Paused runtime scene \"{0}\"", runtime_scene->GetSceneName());
        }
        //else // scene is already paused, proceed to step mode.
        //{
        //    m_runtimeController.GetRuntimeScene().lock()->ProcessFrame(1);
        //}
    }

    void EditorController::LoadScene(const std::string& newPath)
    {
        if (m_activeState == SCENE_STATE::RUNNING) return;

        LOG_TRACE("Editor Scene Loading New Scene at path \"{0}\"", newPath);

        // Remove all old scenes when loading a new one
        m_runtimeController.RemoveScenes();

        // remove the current scene
        //if (m_temporaryAdd)
        //{
        //    m_temporaryAdd = false;
            m_runtimeController.RemoveLoadPathByName(m_editorSceneNameAtRuntime);
        //}

        // set editor to new file path
        m_editorScene->SetNewPath(newPath);
        // reload active scene [ this scene ]
        m_sceneManager.ReloadActiveScene();
        // reset runtime Scene to be nullptr
        m_runtimeController.GetRuntimeScene().lock() = nullptr;
    }

    std::weak_ptr<EditorScene> EditorController::GetEditorScene() const { return m_editorScene; }

    std::weak_ptr<RuntimeScene> EditorController::GetRuntimeScene() const { return m_runtimeController.GetRuntimeScene(); }

    void EditorController::Stop()
    {
        // stop only applies when in runtime scene
        if (m_activeState == SCENE_STATE::EDITING) return;

        OnStopEvent onStopEvent;
        EventManager::Broadcast(&onStopEvent);

        // Reset the important things that the user can modify 
        //{
        //    // Show cursor
        //    reinterpret_cast<oo::WindowsWindow&>(oo::Application::Get().GetWindow()).ShowCursor(true);
        //    // Reset Timescale
        //    oo::Timestep::TimeScale = 1.0;
        //}

        LOG_INFO("Changing to Editor Scene named \"{0}\"!", m_editorSceneName);

        m_runtimeController.RemoveScenes();
        
        // remove the current scene if it was added temporarily
        //if (m_temporaryAdd)
        //{
        //    m_temporaryAdd = false;
            m_runtimeController.RemoveLoadPathByName(m_editorSceneNameAtRuntime);
        //}

        m_activeState = SCENE_STATE::EDITING;
        m_sceneManager.ChangeScene(m_editorScene);
    }

    void EditorController::OnRuntimeSceneChange(Scene::OnInitEvent*)
    {
        // if the scene is active, it has to be a runtime scene!
        if (m_activeState == SCENE_STATE::RUNNING)
        {
            auto new_runtimescene = std::static_pointer_cast<RuntimeScene>(m_sceneManager.GetActiveScene().lock());
            m_runtimeController.SetRuntimeScene(new_runtimescene);
        }
    }

}