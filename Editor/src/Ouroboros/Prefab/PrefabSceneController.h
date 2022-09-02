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
