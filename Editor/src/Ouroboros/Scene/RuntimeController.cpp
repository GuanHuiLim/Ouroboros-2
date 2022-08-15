/************************************************************************************//*!
\file           RuntimeController.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 17, 2022
\brief          Runtime Scene Controller that will be used in the final built
                to load the various scenes.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "RuntimeController.h"
#include <SceneManagement/include/SceneManager.h>
#include "RuntimeScene.h"

namespace oo
{
    //RuntimeController::container_type RuntimeController::m_loadpaths;

    void RuntimeController::SetLoadPaths(container_type loadPaths)
    {
        RemoveScenes();
        ClearSceneLibrary();
        m_loadpaths = loadPaths;
    }

    RuntimeController::container_type RuntimeController::GetLoadPaths() const 
    {
        return m_loadpaths; 
    }

    void RuntimeController::GenerateScenes()
    {
        for (auto& scenePath : m_loadpaths)
        {
            m_sceneManager.CreateNewScene<RuntimeScene>(scenePath.SceneName, scenePath.LoadPath);
        }
    }

    void RuntimeController::RemoveScenes()
    {
        for (auto& scenePath : m_loadpaths)
        {
            m_sceneManager.RemoveScene(scenePath.SceneName);
        }
    }

    bool RuntimeController::HasScene(std::string_view sceneName) const
    {
        auto iter = std::find_if(m_loadpaths.cbegin(), m_loadpaths.cend(), [=](SceneInfo elem) 
                    { 
                        return elem.SceneName == sceneName; 
                    });
        return iter != m_loadpaths.cend();
    }

    bool RuntimeController::HasScene(size_type index) const
    {
        return index >= 0 && index < m_loadpaths.size();
    }

    void RuntimeController::AddLoadPath(std::string_view sceneName, std::string_view loadpath)
    {
        // make sure file path is unique.
        if (std::find_if(m_loadpaths.cbegin(), m_loadpaths.cend(), [=](SceneInfo elem) { return elem.LoadPath == loadpath; }) == m_loadpaths.cend())
        {
            m_loadpaths.emplace_back(sceneName, loadpath/*, m_loadpaths.size()*/);
        }
        else
        {
            LOG_ERROR("Attempting to add an already loaded scene {0}", sceneName);
        }
    }

    void RuntimeController::RemoveLoadPath(std::string_view sceneName)
    {
        auto iter = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName; });
        if (iter != m_loadpaths.end())
            m_loadpaths.erase(iter);
    }

    void RuntimeController::Swap(std::string_view sceneName1, std::string_view sceneName2)
    {
        auto first = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName1; });
        auto second = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName2; });
        if (first != second)
            std::iter_swap(first, second);
    }

    void RuntimeController::Swap(size_type index1, size_type index2)
    {
        if (HasScene(index1) && HasScene(index2))
        {
            std::swap(m_loadpaths[index1], m_loadpaths[index2]);
        }
    }

    void RuntimeController::ChangeRuntimeScene(std::string_view sceneName)
    {
        auto iter = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName; });

        if (iter == m_loadpaths.end())
        {
            LOG_WARN("Scene \"{0}\" was not not added to build thus it was loaded! Ensure you add it to the build via", sceneName);
            return;
        }
        LOG_INFO("Changing to runtime scene \"{0}\"", sceneName);
        m_sceneManager.ChangeScene(sceneName);
    }

    void RuntimeController::ChangeRuntimeScene(size_type index)
    {
        if (index < 0 || index >= m_loadpaths.size())
        {
            LOG_WARN("Scene {0} was not not added to build thus it was loaded! Ensure you add it to the build via", index);
            return;
        }
        LOG_INFO("Changing to runtime scene {0}", m_loadpaths[index].SceneName);
        m_sceneManager.ChangeScene(m_loadpaths[index].SceneName);
    }

}
