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
#pragma once

#include "Scene.h"
#include <vector>

#include "Sceneinfo.h"

#include "App/Editor/Events/LoadProjectEvents.h"

namespace oo
{
    class RuntimeController final
    {
    public:
        using container_type = std::vector<SceneInfo>;
        using size_type = container_type::size_type;
    
    public:
        RuntimeController(SceneManager& sceneManager);
        ~RuntimeController() = default;

    private:
        SceneManager& m_sceneManager;                                   // scene manager it is talking to
        std::unordered_map<std::string, std::string> m_filepathLookup;  // lookup table for { name : filepath } from loaded paths
        //std::unordered_map<std::string, std::string> m_filenameLookup;  // lookup table for { filepath : name } from loaded paths
        container_type m_loadpaths;                                     // all the paths that are loaded. uniquely identified by filepath
        std::weak_ptr<RuntimeScene> m_runtimeScene = {};                // ptr to current runtime scene.
        
        void OnLoadProjectEvent(LoadProjectEvent*);
    public:
        void SetLoadPaths(container_type&& loadPaths);
        container_type GetLoadPaths() const;
        void GenerateScenes();
        void RemoveScenes();
        void ClearSceneLibrary();

        bool HasSceneWithName(std::string_view sceneName) const;
        bool HasSceneWithLoadPath(std::string_view loadpath) const;
        bool HasSceneWithIndex(size_type sceneIndex) const;
        void AddLoadPath(std::string_view sceneName, std::string_view loadpath);
        void RemoveLoadPathByName(std::string_view sceneName);
        void RemoveLoadPathByPath(std::string_view loadpath);
        void SwapSceneOrder(std::string_view sceneName1, std::string_view sceneName2);
        void SwapSceneOrder(size_type sceneIndex1, size_type sceneIndex2);
        void ChangeRuntimeScene(std::string_view sceneName);
        void ChangeRuntimeScene(size_type sceneIndex);

        size_type GetIndexWithLoadPath(std::string_view loadpath) const;

        std::weak_ptr<RuntimeScene> GetRuntimeScene() const;
        void SetRuntimeScene(std::weak_ptr<RuntimeScene> newScene);

        //special editor function
        //void AddLoadPathForEditor(std::string_view sceneName, std::string_view loadpath);
    };

}