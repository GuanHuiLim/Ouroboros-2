/************************************************************************************//*!
\file           EditorApp.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 22, 2022
\brief          Customer side of the project that utilizes the functions of the Engine.
                An Editor will be a use case for game engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"

// specially include this file only at the entry point of the engine.
#include <EntryPoint.h>

#include <Ouroboros/Core/Input.h>
#include <Ouroboros/Core/Timer.h>
#include <Ouroboros/Core/LayerSet.h>
#include <Ouroboros/ImGui/ImGuiAbstraction.h>
#include <Ouroboros/EventSystem/EventManager.h>

// Debug Layers
#include "TestLayers/InputDebugLayer.h"
#include "TestLayers/MainDebugLayer.h"

// Core Essential Layers
#include "CoreLayers/SceneLayer.h"
#include "Corelayers/EditorLayer.h"

// External includes
#include <imgui.h>

//Shared Library related includes
#include <SceneManager.h>
#include <Transform.h>
#include <Quaternion.h>
#include <Scenegraph.h>

class EditorApp final : public oo::Application
{
private:
    // main scene manager
    SceneManager m_sceneManager;

public:
    EditorApp(oo::CommandLineArgs args)
        : Application{ "Ouroboros v2.0", args }
    {
        //Debug Layers
        //m_layerset.PushLayer(std::make_shared<InputDebugLayer>());
        m_layerset.PushLayer(std::make_shared<MainDebugLayer>());
        
        // Main Layers
        m_layerset.PushLayer(std::make_shared<EditorLayer>());

        m_imGuiAbstract = std::make_unique<oo::ImGuiAbstraction>();
        
        oo::EventManager::Subscribe<EditorApp, ImGuiRestartEvent>(this, &EditorApp::RestartImGui);
        
        //m_layerset.PushLayer(std::make_shared<oo::SceneLayer>(m_sceneManager));

    }

    void OnUpdate() override
    {
        m_imGuiAbstract->Begin();

        m_layerset.Update();

        m_imGuiAbstract->End();

        if (oo::input::IsKeyPressed(KEY_ESCAPE))
        {
            Close();
        }
    }

    void RestartImGui(ImGuiRestartEvent*)
    {
        m_imGuiAbstract->Restart();
    }

private:
    oo::LayerSet m_layerset;
    std::unique_ptr<oo::ImGuiAbstraction> m_imGuiAbstract;
};

oo::Application* oo::CreateApplication(oo::CommandLineArgs args)
{
    return new EditorApp{ args };
}
