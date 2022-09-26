/************************************************************************************//*!
\file           ImGui_Utill.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          contains Utility functions that dont generate UI 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <imgui/imgui.h>

class ImGuiUtilities
{
public:
	/*********************************************************************************//*!
	\brief    creates a image with tooltip on hover

	\param    tooltip_name ==> name of the button and desc when hovered over the button
	*//**********************************************************************************/
	static bool ImageButton_ToolTip(const char* tooltip_name, ImTextureID user_texture_id, const ImVec2& size, 
									const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1),
									int frame_padding = -1,
									const ImVec4& bg_col = ImVec4(0, 0, 0, 0), 
									const ImVec4& tint_col = ImVec4(1, 1, 1, 1)
									);
	/*********************************************************************************//*!
	\brief    creates a image button with tooltip on hover
	 
	\param    id ==> an unsigned int to differentiate the UIs from each other, 
					 reduce the hashing required. use when tooltip_name is very long.

	\param    tooltip_desc ==> description that pops out when when hovered
	*//**********************************************************************************/
	static bool ImageButton_ToolTip(const ImGuiID id,const char* tooltip_desc, ImTextureID user_texture_id, const ImVec2& size,
									const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1),
									int frame_padding = -1,
									const ImVec4& bg_col = ImVec4(0, 0, 0, 0),
									const ImVec4& tint_col = ImVec4(1, 1, 1, 1)
									);
private:

};

