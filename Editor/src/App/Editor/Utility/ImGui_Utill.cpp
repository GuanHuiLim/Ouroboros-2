/************************************************************************************//*!
\file           ImGui_Utill.cpp
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          Simple Utility Functions that don't create a new UI 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"

#include "ImGui_Utill.h"
#include "App/Editor/UI/Tools/WarningMessage.h"

#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>
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
	TRACY_PROFILE_SCOPE_NC(ImageButtonToolTip, tracy::Color::Gold3);

	ImGui::PushID(id);
	bool&& pressed = ImGui::ImageButton(user_texture_id, size, uv0, uv1, frame_padding,bg_col, tint_col);
	ImGui::PopID();

	WarningMessage::DisplayToolTip(tooltip_desc);

	TRACY_PROFILE_SCOPE_END();

	return pressed;
}
