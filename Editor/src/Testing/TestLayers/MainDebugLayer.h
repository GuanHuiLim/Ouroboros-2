/************************************************************************************//*!
\file           MainDebugLayer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          Debugging Layer that is used for debugging builds

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <vector>
#include <SceneManagement/include/SceneManager.h>
#include <imgui/imgui.h>

#include "Ouroboros/Core/Base.h"
#include "Ouroboros/Core/Layer.h"

#include "Testing/TestScenes/TestScene.h"
#include "Testing/TestScenes/DebugMsgsTestScene.h"
#include "Testing/TestScenes/ECSTestScene.h"


class MainDebugLayer final : public oo::Layer
{
private:
    std::vector<std::pair<std::string, SceneManager::key_type>> m_scenes;
    SceneManager m_sceneManager;

    template <typename Scene, typename ...Args>
    SceneManager::key_type AddScene(Args... args)
    {
        auto fullname = std::string(typeid(Scene).name()).substr(6);
        std::string finalName = fullname;
        std::size_t additional_index = 1;
        while (std::find_if(m_scenes.begin(), m_scenes.end(),
            [&](auto&& name_key_pair)
            {
                return name_key_pair.first == finalName;
            })
            != m_scenes.end())
        {
            finalName = fullname + " " + std::to_string(additional_index++);
        }

        auto [success, key, scene] = m_sceneManager.CreateNewScene<Scene>(finalName, args...);
        m_scenes.emplace_back(finalName, key);
        return key;
    };

    std::string GetSceneName(SceneManager::key_type key) const
    {
        for (auto& [name, scene_key]: m_scenes)
        {
            if (scene_key == key)
                return name;
        }

        return "not found";
    }

public:
    MainDebugLayer()
        : Layer{ "Main Debug Layer" }
    {
        LOG_TRACE("SUCCESSFULLY LOADED DEBUG LAYER");
    }

    void OnAttach() override final
    {
        // - empty test scene. most basic test scene
        //AddScene<TestScene>();
        // - debug print test scene for debug msgs
        //bool print_debug_messages = true;
        //AddScene<DebugMsgsTestScene>(print_debug_messages);
        // - ecs testing ground
        //AddScene<ECSTestScene>();
        // - scene testing ground
        AddScene<oo::Scene>("Default test");
        
        // - make sure this runs last.
        m_sceneManager.Init();
    }

    void OnDetach() override final
    {
        m_sceneManager.Terminate();
    }

    void OnUpdate() override final
    {
        ImGui::Begin("Debug Scenes!", nullptr, ImGuiWindowFlags_NoDocking);
        if (m_sceneManager.HasActiveScene())
        {
            auto displayText = "Current Debug Scene: " + GetSceneName(m_sceneManager.GetActiveScene().lock()->GetID());
            ImGui::Text(displayText.c_str());
        }
        for (auto& scene : m_scenes)
        {
            if (ImGui::Button(scene.first.c_str()))
            {
                m_sceneManager.ChangeScene(scene.second);
            }
        }
        ImGui::End();

        m_sceneManager.Update();
    }
};
