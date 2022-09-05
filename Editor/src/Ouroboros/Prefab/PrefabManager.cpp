#include "pch.h"
#include "PrefabManager.h"
#include "PrefabComponent.h"
#include "App/Editor/Serializer.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Scene/Scene.h"
oo::PrefabManager::PrefabManager()
{
}

oo::PrefabManager::~PrefabManager()
{
}

void oo::PrefabManager::MakePrefab(std::shared_ptr<oo::GameObject> go)
{
	auto prefabPath = Serializer::SavePrefab(go, *ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>());
	auto& component = go->AddComponent<oo::PrefabComponent>();
	component.prefab_filePath = prefabPath;
	go->SetIsPrefab(true);
	for (auto childs : go->GetChildren(true))
		childs.SetIsPrefab(true);
}
