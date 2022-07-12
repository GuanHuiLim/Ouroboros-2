/************************************************************************************//*!
\file           SceneLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 1, 2022
\brief          Defines a layer that will be running in the both the editor and
                the final distribution build

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#include <SceneManager.h>
#include "Ouroboros/Core/Layer.h"

#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/Scene/EditorController.h"

#include "Ouroboros/Core/Input.h"

namespace oo
{
    class SceneLayer final : public oo::Layer
    {
    private:
        SceneManager& m_sceneManager;
        RuntimeController m_runtimeController;
        EditorController m_editorController;
    public:

        SceneLayer(SceneManager& sceneManager)
            : m_sceneManager { sceneManager }
            , m_runtimeController { m_sceneManager }
            , m_editorController{ m_sceneManager, m_runtimeController }
            , Layer("Scene Management Layer")
        {
            m_editorController.Init();
        }

        ~SceneLayer()
        {
        }
        
        void OnAttach() override final
        {
            m_sceneManager.Init();
        }

        void OnDetach() override final
        {
            m_sceneManager.Terminate();
        }

        void OnUpdate() override final
        {
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

            m_sceneManager.Update();
        }

    };

}
