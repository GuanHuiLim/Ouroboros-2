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
        container_type m_loadpaths;
        SceneManager& m_sceneManager;

        std::weak_ptr<RuntimeScene> m_runtimeScene = {};

        void OnLoadProjectEvent(LoadProjectEvent*);

    public:
        void SetLoadPaths(container_type&& loadPaths);
        container_type GetLoadPaths() const;
        void GenerateScenes();
        void RemoveScenes();
        void ClearSceneLibrary() { m_loadpaths.clear(); }

        bool HasScene(std::string_view sceneName) const;
        bool HasScene(size_type index) const;

        void AddLoadPath(std::string_view sceneName, std::string_view loadpath);
        void RemoveLoadPath(std::string_view sceneName);
        void Swap(std::string_view sceneName1, std::string_view sceneName2);
        void Swap(size_type index1, size_type index2);
        void ChangeRuntimeScene(std::string_view sceneName);
        void ChangeRuntimeScene(size_type index);

        std::weak_ptr<RuntimeScene> GetRuntimeScene() const;
    };

}