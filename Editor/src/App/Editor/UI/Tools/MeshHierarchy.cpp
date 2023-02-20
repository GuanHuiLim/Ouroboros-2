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
#include "Ouroboros/Vulkan/SkinRendererComponent.h"
#include "Ouroboros/Transform/TransformSystem.h"
#include "Ouroboros/Animation/Animation.h"
#include "Ouroboros/Animation/AnimationSystem.h"
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
		auto asset = Project::GetAssetManager()->GetOrLoadPath(relativepath);
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
	if (ImGui::BeginChild("MeshChild", { contentRegion.x,contentRegion.y * 0.6f }, true) == false)
	{
		ImGui::EndChild();
		return;
	}

	ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)node->children.size() ? ImGuiTreeNodeFlags_DefaultOpen: ImGuiTreeNodeFlags_Bullet;
	bool opened = ImGui::TreeNodeEx(node->name.c_str(), flags | ImGuiTreeNodeFlags_OpenOnArrow);
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAutoExpirePayload))
	{
		ImGui::SetDragDropPayload("MESH_HIERARCHY", &node, sizeof(Node**));
		ImGui::EndDragDropSource();
	}
	if (opened == false)
	{
		ImGui::EndChild();
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
			opened = ImGui::TreeNodeEx(node->name.c_str(), flags| ImGuiTreeNodeFlags_OpenOnArrow);
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
		auto reversed_node = node->children;	// intentional copy
		std::reverse(reversed_node.begin(), reversed_node.end());
		for (auto data : reversed_node)
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
	if (ImGui::Button("Generate Animation"))
	{
		auto anims = oo::Anim::AnimationSystem::LoadAnimationFromFBX(asset.GetFilePath().string(), modeldata);

		if constexpr(true)
		{
			auto anim = anims.front();

			auto const fn = [&](size_t start, size_t end, std::string name, oo::Anim::UID uid = oo::Anim::internal::invalid_ID) {
				
				oo::Anim::SplitAnimationInfo info{
					.in_frames{true},
					.start_frame{start},
					.end_frame{end},
					.anim_ID{anim->animation_ID},
					.split_anim_ID{uid},
					.split_animation_name{name}
				};
				auto result = oo::Anim::AnimationSystem::SplitAnimation(info);
				assert(result);
				return result;
			};

			fn(1, 397, "Char_Idle", 11904289930031233588ull);
			fn(399, 436, "Char_Jump", 14468092223772484637ull);
			fn(439, 468, "Char_Land", 15483678231358038326ull);
			fn(481, 550, "Char_Falling", 3027657552290603723ull);
			fn(552, 600, "Char_Running", 11102568273309958006ull);
			fn(602, 700, "Char_Punch1", 12842570071285323175ull);
			fn(702, 780, "Char_Punch2", 14933767867787793757ull);
			fn(782, 867, "Char_Punch3", 15887215621292314739ull);
			fn(869, 1045, "Char_KeyHit1", 7976973829188605073ull);
			fn(1047, 1210, "Char_KeyHit2", 16651380583556832464ull);
			fn(1330, 1526, "Char_KeyHit3", 671498236959028427ull);
			fn(1528, 1582, "Char_KeyAim", 16386213344299929188ull);
			fn(1583, 1660, "Char_KeyIdle", 9005349775375630608ull);
			fn(1662, 1700, "Char_KeyThrow", 5112540270552868611ull);
			fn(1702, 1750, "Char_KeyAim_WalkForward", 13369109700108596100ull);
			fn(1752, 1800, "Char_KeyAim_Strafe_Left", 14080966758555531269ull);
			fn(1802, 1850, "Char_KeyAim_Strafe_Right", 10825844878584567926ull);
			fn(1852, 1900, "Char_KeyAim_Walk_Back", 4090995566703906600ull);
			fn(1924, 1961, "Char_Key_Calling", 9585537277754178479ull);
			fn(1963, 2069, "Char_Key_Retrieval", 18114998630772137397ull);
			fn(2071, 2104, "Char_KeyAim_Jump", 17201684321423207373ull);
			fn(2105, 2120, "Char_KeyAim_Land", 9971078294644923685ull);
			fn(2122, 2150, "Char_KeyAim_Falling", 395494563579935615ull);
			fn(2152, 2200, "Char_Falling_KeyThrow", 3040187778218234769ull);
			fn(2210, 2262, "Char_Falling_Key_Calling", 5882446694374748978ull);
			fn(2262, 2320, "Char_Falling_Key_Retrieval", 8604496732628043322ull);
			fn(2322, 2360, "Char_Dash", 16915825071909083776ull);
			fn(2362, 2396, "Char_Sprint", 5237368080912885538ull);
			fn(2398, 2420, "Char_Stagger1", 8203964857723252188ull);
			fn(2422, 2444, "Char_Stagger2", 15999465100576333431ull);
			fn(2446, 2468, "Char_Stagger3", 18274652021443959365ull);
			fn(2470, 2600, "Char_Death", 2019095094436478344ull);
			fn(2601, 2711, "Char_HighKeyHit");
			fn(2712, 2900, "Char_Respawn_from_Respawner");
			fn(2902, 2932, "Char_KnockedDown_Key_Pops_Out");
			fn(2933, 2975, "Char_KnockedDown_Land");
			fn(2976, 3078, "Char_KnockedDown_Recovery");
			fn(3080, 3140, "Char_KnockedDown_Falling");
			fn(3141, 3180, "Char_DoubleJump");
			fn(3182, 3221, "Char_SprintDrag");
			fn(3223, 3307, "Char_SprintDragHit");
			fn(3309, 3349, "Char_TrampolineJump");


		}
	}
}

void MeshHierarchy::CreateObject(Node* node,oo::AssetID asset_id)
{
	auto assetmanager = Project::GetAssetManager();
	auto asset = assetmanager->Get(asset_id);

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	std::stack<Node*> node_list;
	std::vector<std::pair<Node*, std::shared_ptr<oo::GameObject>>> node_parent;
	node_list.push(node);
	//creation
	std::shared_ptr<oo::GameObject> gameobject = scene->CreateGameObjectImmediate();
	auto containing_gameobj = gameobject;
	gameobject->SetName(node->name);
	node_parent.push_back({ node->parent ,gameobject });
	scene->GetRoot()->AddChild(*gameobject,true);

	auto modeldata = asset.GetData<ModelFileResource*>();
	auto const has_skeleton = modeldata->skeleton != nullptr; 
	uint32_t gfx_ID{ std::numeric_limits<uint32_t>().max()};


	
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

			if (has_skeleton)
			{
				auto& renderer = gameobject->EnsureComponent<oo::SkinMeshRendererComponent>();
				assert(renderer.gfx_Object);
				renderer.SetModelHandle(asset, node->meshRef);
				gfx_ID = renderer.graphicsWorld_ID;
				renderer.gfx_Object->modelID = modeldata->meshResource;
				renderer.num_bones = modeldata->skeleton->inverseBindPose.size();

				//load Skeleton
				auto skeleton = CreateSkeleton(modeldata, gfx_ID, gameobject->GetComponent<oo::GameObjectComponent>().Id);
				containing_gameobj->AddChild(*skeleton);
			}
			else //no skeleton
			{
				auto& renderer = gameobject->EnsureComponent<oo::MeshRendererComponent>();
				renderer.SetModelHandle(asset, node->meshRef);
			}
			
			
			while ((node_parent.empty() == false) && (node->parent != node_parent.back().first))
			{
				node_parent.pop_back();
			}
			node_parent.back().second->AddChild(*gameobject, true);

		}
		node_parent.push_back({ node, gameobject });
		auto reversed_node = node->children;	// intentional copy
		std::reverse(reversed_node.begin(), reversed_node.end());
		for (auto data : reversed_node)
		{
			node_list.push(data);
		}
		//ImGui::TreeNodeEx(node->name.c_str(),ImGuiTreeNodeFlags_DefaultOpen| ImGuiTreeNodeFlags_NoTreePushOnOpen);
	}

	
}

std::shared_ptr<oo::GameObject> MeshHierarchy::CreateSkeleton(ModelFileResource* resource, uint32_t gfx_ID, oo::UUID uid)
{
	decltype(ModelFileResource::skeleton) skele = resource->skeleton;
	assert(skele);
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	skele->m_boneNodes;
	std::stack<oGFX::BoneNode*> node_list;
	node_list.push(skele->m_boneNodes);
	std::vector<std::pair<std::shared_ptr<oo::GameObject>, oGFX::BoneNode*>> parentnode;
	std::vector<std::pair<std::shared_ptr<oo::GameObject>, oGFX::BoneNode*>> all_nodes;
	auto rootbone = scene->CreateGameObjectImmediate(); // for return
	//set number of bones & model handle
	//auto& rendererComp = rootbone->EnsureComponent<oo::SkinMeshRendererComponent>();
	//rendererComp.num_bones = skele->inverseBindPose.size();
	//rendererComp.meshResource = resource->meshResource;
	//assert(rendererComp.gfx_Object);
	//rendererComp.gfx_Object->modelID = resource->meshResource;
	/*for (size_t i = 0; i < resource->numSubmesh; i++)
	{
		rendererComp.gfx_Object->submesh[i] = true;
	}
	auto graphicsWorld_ID = rendererComp.graphicsWorld_ID;*/
	auto bone = rootbone;
	oGFX::BoneNode* bonenode;
	while (true)
	{
		bonenode = node_list.top();
		node_list.pop();
		
		if (parentnode.size())
		{
			while (parentnode.back().second != bonenode->mpParent)
				parentnode.pop_back();

			parentnode.back().first->AddChild(*bone);
		}

		all_nodes.push_back(std::pair{ bone, bonenode });
		if (bonenode->mChildren.size())
		{
			auto reversed_node = bonenode->mChildren;	// intentional copy
			std::reverse(reversed_node.begin(), reversed_node.end());
			for (auto data : reversed_node)
			{
				node_list.push(data);
			}
			parentnode.push_back(std::pair{bone,bonenode});
		}

		if (node_list.empty() == false)
		{
			bone = scene->CreateGameObjectImmediate();
		}
		else
			break;
	}
	scene->GetWorld().Get_System<oo::TransformSystem>()->UpdateSubTree(*rootbone, false);
	int node_index = 0;
	for (auto& node : all_nodes)
	{
		auto* curr_bone = node.second;
		node.first->SetName(curr_bone->mName);
		auto& transform = node.first->EnsureComponent<oo::TransformComponent>();
		transform.SetLocalTransform(curr_bone->mModelSpaceGlobal);

		if (node_index == 0)
		{
			++node_index;
			continue; //skip root bone
		}
		auto& bonecomponent = node.first->EnsureComponent<oo::SkinMeshBoneComponent>();
		bonecomponent.bone_name = curr_bone->mName;
		bonecomponent.inverseBindPose_info = skele->inverseBindPose[curr_bone->m_BoneIndex];
		bonecomponent.graphicsWorld_ID = gfx_ID;
		bonecomponent.root_bone_object = uid;
		//bonecomponent.gfxbones_index = curr_bone->m_BoneIndex;

	   ++node_index;
	}
	return rootbone;
}

