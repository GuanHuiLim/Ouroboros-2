/************************************************************************************//*!
\file           ImGuiStylePresets.h
\project        Editor
\author         Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par            email: junxiang.leong\@digipen.edu
\date           September 26, 2022
\brief          contains the frequent sizes / color used in the editor 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <imgui/imgui.h>
class ImGui_StylePresets
{
public:
	inline static const ImVec2 image_small = { 20,20 };
	inline static const ImVec2 image_medium = { 50,50 };
	inline static const ImVec4 disabled_color = { 0.5f, 0.5f, 0.5f, 1.0f };
	inline static const ImVec4 prefab_text_color = { 0.1f, 0.5f, 0.9f, 1.0f };
private:

};

