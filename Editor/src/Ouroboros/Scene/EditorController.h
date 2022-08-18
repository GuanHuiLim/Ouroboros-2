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
        EditorController(SceneManager& sceneManager, RuntimeController& runtimeController);


        void Simulate();
        void Pause();
        void Stop();

        void Init(SceneInfo startfile);
        void LoadScene(const std::string& file);
        bool GetActiveScenePaused() const;
        bool GetActiveSceneStepMode() const;

        enum class STATE
        {
            EDITING,
            RUNNING,
        };

        STATE GetActiveState() const { return m_activeState; };

    private:
        void OnLoadProjectEvent(LoadProjectEvent* loadProjEvent);
        void OnRuntimeSceneChange(Scene::OnInitEvent*);

        std::weak_ptr<EditorScene> m_editorScene = {};
        std::weak_ptr<RuntimeScene> m_runtimeScene = {};

        STATE m_activeState = STATE::EDITING;
        bool m_temporaryAdd = false;

        SceneManager& m_sceneManager;
        RuntimeController& m_runtimeController;
    };

}

