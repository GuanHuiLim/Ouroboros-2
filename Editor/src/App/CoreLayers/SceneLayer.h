/************************************************************************************//*!
\file           SceneLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 1, 2022
\brief          Defines a layer that will be running in the both the editor and
                the final distribution build that contains the main rendering scene

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <SceneManagement/include/SceneManager.h>
#include "Ouroboros/Core/Layer.h"

#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Scene/EditorController.h"

#include "Ouroboros/EventSystem/Event.h"
#include "App/SceneHeader.h"

class ToolbarButtonEvent;

namespace oo
{
    // forward declare event
    struct GetCurrentSceneEvent;
    struct GetCurrentSceneStateEvent;

    class SceneLayer final : public oo::Layer
    {
    private:
        SceneManager& m_sceneManager;
        RuntimeController m_runtimeController;
#ifdef OO_EDITOR 
        EditorController m_editorController;
#endif
    public:

        SceneLayer(SceneManager& sceneManager);
        virtual ~SceneLayer() = default;
        
        void OnAttach() override final;
        void OnDetach() override final;
        void OnUpdate() override final;

        inline SCENE_STATE GetActiveState() const { return m_activeState; };
        bool GetActiveScenePaused() const;
        bool GetActiveSceneStepMode() const;

    private:
        void OnGetCurrentSceneEvent(GetCurrentSceneEvent* e);
        void OnGetCurrentSceneStateEvent(GetCurrentSceneStateEvent* e);
#ifdef OO_EDITOR
        void OnToolbarButtonEvent(ToolbarButtonEvent* e);
#endif
    
    private:
        SCENE_STATE m_activeState;
    };

}
