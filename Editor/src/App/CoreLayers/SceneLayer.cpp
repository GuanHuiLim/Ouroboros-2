#include "pch.h"
#include "SceneLayer.h"

#include "Ouroboros/Core/Input.h"
#include "Ouroboros/EventSystem/EventManager.h"

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
        EventManager::Subscribe<SceneLayer, GetCurrentSceneEvent>(this, &SceneLayer::OnGetCurrentSceneEvent);
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
#ifdef OO_EDITOR
        if (oo::input::IsKeyPressed(KEY_Q))
        {
            m_editorController.Simulate();
        }
        if (oo::input::IsKeyPressed(KEY_W))
        {
            m_editorController.Pause();
        }
        if (oo::input::IsKeyPressed(KEY_E))
        {
            m_editorController.Stop();
        }
#endif
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
#endif
        m_runtimeController;
    }
}

