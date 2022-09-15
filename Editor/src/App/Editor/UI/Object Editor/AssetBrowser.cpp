#include "pch.h"
#include "AssetBrowser.h"
#include "Project.h"
#include <imgui/imgui.h>

#include "App/Editor/Utility/ImGuiStylePresets.h"
void AssetBrowser::AssetPickerUI(rttr::variant& data, bool& edited)
{
	ImGui::BeginChild("AssetBrowserWindow", { 0,ImGui::GetContentRegionAvail().y * 0.3f },true);
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImVec2 spacing = ImGui::GetStyle().ItemSpacing;
	ImGui::BeginTable("##Assets",(windowSize.x/ (ImGui_StylePresets::image_medium.x + spacing.x)));
	for (const auto& assets : Project::GetAssetManager()->GetAssets())
	{
		if (assets.second.GetType() == oo::AssetInfo::Type::Texture)
		{
			ImGui::TableNextColumn();
			ImGui::BeginGroup();
			if (ImGui::ImageButton(assets.second.GetData<ImTextureID>(), ImGui_StylePresets::image_medium))
			{
				data.clear();
				data = assets.second;
				edited = true;
			}
			ImGui::Text(assets.second.GetFilePath().stem().string().c_str());
			ImGui::EndGroup();
		}
	}
	ImGui::EndTable();
	ImGui::EndChild();
}