/************************************************************************************//*!
\file          RendererDebugger.cpp
\project       Sandbox
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         a debugging feature for the renderer 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "RendererDebugger.h"
#include "SceneManagement/include/SceneManager.h"
#include "Ouroboros/Scene/Scene.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
RendererDebugger::RendererDebugger()
{
}

RendererDebugger::~RendererDebugger()
{
}

void RendererDebugger::Show()
{
	auto* graphics_world = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>()->GetGraphicsWorld();
	int counter = 0;
	for (auto& lights : graphics_world->m_HardcodedOmniLights)
	{
		++counter;
		
		ImGui::PushID(counter);
		ImGui::Separator();
		ImGui::Text("Lights %i", counter);
		ImGui::DragFloat4("Position", glm::value_ptr(lights.position), 0.1f);
		ImGui::ColorPicker4("Light Color", glm::value_ptr(lights.color));
		ImGui::DragFloat("Radius", glm::value_ptr(lights.radius), 0.1f);
		ImGui::Separator();
		ImGui::PopID();
	}
}
