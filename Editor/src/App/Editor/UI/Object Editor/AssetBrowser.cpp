#include "pch.h"
#include "AssetBrowser.h"
#include "Project.h"
#include <imgui/imgui.h>
#include "App/Editor/Utility/ImGuiStylePresets.h"
void AssetBrowser::AssetPickerUI(rttr::variant& data, bool& edit, bool& edited)
{

	ImGui::BeginChild("AssetBrowserWindow", { 0,ImGui::GetContentRegionAvail().y * 0.3f },true);
	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	ImGui::BeginTable("##Assets",(windowSize.x/ ImGui_StylePresets::image_medium.x));
	for (const auto& assets : Project::GetAssetManager()->GetAssets())
	{
		//assets.second.GetData()
	}
	ImGui::EndTable();
	ImGui::EndChild();
}
