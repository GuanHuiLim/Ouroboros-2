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

#include "Ouroboros/Scene/Scene.h"
#include <unordered_map>
#include <string>
namespace oo
{
    class PrefabScene final : public Scene
    {
    public:
        explicit PrefabScene(std::string const& filepath);

        // Prefab Specific functionality
        //go_ptr GetPrefab(std::string const& filepath);
		std::string& GetPrefab(std::string const& filepath);
    private:
        //bool PrefabIsLoaded(std::string const& filepath) const;
		//Scene::go_ptr LoadPrefab(std::string const& filepath);
        //Scene::go_ptr LookUpPrefab(std::string const& filepath);
		std::string& LoadPrefab(std::string const& filepath);
		std::string& LookUpPrefab(std::string const& filepath);
        //std::unordered_map<std::string, go_ptr> m_loadedPrefabMap;
		std::unordered_map<std::string, std::string> m_loadedPrefabMap;

    };
}