/************************************************************************************//*!
\file           EditorController.h
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
#pragma once

#include <string>
#include <memory>

#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/EventSystem/Event.h>

#include "RuntimeController.h"
#include "RuntimeScene.h"
#include "EditorScene.h"

//#include "App/Editor/Events/LoadProjectEvents.h"
#include "App/Editor/Events/OpenFileEvent.h"

#include "App/SceneHeader.h"
namespace oo
{
    class EditorController
    {
        //Editor related events
        //Note: only gets that succeed will be invoked
    public:
        class OnSimulateEvent : public Event
        {
        };
        class OnPauseEvent : public Event
        {
        };
        class OnStopEvent : public Event
        {
        };

    public:
        EditorController(SceneManager& sceneManager, RuntimeController& runtimeController, SCENE_STATE& activeState);

        void Simulate();
        void Pause();
        void Stop();

        void Init(SceneInfo startfile);
        void LoadScene(const std::string& file);
        /*bool GetActiveScenePaused() const;
        bool GetActiveSceneStepMode() const;*/

        std::weak_ptr<EditorScene> GetEditorScene() const;
        std::weak_ptr<RuntimeScene> GetRuntimeScene() const;
        
        // we can put static editor Camera publicly accesible here.
        inline static Camera EditorCamera;

    private:
        void OnLoadProjectEvent(LoadProjectEvent* loadProjEvent);
        void OnOpenFileEvent(OpenFileEvent* openFileEvent);
        void OnRuntimeSceneChange(Scene::OnInitEvent*);

        std::weak_ptr<EditorScene> m_editorScene = {};
        
        bool m_temporaryAdd = false;

        SCENE_STATE& m_activeState;
        SceneManager& m_sceneManager;
        RuntimeController& m_runtimeController;
    };

}

