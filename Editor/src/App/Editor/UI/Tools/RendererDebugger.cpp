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
