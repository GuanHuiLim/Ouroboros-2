/************************************************************************************//*!
\file          AssetBrowser.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Browse all assets and can even interact with them. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "AssetBrowser.h"
#include "Project.h"
#include <imgui/imgui.h>
#include "OO_Vulkan/src/MeshModel.h"
#include "App/Editor/Utility/ImGuiStylePresets.h"
void AssetBrowser::AssetPickerUI(rttr::variant& data, bool& edited,int asset_type)
{
	ImGui::BeginChild("AssetBrowserWindow", { 0,ImGui::GetContentRegionAvail().y * 0.3f },true);
	switch (static_cast<oo::AssetInfo::Type>(asset_type))
	{
	case oo::AssetInfo::Type::Text:
		break;
	case oo::AssetInfo::Type::Texture:
		TextureUI(data, edited); break;
	case oo::AssetInfo::Type::Font:
		FontUI(data, edited); break;
	case oo::AssetInfo::Type::Audio:
		AudioUI(data, edited); break;
	case oo::AssetInfo::Type::Model:
		MeshUI(data, edited); break;
	case oo::AssetInfo::Type::AnimationTree:
		AnimationTreeUI(data, edited); break;

	}
	ImGui::EndChild();
}

void AssetBrowser::TextureUI(rttr::variant& data, bool& edited)
{
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
	ImGui::BeginTable("##Assets", static_cast<int>(windowSize.x / (ImGui_StylePresets::image_medium.x + spacing.x)));

	for (const auto& assets : Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::Texture))
	{
		ImGui::TableNextColumn();
		ImGui::BeginGroup();
		if (ImGui::ImageButton(assets.GetData<ImTextureID>(), ImGui_StylePresets::image_medium))
		{
			data.clear();
			data = assets;
			edited = true;
		}
		ImGui::Text(assets.GetFilePath().stem().string().c_str());
		ImGui::EndGroup();
	}
	ImGui::EndTable();
}

void AssetBrowser::FontUI(rttr::variant& data, bool& edited)
{
}

void AssetBrowser::AudioUI(rttr::variant& data, bool& edited)
{
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImVec2 spacing = ImGui::GetStyle().ItemSpacing;

	for (const auto& assets : Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::Audio))
	{
		if (ImGui::Selectable(assets.GetFilePath().stem().string().c_str()))
		{
			data.clear();
			data = assets;
			edited = true;
		}
	}
}

void AssetBrowser::MeshUI(rttr::variant& data, bool& edited)
{
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImVec2 spacing = ImGui::GetStyle().ItemSpacing;


	for (const auto& assets : Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::Model))
	{
		if (ImGui::Selectable(assets.GetFilePath().stem().string().c_str()))
		{
			data.clear();
			data = assets;
			edited = true;
		}
		/*auto* modeldata = assets.GetData<ModelFileResource*>();
		auto* node = modeldata->sceneInfo;
		size_t childSize = node->children.size();
		ImGuiTreeNodeFlags_ flag = childSize ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Bullet;
		bool opened = ImGui::TreeNodeEx(node->name.c_str(), flag);
		if (opened == false)
			continue;
		if(ImGui::IsItemClicked() && childSize == 0)
		{
			data.clear();
			data = assets;
			edited = true;
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
				childSize = node->children.size();
				flag = childSize ? ImGuiTreeNodeFlags_OpenOnArrow : ImGuiTreeNodeFlags_Bullet;
				opened = ImGui::TreeNodeEx(node->name.c_str(), flag);
				
				if (opened == false)
					continue;
				if (ImGui::IsItemClicked() && childSize == 0)
				{
					
					data.clear();
					data = assets;
					edited = true;
				}
			}
			node_parent.push_back({ node });
			for (auto data : node->children)
			{
				node_list.push(data);
			}
		}
		while (node_parent.empty() == false)
		{
			node_parent.pop_back();
			ImGui::TreePop();
		}*/
	}

}

void AssetBrowser::AnimationTreeUI(rttr::variant& data, bool& edited)
{
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImVec2 spacing = ImGui::GetStyle().ItemSpacing;

	for (const auto& assets : Project::GetAssetManager()->GetAssetsByType(oo::AssetInfo::Type::AnimationTree))
	{
		if (ImGui::Selectable(assets.GetFilePath().stem().string().c_str()))
		{
			data.clear();
			data = assets;
			edited = true;
		}
	}
}
