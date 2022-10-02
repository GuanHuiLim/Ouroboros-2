/************************************************************************************//*!
\file           SceneLayer.cpp
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
#include "pch.h"
#include "SceneLayer.h"

#include "Ouroboros/Core/Input.h"
#include "Ouroboros/EventSystem/EventManager.h"

#include "App/Editor/Events/ToolbarButtonEvent.h"
#include "Ouroboros/EventSystem/EventTypes.h"

#include "App/Editor/Utility/ImGuiManager.h"

namespace oo
{
    SceneLayer::SceneLayer(SceneManager& sceneManager)
        : m_sceneManager{ sceneManager }
        , m_runtimeController{ m_sceneManager }
#ifdef OO_EDITOR
        , m_editorController{ m_sceneManager, m_runtimeController }
#endif
        , Layer("Scene Management Layer")
    {
        // This should be the only subscriber for this event!
        EventManager::Subscribe<SceneLayer, GetCurrentSceneEvent>(this, &SceneLayer::OnGetCurrentSceneEvent);

#ifdef OO_EDITOR
        EventManager::Subscribe<SceneLayer, ToolbarButtonEvent>(this, &SceneLayer::OnToolbarButtonEvent);
        ImGuiManager::s_runtime_controller = &m_runtimeController;
#endif

    }

    void SceneLayer::OnAttach()
    {
        m_sceneManager.Init();
    }

    void SceneLayer::OnDetach()
    {
        m_sceneManager.Terminate();
    }

    void SceneLayer::OnUpdate()
    {
        m_sceneManager.Update();
    }

    void SceneLayer::OnGetCurrentSceneEvent(GetCurrentSceneEvent* e)
    {
#ifdef OO_EDITOR
        switch (m_editorController.GetActiveState())
        {
        case EditorController::STATE::EDITING:
            e->CurrentEditorScene = m_editorController.GetEditorScene().lock();
            e->CurrentRuntimeScene = nullptr;
            e->CurrentScene = std::static_pointer_cast<Scene>(e->CurrentEditorScene);
            e->IsEditor = true;
            break;

        case EditorController::STATE::RUNNING:
            e->CurrentEditorScene = nullptr;
            e->CurrentRuntimeScene = m_editorController.GetRuntimeScene().lock();
            e->CurrentScene = std::static_pointer_cast<Scene>(e->CurrentRuntimeScene);
            e->IsEditor = false;
            break;
        }
#else
        //TODO!
        UNREFERENCED(e);
        m_runtimeController;

#endif
    }

    void SceneLayer::OnToolbarButtonEvent(ToolbarButtonEvent* e)
    {
#ifdef OO_EDITOR
        switch (e->m_buttonType)
        {
        case ToolbarButtonEvent::ToolbarButton::PLAY:
            m_editorController.Simulate();
            break;
        case ToolbarButtonEvent::ToolbarButton::PAUSE:
            m_editorController.Pause();
            break;
        case ToolbarButtonEvent::ToolbarButton::STOP:
            m_editorController.Stop();
            break;
        }
#endif
        // TODO
        UNREFERENCED(e);
    }
}

