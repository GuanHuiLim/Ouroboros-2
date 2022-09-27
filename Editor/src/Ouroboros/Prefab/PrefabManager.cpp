/************************************************************************************//*!
\file           PrefabManager.cpp
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          contains a function to create a prefab

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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
