/************************************************************************************//*!
\file          EditorViewport.cpp
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         a viewport for the editor to allow the gizmo to interact with the object. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <imgui/ImGuizmo.h>
#include "EditorViewport.h"
#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Core/WindowsWindow.h"
#include "Ouroboros/Vulkan/VulkanContext.h"

#include "SceneManagement/include/SceneManager.h"
#include "Ouroboros/Scene/Scene.h"

#include "Ouroboros/ECS/GameObject.h"
#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/UI/Object Editor/Hierarchy.h"
#include "Ouroboros/Transform/TransformComponent.h"

#include "Ouroboros/Core/Input.h"
EditorViewport::EditorViewport()
{
}

EditorViewport::~EditorViewport()
{
}

void EditorViewport::Show()
{

	auto& camera_matrices = oo::Application::Get().GetWindow().GetVulkanContext()->getRenderer()->camera.matrices;//perspective
	auto& window = oo::Application::Get().GetWindow();
	
	int windowWidth = window.GetSize().first;
	int windowHeight = window.GetSize().second;
	
	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	ImVec2 vpDim = { vMax.x - vMin.x ,vMax.y - vMin.y };

	//auto contentWidth = vMax.x - vMin.x;
	//auto contentHeight = vMax.y - vMin.y;

	auto contentWidth = float(windowWidth);
	auto contentHeight = float(windowHeight);
	vMin.x = 0;
	vMin.y = 0;
	vMax.x = contentWidth;
	vMax.y = contentHeight;

	ImVec2 mainWindowPos = ImGui::GetMainViewport()->Pos;



	//guarding against negative content sizes
	auto& selectedItems = Hierarchy::GetSelected();

	if (contentWidth <= 0 || contentHeight <= 0 || selectedItems.empty() )
	{
		return;
	}

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto gameobject = scene->FindWithInstanceID(*selectedItems.begin());
	if (gameobject == nullptr || scene->IsValid(*gameobject) == false)
	{
		return;
	}

	const float* view;
	const float* projection;

	view = glm::value_ptr(camera_matrices.view);
	projection = glm::value_ptr(camera_matrices.perspective);

	//Debug Red box
	//ImGui::GetForegroundDrawList()->AddRect(vMin, {vMin.x + contentWidth, vMin.y+contentHeight}, ImU32(0xFF0000FF));

	ImGui::SetNextWindowSize({ contentWidth,contentHeight });
	ImGui::SetNextWindowPos(vMin);

	//window hole should be same size as content area
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), vMin, { contentWidth,contentHeight });

	// IMPORTANT: we now NEED to call this before begin frame
	ImGuizmo::SetRect(vMin.x, vMin.y, contentWidth, contentHeight);

	//keep a decent scaling for the guizmo
	float gizmoSize = windowWidth / contentWidth;



	static constexpr float originalGuizmoSize = 0.1f;
	ImGuizmo::SetGizmoSizeClipSpace(originalGuizmoSize * gizmoSize);

	ImGuizmo::BeginFrame();
	oo::TransformComponent& transform = gameobject->GetComponent<oo::TransformComponent>();

	glm::mat4 m_matrix = transform.GlobalTransform;
	ImGuizmo::SetOrthographic(false);

	ImGuizmo::SetDrawlist();

	//ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(mTrans), glm::value_ptr(mRot), glm::value_ptr(mScale), glm::value_ptr(m_matrix));

	if (ImGuizmo::Manipulate(view, projection, (ImGuizmo::OPERATION)m_gizmoOperation, (ImGuizmo::MODE)m_gizmoMode, glm::value_ptr(m_matrix)))
	{
		if (ImGuizmo::IsUsing())
		{
			glm::vec3 mScale = transform.GetGlobalScale();
			glm::quat mRot = transform.GetRotationQuat();
			glm::vec3 mTrans = transform.GetGlobalPosition();

			/*ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(m_matrix),
				glm::value_ptr(mTrans),
				glm::value_ptr(mRot),
				glm::value_ptr(mScale));*/
			
			// If we can't trust imguizmo, we can still trust glm.
			Transform3D::DecomposeValues(m_matrix, mTrans, mRot, mScale);

			transform.SetGlobalTransform(mTrans, mRot, mScale);
			//transform.SetGlobalTransform(m_matrix); <- DONT call this, IT WONT work.
		}
	}
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::Q)))
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::TRANSLATE);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD);
	}
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::W)))
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::ROTATE);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD);
	}
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::E)))
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::SCALE);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::LOCAL);
	}
}
