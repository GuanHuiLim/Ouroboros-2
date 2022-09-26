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

// Scripting
#include <Scripting/Scripting.h>


class EditorApp final : public oo::Application
{
private:
    // main scene manager
    SceneManager m_sceneManager;
    //oo::AssetManager m_assetManager{ "./" };
#ifndef OO_END_PRODUCT
    std::shared_ptr<EditorLayer> m_editorLayer;
#endif

public:
    EditorApp(oo::CommandLineArgs args)
        : Application{ "Ouroboros v2.0", args }
        //, m_assetManager{oo::AssetManager("./assets")}
    {
        // Scripting Layer
        m_layerset.PushLayer(std::make_shared<oo::ScriptingLayer>(m_sceneManager));

        //Debug Layers
        // m_layerset.PushLayer(std::make_shared<InputDebugLayer>());
#ifdef OO_EDITOR
        m_layerset.PushLayer(std::make_shared<MainDebugLayer>());
        m_layerset.PushLayer(std::make_shared<FPSDisplayLayer>());
#endif
        // Main Layers
        m_layerset.PushLayer(std::make_shared<AssetDebugLayer>());
        m_layerset.PushLayer(std::make_shared<oo::SceneLayer>(m_sceneManager));

#ifndef OO_END_PRODUCT
        m_editorLayer = std::make_shared<EditorLayer>(m_sceneManager);
        m_layerset.PushLayer(m_editorLayer);
#else   // only for the end product we do this instead
        std::filesystem::path p("./Project/Config.json");
        Project::LoadProject(p);
#endif

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

#ifndef OO_END_PRODUCT
        if (oo::input::IsKeyPressed(KEY_ESCAPE))
        {
            oo::WindowCloseEvent ununsed;
            CloseApp(&ununsed);
        }
#endif

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
    oo::LayerSet m_layerset;
    std::unique_ptr<oo::ImGuiAbstraction> m_imGuiAbstract;
};

oo::Application* oo::CreateApplication(oo::CommandLineArgs args)
{
    return new EditorApp{ args };
}
