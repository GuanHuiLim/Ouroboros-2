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

// Basic files
#include <Ouroboros/Core/Input.h>
#include <Ouroboros/Core/Timer.h>
#include <Ouroboros/Core/LayerSet.h>
#include <Ouroboros/ImGui/ImGuiAbstraction.h>
#include <Ouroboros/EventSystem/EventManager.h>
#include <Ouroboros/Asset/AssetManager.h>

// Debug Layers
#include "Testing/TestLayers/InputDebugLayer.h"
#include "Testing/TestLayers/MainDebugLayer.h"
#include "Testing/TestLayers/AssetDebugLayer.h"
#include "Testing/TestLayers/FPSDisplayLayer.h"

// Core Essential Layers
#include "App/CoreLayers/SceneLayer.h"
#include "App/Corelayers/EditorLayer.h"
#include "App/CoreLayers/ScriptingLayer.h"
#include "App/CoreLayers/CoreLinkingLayer.h"

// External includes
#include <imgui/imgui.h>

//Shared Library related includes
#include <SceneManagement/include/SceneManager.h>
#include <Quaternion/include/Transform.h>
#include <Quaternion/include/Quaternion.h>
#include <Scenegraph/include/scenegraph.h>

// General Events
#include <Ouroboros/Core/Events/ApplicationEvent.h>
//events
#include <App/Editor/Events/LoadProjectEvents.h>
#include <App/Editor/Events/OpenPromtEvent.h>

//Tracy
#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>
//Optick
#include <optick.h>
// Scripting
#include <Scripting/Scripting.h>

// Should only let guan hui change these variables.
static constexpr const char* const EditorVersionNumber = "2.00";
static constexpr const char* const GameVersionNumber = "1.00";

class EditorApp final : public oo::Application
{
public:
    EditorApp(oo::CommandLineArgs args)
        : Application{ std::string{"Ouroboros v"} + EditorVersionNumber, args }
        , m_imGuiAbstract{ std::make_unique<oo::ImGuiAbstraction>() }
    {
        //Debug Test Layers
        // m_layerset.PushLayer(std::make_shared<InputDebugLayer>());
        //m_layerset.PushLayer(std::make_shared<AssetDebugLayer>());
#ifdef OO_EDITOR
        //m_layerset.PushLayer(std::make_shared<MainDebugLayer>());     //menu to test various debug scenes
        m_layerset.PushLayer(std::make_shared<FPSDisplayLayer>());      //FPS display counter
#endif
        // Main Layers
        // Scripting Layer
        m_layerset.PushLayer(std::make_shared<oo::ScriptingLayer>(m_sceneManager));
        // Scene Layer [Have differing code for OO_EDITOR AND OO_EXECUTABLE]
        m_layerset.PushLayer(std::make_shared<oo::SceneLayer>(m_sceneManager));
        // Link core important setup regardless of editor or final build
        m_layerset.PushLayer(std::make_shared<oo::CoreLinkingLayer>());
        // Editor Layer [Have differing code for OO_EDITOR AND OO_EXECUTABLE]
        // have this for both executable version as well for easy debugging purposes.
        m_editorLayer = std::make_shared<EditorLayer>(m_sceneManager);
        m_layerset.PushLayer(m_editorLayer);

        // binding to events
        oo::EventManager::Subscribe<EditorApp, ImGuiRestartEvent>(this, &EditorApp::RestartImGui);
        oo::EventManager::Subscribe<EditorApp, oo::WindowCloseEvent>(this, &EditorApp::CloseApp);
    }

    void OnUpdate() override
    {
        TRACY_PROFILE_SCOPE_N(editor_app_update);
        OPTICK_FRAME("editor_app_update");
        m_imGuiAbstract->Begin();

        m_layerset.Update();

        m_imGuiAbstract->End();
        
        // only present in the non-final version
        if (oo::input::IsKeyPressed(KEY_ESCAPE))
        {
            oo::WindowCloseEvent ununsed;
            CloseApp(&ununsed);
        }

        TRACY_PROFILE_SCOPE_END();
    }

    void RestartImGui(ImGuiRestartEvent*)
    {
        m_imGuiAbstract->Restart();
    }

    void CloseApp(oo::WindowCloseEvent*)
    {
        if (m_editorLayer->GetEditorMode() == true)
        {
            CloseProjectEvent e;
            OpenPromptEvent<CloseProjectEvent> ope(e, [&]() { Close(); });
            oo::EventManager::Broadcast(&ope);
        }
        else
        {
            Close();
        }
    }

private:
    // main scene manager
    SceneManager m_sceneManager;
    std::shared_ptr<EditorLayer> m_editorLayer;
    oo::LayerSet m_layerset;
    std::unique_ptr<oo::ImGuiAbstraction> m_imGuiAbstract;
};

class EndProduct final : public oo::Application
{
public:
    EndProduct(oo::CommandLineArgs args)
        : Application{ std::string{"Minute v"} + GameVersionNumber, args }
    {
        //Debug Layers
        // m_layerset.PushLayer(std::make_shared<InputDebugLayer>());

        // Main Layers
        // Scripting Layer
        m_layerset.PushLayer(std::make_shared<oo::ScriptingLayer>(m_sceneManager));
        // Scene Layer
        m_layerset.PushLayer(std::make_shared<oo::SceneLayer>(m_sceneManager));
        // Link core important setup regardless of editor or final build
        m_layerset.PushLayer(std::make_shared<oo::CoreLinkingLayer>());

        // only for the end product we do this instead
        std::filesystem::path p("./Project/Config.json");
        Project::LoadProject(p);
    }

    void OnUpdate() override
    {
        TRACY_PROFILE_SCOPE_N(end_product_app_update);

        m_layerset.Update();

        TRACY_PROFILE_SCOPE_END();
    }

private:
    // main scene manager
    SceneManager m_sceneManager;
    oo::LayerSet m_layerset;
};

oo::Application* oo::CreateApplication(oo::CommandLineArgs args)
{
#ifdef OO_END_PRODUCT
    return new EndProduct{ args };
#else // OO_EDITOR or OO_EXECUTABLE
    return new EditorApp{ args }; 
#endif
}
