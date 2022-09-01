#include "pch.h"
#include "PrefabSceneController.h"
#include "App/Editor/Serializer.h"
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

	std::string& PrefabSceneController::RequestForPrefab(std::string const& filepath)
	{
		auto prefabscene = m_prefabScene.lock();
		ASSERT_MSG(prefabscene == nullptr, "Prefab scene not initalized");
		return prefabscene->GetPrefab(filepath);
	}

    //Scene::go_ptr PrefabSceneController::RequestForPrefab(std::string const& filepath)
    //{
    //    return m_prefabScene.lock()->GetPrefab(filepath);
    //}
    
}
