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
#include "Testing/TestLayers/InputDebugLayer.h"
#include "Testing/TestLayers/MainDebugLayer.h"

// Core Essential Layers
#include "App/CoreLayers/SceneLayer.h"
#include "App/Corelayers/EditorLayer.h"
#include "App/CoreLayers/ScriptingLayer.h"

// External includes
#include <imgui/imgui.h>

//Shared Library related includes
#include <SceneManagement/include/SceneManager.h>
#include <Quaternion/include/Transform.h>
#include <Quaternion/include/Quaternion.h>
#include <Scenegraph/include/scenegraph.h>


#include <Ouroboros/Core/Events/ApplicationEvent.h>

//Tracy
#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>

#include <Scripting/Scripting.h>

class EditorApp final : public oo::Application
{
private:
    // main scene manager
    SceneManager m_sceneManager;

public:
    EditorApp(oo::CommandLineArgs args)
        : Application{ "Ouroboros v2.0", args }
    {
        // Scripting Layer
        m_layerset.PushLayer(std::make_shared<oo::ScriptingLayer>());

        //Debug Layers
        //m_layerset.PushLayer(std::make_shared<InputDebugLayer>());
        m_layerset.PushLayer(std::make_shared<MainDebugLayer>());

        m_layerset.PushLayer(std::make_shared<oo::SceneLayer>(m_sceneManager));
        // Main Layers
        m_layerset.PushLayer(std::make_shared<EditorLayer>(m_sceneManager));

        m_imGuiAbstract = std::make_unique<oo::ImGuiAbstraction>();
        
        // binding to events
        oo::EventManager::Subscribe<EditorApp, ImGuiRestartEvent>(this, &EditorApp::RestartImGui);
        oo::EventManager::Subscribe<EditorApp, oo::WindowCloseEvent>(this, &EditorApp::CloseApp);
    }

    void OnUpdate() override
    {
        constexpr const char* const editor_update = "Editor App Update";
        TRACY_PROFILE_SCOPE_N(editor_update);

        m_imGuiAbstract->Begin();

        m_layerset.Update();

        m_imGuiAbstract->End();

        if (oo::input::IsKeyPressed(KEY_ESCAPE))
        {
            Close();
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void RestartImGui(ImGuiRestartEvent*)
    {
        m_imGuiAbstract->Restart();
    }

    void CloseApp(oo::WindowCloseEvent*)
    {
        Close();
    }

private:
    oo::LayerSet m_layerset;
    std::unique_ptr<oo::ImGuiAbstraction> m_imGuiAbstract;
};

oo::Application* oo::CreateApplication(oo::CommandLineArgs args)
{
    return new EditorApp{ args };
}
