/************************************************************************************//*!
\file           PrefabSceneController.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Sept 30, 2022
\brief          Prefab controller for adjusting and controller the prefab scene backend
                for fast storing and loading from a 3rd invisible scene

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <SceneManagement/include/SceneManager.h>
#include "PrefabScene.h"

namespace oo
{
    class PrefabSceneController
    {
    public:
        PrefabSceneController(SceneManager& sceneManager);

        //Scene::go_ptr RequestForPrefab(std::string const& filepath, oo::Scene& targetScene);
		std::string& RequestForPrefab(std::string const& filepath);
    private:
        SceneManager& m_sceneManager;
        std::weak_ptr<PrefabScene> m_prefabScene = {};
    };
}
