#include "pch.h"

#include "ImGui_Utill.h"
#include "Editor/UI/Tools/WarningMessage.h"

bool ImGuiUtilities::ImageButton_ToolTip(const char* tooltip_name, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	ImGui::PushID(tooltip_name);
	bool&& pressed = ImGui::ImageButton(user_texture_id,size, uv0, uv1, frame_padding,bg_col, tint_col);
	ImGui::PopID();
	WarningMessage::DisplayToolTip(tooltip_name);
	
	return pressed;
}

bool ImGuiUtilities::ImageButton_ToolTip(const ImGuiID id, const char* tooltip_desc, ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	ImGui::PushID(id);
	bool&& pressed = ImGui::ImageButton(user_texture_id, size, uv0, uv1, frame_padding,bg_col, tint_col);
	ImGui::PopID();

	WarningMessage::DisplayToolTip(tooltip_desc);
	return pressed;
}
