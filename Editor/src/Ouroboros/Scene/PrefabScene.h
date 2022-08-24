/************************************************************************************//*!
\file           PrefabScene.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Oct 7, 2022
\brief          EditorScene describes the scene when its meant for editing.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Scene.h"

namespace oo
{
    class PrefabScene final : public Scene
    {
    public:
        explicit PrefabScene(std::string const& filepath);

        // Prefab Specific functionality
        go_ptr GetPrefab(std::string const& filepath);

    private:
        bool PrefabIsLoaded(std::string const& filepath) const;
        void LoadPrefab(std::string const& filepath);
        Scene::go_ptr LookUpPrefab(std::string const& filepath);

        std::map<std::string, go_ptr> m_loadedPrefabMap;
    };
}