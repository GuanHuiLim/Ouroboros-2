#include "pch.h"
#include "AssetBrowser.h"
#include "Project.h"
#include <imgui/imgui.h>

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
	}
	ImGui::EndChild();
}

void AssetBrowser::TextureUI(rttr::variant& data, bool& edited)
{
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
	ImGui::BeginTable("##Assets", (windowSize.x / (ImGui_StylePresets::image_medium.x + spacing.x)));

	for (const auto& assets : Project::GetAssetManager()->GetLoadedAssetsByType(oo::AssetInfo::Type::Texture))
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
}
