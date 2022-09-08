/************************************************************************************//*!
\file           EditorLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jul 31, 2022
\brief          Defines a layer that will be running during editor mode 
                and its related events

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <Ouroboros/Core/Layer.h>
#include <Ouroboros/EventSystem/Event.h>

// Project Tracker related includes
#include <Launcher/Launcher/ProjectTracker.h>
#include <App/Editor/Editor.h>
#include <App/Editor/Utility/ImGuiManager.h>

#include <SceneManagement/include/SceneManager.h>
#include <Ouroboros/Scene/Scene.h>
#include <functional>
#include "Project.h"
#include "App/Editor/Events/ImGuiRestartEvent.h"
#include "Ouroboros/Prefab/PrefabSceneController.h"
class EditorLayer final : public oo::Layer
{
private:
    //order matters dont change it
    bool m_demo = true;
    bool m_editormode = false;
	oo::PrefabSceneController m_prefab_controller;
    ProjectTracker m_tracker;

#if OO_EDITOR
	Editor m_editor;
#endif

public:
    EditorLayer(SceneManager& m_sceneManager)
        : oo::Layer{ "EditorLayer" },
        m_tracker{ [this](std::filesystem::path& p) {Project::LoadProject(p); m_editormode = true; } },
		m_prefab_controller{ m_sceneManager }
    {
        LOG_INFO("Test Info");
        LOG_TRACE("Test Trace");
        LOG_WARN("Test Warn");
        LOG_ERROR("Test Error");
        LOG_CRITICAL("Test Critical");
		ImGuiManager::s_scenemanager = &m_sceneManager;
		ImGuiManager::s_prefab_controller = &m_prefab_controller;
    }

    virtual void OnAttach() override final;

    // TODO : IMGUI DOESNT WORK YET FOR NOW. VULKAN NEEDS TO BE SET UP
    // PROPERLY FOR IMGUI RENDERING TO TAKE PLACE
    virtual void OnUpdate() override final;

};
