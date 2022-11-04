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

#include "Ouroboros/EventSystem/EventManager.h"
namespace oo
{
    RuntimeController::RuntimeController(SceneManager& sceneManager) 
        : m_sceneManager{ sceneManager }
    {
        EventManager::Subscribe<RuntimeController, LoadProjectEvent>(this, &RuntimeController::OnLoadProjectEvent);
    }

    void RuntimeController::OnLoadProjectEvent(LoadProjectEvent* loadProjEvent)
    {
        SetLoadPaths(std::move(loadProjEvent->m_filename_pathname));
    // start the project immediately at first detected scene
#if OO_EXECUTABLE
        GenerateScenes();
        ChangeRuntimeScene(0);
#endif
    }

    void RuntimeController::SetLoadPaths(container_type&& loadPaths)
    {
        RemoveScenes();
        ClearSceneLibrary();
        m_loadpaths = std::move(loadPaths);
    }

    RuntimeController::container_type RuntimeController::GetLoadPaths() const 
    {
        return m_loadpaths; 
    }

    void RuntimeController::GenerateScenes()
    {
        // loops through all loadpaths, create the neccesary scenes
        for (auto& scenePath : m_loadpaths)
        {
            m_sceneManager.CreateNewScene<RuntimeScene>(scenePath.SceneName, scenePath.LoadPath);
        }
    }

    void RuntimeController::RemoveScenes()
    {
        // cleans up all the loaded scenes by name.
        for (auto& sceneInfo : m_loadpaths)
        {
            m_sceneManager.RemoveScene(sceneInfo.SceneName);
        }
    }

    void RuntimeController::ClearSceneLibrary() 
    { 
        m_filepathLookup.clear();
        m_loadpaths.clear(); 
    }

    bool RuntimeController::HasScene(std::string_view sceneName) const
    {
        return m_filepathLookup.contains(sceneName.data());
    }

    bool RuntimeController::HasScene(size_type sceneIndex) const
    {
        return sceneIndex >= 0 && sceneIndex < m_loadpaths.size();
    }

    void RuntimeController::AddLoadPath(std::string_view sceneName, std::string_view loadpath)
    {
        // ensure unique file path
        if (std::find(m_loadpaths.cbegin(), m_loadpaths.cend(), loadpath) == m_loadpaths.cend())
        {
            m_loadpaths.emplace_back(sceneName, loadpath);
            m_filepathLookup.emplace(sceneName, loadpath);
        }
        else
        {
            LOG_ERROR("Attempting to add a scene named \"{0}\" with an already existing file path location {1}", sceneName, loadpath);
        }
    }

    void RuntimeController::RemoveLoadPath(std::string_view sceneName)
    {
        auto iter = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName; });
        if (iter != m_loadpaths.end())
        {
            m_loadpaths.erase(iter);
            ASSERT_MSG(HasScene(sceneName) == false, "This should never happen!");
            m_filepathLookup.erase(sceneName.data());
        }
    }

    void RuntimeController::SwapSceneOrder(std::string_view sceneName1, std::string_view sceneName2)
    {
        auto first = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName1; });
        auto second = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName2; });
        if (first != second)
        {
            std::iter_swap(first, second);
            LOG_INFO("Swapping scene order via name! \"{0}\" swapped with \"{1}\"", sceneName1, sceneName2);
        }
        // no need to change filelookup
    }

    void RuntimeController::SwapSceneOrder(size_type index1, size_type index2)
    {
        if (HasScene(index1) && HasScene(index2))
        {
            std::swap(m_loadpaths[index1], m_loadpaths[index2]);
            LOG_INFO("Swapping scene order! Index {0} swapped with Index {1}", index1, index2);
        }
        else
        {
            LOG_INFO("Either Index is Invalid {0}, {1}! Failed attempting to swap runtime scene order .", index1, index2);
        }
    }

    void RuntimeController::ChangeRuntimeScene(std::string_view sceneName)
    {
        auto iter = std::find_if(m_loadpaths.begin(), m_loadpaths.end(), [=](SceneInfo elem) { return elem.SceneName == sceneName; });
        if (iter == m_loadpaths.end())
        {
            LOG_WARN("Scene named \"{0}\" was not added to build thus it was not loaded! Ensure you add it to the runtime controller and the name is correct", sceneName);
            return;
        }
        LOG_INFO("Changing to runtime scene via name : \"{0}\" ", sceneName);
        m_sceneManager.ChangeScene(sceneName);
    }

    void RuntimeController::ChangeRuntimeScene(size_type sceneIndex)
    {
        if (HasScene(sceneIndex))
        {
            LOG_INFO("Changing runtime scene to {1} via index {0} ", sceneIndex, m_loadpaths[sceneIndex].SceneName);
            m_sceneManager.ChangeScene(m_loadpaths[sceneIndex].SceneName);
        }
        else
        {
            LOG_INFO("Invalid index {0}! Failed attempting to change runtime scene.", sceneIndex);
        }
    }

    std::weak_ptr<RuntimeScene> RuntimeController::GetRuntimeScene() const 
    { 
        return m_runtimeScene; 
    }

    void RuntimeController::SetRuntimeScene(std::weak_ptr<RuntimeScene> newScene)
    {
        m_runtimeScene = newScene;
    }

}
