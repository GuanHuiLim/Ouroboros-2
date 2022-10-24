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

void oo::PrefabManager::BreakPrefab(std::shared_ptr<oo::GameObject> go)
{
	std::stack<scenenode::raw_pointer> hierarchy_nodes;
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	scenenode::shared_pointer temp = go->GetSceneNode().lock();
	scenenode::raw_pointer curr = temp.get();
	{
		auto gameobject = scene->FindWithInstanceID(curr->get_handle());
		gameobject->RemoveComponent<PrefabComponent>();
		gameobject->SetIsPrefab(false);
	}
	for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
	{
		scenenode::shared_pointer child = *iter;
		hierarchy_nodes.push(child.get());
	}
	while (!hierarchy_nodes.empty())
	{
		curr = hierarchy_nodes.top();
		hierarchy_nodes.pop();
		auto gameobject = scene->FindWithInstanceID(curr->get_handle());
		if (gameobject->HasComponent<PrefabComponent>())
			continue;
		gameobject->SetIsPrefab(false);
		for (auto iter = curr->rbegin(); iter != curr->rend(); ++iter)
		{
			scenenode::shared_pointer child = *iter;
			hierarchy_nodes.push(child.get());
		}
	}
}
