/************************************************************************************//*!
\file          MeshHierarchy.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Ability to import the whole scene from fbx. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "MeshHierarchy.h"

#include <stack>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Asset/AssetManager.h"
#include "Project.h"

#include "OO_Vulkan/src/MeshModel.h"
#include "Ouroboros/Scene/Scene.h"
#include "Ouroboros/ECS/GameObject.h"
#include "Ouroboros/Transform/TransformComponent.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "Ouroboros/Vulkan/MeshRendererComponent.h"
#include "SceneManagement/include/SceneManager.h"
MeshHierarchy::MeshHierarchy()
{
	oo::EventManager::Subscribe<MeshHierarchy, OpenFileEvent>(this, &MeshHierarchy::OpenFileCallBack);
}

MeshHierarchy::~MeshHierarchy()
{
}

void MeshHierarchy::OpenFileCallBack(OpenFileEvent* e)
{
	if (e->m_type == OpenFileEvent::FileType::FBX)
	{
		ImGuiManager::GetItem("Mesh Hierarchy").m_enabled = true;
		auto relativepath = std::filesystem::relative(e->m_filepath, Project::GetAssetFolder());
		auto asset = Project::GetAssetManager()->LoadPath(relativepath);
		m_current_id = asset.GetID();
		//asset = Project::GetAssetManager()->Get(m_current_id);
		//auto path = asset.GetFilePath();
		//temp = relativepath;
	}
}

void MeshHierarchy::Show()
{
	if (m_current_id == 0)
		return;

	auto assetmanager = Project::GetAssetManager();
	auto asset = assetmanager->Get(m_current_id);
	auto modeldata = asset.GetData<ModelFileResource*>();
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();

	auto* node = modeldata->sceneInfo;
	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImGui::BeginChild("MeshChild", { contentRegion.x,contentRegion.y * 0.6f }, true);

	ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)node->children.size() ? ImGuiTreeNodeFlags_DefaultOpen: ImGuiTreeNodeFlags_Bullet;
	bool opened = ImGui::TreeNodeEx(node->name.c_str(), flags);
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAutoExpirePayload))
	{
		ImGui::SetDragDropPayload("MESH_HIERARCHY", &node, sizeof(Node**));
		ImGui::EndDragDropSource();
	}
	if (opened == false)
	{
		return;
	}
	std::stack<Node*> node_list;
	std::vector<Node*> node_parent;
	node_list.push(node);

	while (node_list.empty() == false)
	{
		node = node_list.top();
		node_list.pop();
		if (node->meshRef != static_cast<uint32_t>(-1))
		{
			while ((node_parent.empty() == false) && (node->parent != node_parent.back()))
			{
				node_parent.pop_back();
				ImGui::TreePop();
			}
			flags = node->children.size() ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Bullet;
			opened = ImGui::TreeNodeEx(node->name.c_str(), flags);
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAutoExpirePayload))
			{
				MeshHierarchyDragDropData data;
				data.data = node;
				data.id = m_current_id;
				ImGui::SetDragDropPayload("MESH_HIERARCHY", &data, sizeof(MeshHierarchyDragDropData));
				ImGui::EndDragDropSource();
			}
			if (opened == false)
				continue;
		}
		node_parent.push_back({ node });
		for (auto data : node->children)
		{
			node_list.push(data);
		}
		//ImGui::TreeNodeEx(node->name.c_str(),ImGuiTreeNodeFlags_DefaultOpen| ImGuiTreeNodeFlags_NoTreePushOnOpen);
	}
	while (node_parent.empty() == false)
	{
		node_parent.pop_back();
		ImGui::TreePop();
	}
	ImGui::EndChild();
	if(ImGui::Button("Add Whole Mesh"))
	{
		CreateObject(modeldata->sceneInfo,m_current_id);
	}
}

void MeshHierarchy::CreateObject(Node* node,oo::AssetID asset_id)
{
	auto assetmanager = Project::GetAssetManager();
	auto asset = assetmanager->Get(asset_id);
	//auto modeldata = asset.GetData<ModelFileResource*>();
	

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	std::stack<Node*> node_list;
	std::vector<std::pair<Node*, std::shared_ptr<oo::GameObject>>> node_parent;
	node_list.push(node);
	//creation
	std::shared_ptr<oo::GameObject> gameobject = scene->CreateGameObjectImmediate();
	gameobject->SetName(node->name);
	node_parent.push_back({ node->parent ,gameobject });
	scene->GetRoot()->AddChild(*gameobject,true);
	while (node_list.empty() == false)
	{
		node = node_list.top();
		node_list.pop();
		if (node->meshRef != static_cast<uint32_t>(-1))
		{
			gameobject = scene->CreateGameObjectImmediate();
			gameobject->SetName(node->name);
			auto& transform = gameobject->EnsureComponent<oo::TransformComponent>();
			transform.SetGlobalTransform(node->transform);
			auto& renderer = gameobject->EnsureComponent<oo::MeshRendererComponent>();
			renderer.SetModelHandle(asset,node->meshRef);
			while ((node_parent.empty() == false) && (node->parent != node_parent.back().first))
			{
				node_parent.pop_back();
			}
			node_parent.back().second->AddChild(*gameobject, true);

		}
		node_parent.push_back({ node, gameobject });
		for (auto data : node->children)
		{
			node_list.push(data);
		}
		//ImGui::TreeNodeEx(node->name.c_str(),ImGuiTreeNodeFlags_DefaultOpen| ImGuiTreeNodeFlags_NoTreePushOnOpen);
	}
}

