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
#include "Ouroboros/Vulkan/RendererComponent.h"
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
		asset = Project::GetAssetManager()->Get(m_current_id);
		auto path = asset.GetFilePath();
		temp = relativepath;
	}
}

void MeshHierarchy::Show()
{
	if (m_current_id == 0)
		return;

	auto assetmanager = Project::GetAssetManager();
	auto asset = assetmanager->LoadName(temp)[0];
	auto modeldata = asset.GetData<ModelData*>();
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();

	std::stack<Node*> node_list;
	std::vector<std::pair<Node*, std::shared_ptr<oo::GameObject>>> node_parent;
	auto* node = modeldata->sceneInfo;
	node_list.push(node);

	std::shared_ptr<oo::GameObject> gameobject = scene->CreateGameObjectImmediate();
	gameobject->SetName(node->name);
	ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)node->children.size() ? ImGuiTreeNodeFlags_DefaultOpen: ImGuiTreeNodeFlags_Bullet;
	ImGui::TreeNodeEx(node->name.c_str());
	while (node_list.empty() == false)
	{
		node = node_list.top();
		node_list.pop();
		if (node->meshRef != static_cast<uint32_t>(-1))
		{
			while (node->parent != node_parent.back().first)
			{
				node_parent.pop_back();
				ImGui::TreePop();
			}
			ImGuiTreeNodeFlags flags = node->children.size() ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Bullet;
			ImGui::TreeNodeEx(node->name.c_str(), flags);
		}
		node_parent.push_back({ node, gameobject });
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
}

void MeshHierarchy::CreateObject(Node* node)
{
	auto assetmanager = Project::GetAssetManager();
	auto asset = assetmanager->LoadName(temp)[0];
	auto modeldata = asset.GetData<ModelData*>();
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	std::stack<Node*> node_list;
	std::vector<std::pair<Node*, std::shared_ptr<oo::GameObject>>> node_parent;
	node_list.push(node);
	
	//creation
	std::shared_ptr<oo::GameObject> gameobject = scene->CreateGameObjectImmediate();
	gameobject->SetName(node->name);
	while (node_list.empty() == false)
	{
		node = node_list.top();
		node_list.pop();
		if (node->meshRef != static_cast<uint32_t>(-1))
		{
			gameobject = scene->CreateGameObjectImmediate();
			gameobject->SetName(node->name);
			auto& transform = gameobject->EnsureComponent<oo::TransformComponent>();
			transform.SetLocalTransform(node->transform);
			auto& renderer = gameobject->EnsureComponent<oo::MeshRendererComponent>();
			renderer.model_handle = modeldata->gfxMeshIndices[node->meshRef];

			while (node->parent != node_parent.back().first)
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

