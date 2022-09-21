#include "pch.h"
#include "MeshHierarchy.h"

#include <stack>

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
	auto parent = scene->GetRoot();

	auto* node = modeldata->sceneInfo;
	std::stack<Node*> node_list;
	node_list.push(node);
	if (ImGui::Button("Press"))
	{
		while (node_list.empty() == false)
		{
			node = node_list.top();
			node_list.pop();
			if (node->meshRef != static_cast<uint32_t>(-1))
			{
				auto gameobject = scene->CreateGameObjectImmediate();
				gameobject->SetName(node->name);
				auto& transform = gameobject->EnsureComponent<oo::TransformComponent>();
				transform.SetGlobalTransform(node->transform);
				auto& renderer = gameobject->EnsureComponent<oo::MeshRendererComponent>();
				renderer.model_handle = modeldata->gfxMeshIndices[node->meshRef];
				//renderer.GetModelHandle();
				parent->AddChild(*gameobject, true);
				parent = gameobject;
			}
			//ImGui::TreeNodeEx(node->name.c_str(),ImGuiTreeNodeFlags_DefaultOpen| ImGuiTreeNodeFlags_NoTreePushOnOpen);

			for (auto data : node->children)
			{
				node_list.push(data);
			}
		}
	}
	
	//EntityHelper = [&](ModelData* model, Node* node) {
		//    if (node->meshRef != static_cast<uint32_t>(-1))
		//    {
		//        auto& ed = entities.emplace_back(EntityInfo{});
		//        ed.modelID = model->gfxMeshIndices[node->meshRef];
		//        ed.name = node->name;
		//        ed.entityID = FastRandomMagic();
		//        // this is trash just take the xform
		//        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(node->transform), glm::value_ptr(ed.position), glm::value_ptr(ed.rotVec), glm::value_ptr(ed.scale));
		//        
		//        ed.bindlessGlobalTextureIndex_Albedo = diffuseTexture3;
		//        ed.instanceData = 0;
		//    }
		//    for (auto& child : node->children)
		//    {
		//        EntityHelper(model,child);
		//    }   
}

