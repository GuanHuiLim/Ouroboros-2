#include "pch.h"
#include "PrefabSceneController.h"

namespace oo
{
    PrefabSceneController::PrefabSceneController(SceneManager& sceneManager)
        : m_sceneManager{ sceneManager }
    {
        // Create prefab scene upon creation : expected to create one of this prefab controllers only
        auto [success, editor_key, prefabScene] = m_sceneManager.CreateNewScene<PrefabScene>("prefab Scene key", "default prefab path");
        ASSERT_MSG(!success, "Couldn't load prefab instancing scene, is the file path passed in correct?");
        m_prefabScene = prefabScene;
    }

    Scene::go_ptr PrefabSceneController::RequestForPrefab(std::string const& filepath)
    {
        return m_prefabScene.lock()->GetPrefab(filepath);
    }
    
}
