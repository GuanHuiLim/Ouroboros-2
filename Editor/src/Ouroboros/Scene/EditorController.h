/************************************************************************************//*!
\file           EditorController.h
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
#pragma once

#include <string>
#include <memory>

//#include <Ouroboros.h>

//#include "Scene/EditorScene.h"
//#include "Scene/RuntimeScene.h"
//#include "Scene/SceneManager.h"

#include "SceneManager.h"
#include "RuntimeScene.h"
#include "EditorScene.h"

namespace oo
{
    class EditorController
    {
    public:
        EditorController(SceneManager& sceneManager) : m_sceneManager{ sceneManager } {};

        void Init();
        void Simulate();
        void Pause();
        void Stop();
        void LoadScene(const std::string& file);
        bool GetActiveScenePaused() const;
        bool GetActiveSceneStepMode() const;

        enum class STATE
        {
            EDITING,
            RUNNING,
        };
        STATE GetActiveState() { return m_activeState; };

    private:
        void OnRuntimeSceneChange();

        std::shared_ptr<EditorScene> m_editorScene = nullptr;
        std::shared_ptr<RuntimeScene> m_runtimeScene = nullptr;

        STATE m_activeState = STATE::EDITING;
        bool m_temporaryAdd = false;

        SceneManager& m_sceneManager;
    };

}

