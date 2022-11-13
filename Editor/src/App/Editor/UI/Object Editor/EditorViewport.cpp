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
#include "Ouroboros/Commands/CommandStackManager.h"
#include "Ouroboros/Commands/Component_ActionCommand.h"
#include "App/Editor/Events/GizmoOperationEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Core/Input.h"

#include "Ouroboros/Scene/EditorController.h"

#include "Ouroboros/EventSystem/EventTypes.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "Ouroboros/Core/Timer.h"

// Default settings for editor camera
Camera EditorViewport::EditorCamera = [&]()
{
	Camera camera;
	camera.m_CameraMovementType = Camera::CameraMovementType::firstperson;
	camera.movementSpeed = 5.0f;
	camera.SetPosition({ 0, 8, 8 });
	camera.Rotate({ 45, 180, 0 });
	return camera;
}();

EditorViewport::EditorViewport()
{
	ImGuizmo::AllowAxisFlip(false);
	m_cc.SetCamera(&EditorCamera);
}

EditorViewport::~EditorViewport()
{
}

void EditorViewport::Show()
{
	// Update Editor Camera
	// Camera controller updates editor camera
	m_cc.Update(oo::timer::dt(), true);

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto graphicsworld = scene->GetGraphicsWorld();
	auto& camera_matrices = EditorCamera.matrices;//perspective
	auto& window = oo::Application::Get().GetWindow();
	
	int windowWidth = window.GetSize().first;
	//int windowHeight = window.GetSize().second;	unused var
	
	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	ImVec2 vpDim = { vMax.x - vMin.x ,vMax.y - vMin.y };

	auto contentWidth = vpDim.x;
	auto contentHeight = vpDim.y;
	
	if (m_viewportWidth != contentWidth || m_viewportHeight != contentHeight)
	{
		//resize viewport
		m_viewportWidth = contentWidth;
		m_viewportHeight = contentHeight;

		EditorViewportResizeEvent e;
		e.X = m_viewportWidth;
		e.Y = m_viewportHeight;
		oo::EventManager::Broadcast<EditorViewportResizeEvent>(&e);
	}

	//framebuffer
	ImVec2 prevpos = ImGui::GetCursorPos();
	ImGui::Image(graphicsworld->imguiID[1], ImGui::GetContentRegionAvail());
	ImGui::SetCursorPos(prevpos);

	//ImVec2 mainWindowPos = ImGui::GetMainViewport()->Pos;



	//guarding against negative content sizes
	auto& selectedItems = Hierarchy::GetSelected();

	if (m_viewportWidth <= 0 || m_viewportHeight <= 0 || selectedItems.empty() )
	{
		return;
	}

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
	//ImGui::GetForegroundDrawList()->AddRect(vMin, {vMin.x + m_viewportWidth, vMin.y + m_viewportHeight}, ImU32(0xFF0000FF));

	ImGui::SetNextWindowSize({ m_viewportWidth, m_viewportHeight });
	ImGui::SetNextWindowPos(vMin);

	//window hole should be same size as content area
	ImGui::SetWindowHitTestHole(ImGui::GetCurrentWindow(), vMin, { m_viewportWidth, m_viewportHeight });

	// IMPORTANT: we now NEED to call this before begin frame
	ImGuizmo::SetRect(vMin.x, vMin.y, m_viewportWidth, m_viewportHeight);

	//keep a decent scaling for the guizmo
	float gizmoSize = windowWidth / m_viewportWidth;



	static constexpr float originalGuizmoSize = 0.1f;
	ImGuizmo::SetGizmoSizeClipSpace(originalGuizmoSize * gizmoSize);

	ImGuizmo::BeginFrame();
	oo::TransformComponent& transform = gameobject->GetComponent<oo::TransformComponent>();

	glm::mat4 m_matrix = transform.GlobalTransform;
	ImGuizmo::SetOrthographic(false);

	ImGuizmo::SetDrawlist();

	//ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(mTrans), glm::value_ptr(mRot), glm::value_ptr(mScale), glm::value_ptr(m_matrix));
	static rttr::variant before_edit;

	if (ImGuizmo::Manipulate(view, projection, (ImGuizmo::OPERATION)m_gizmoOperation, (ImGuizmo::MODE)m_gizmoMode, glm::value_ptr(m_matrix)))
	{
		if (before_edit.is_valid() == false)
		{
			switch (m_gizmoOperation)
			{
			case ImGuizmo::OPERATION::TRANSLATE:
				before_edit = transform.GetPosition();break;
			case ImGuizmo::OPERATION::ROTATE:
				before_edit = transform.GetRotationQuat();break;
			case ImGuizmo::OPERATION::SCALE:
				before_edit = transform.GetScale();break;
			}
		}
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
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)&& before_edit.is_valid())
	{
		rttr::variant after_edit;
		switch (m_gizmoOperation)
		{
		case ImGuizmo::OPERATION::TRANSLATE:
		{
			after_edit = transform.GetPosition();
			rttr::property prop = transform.get_type().get_property("Position");
			oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<oo::TransformComponent>(before_edit, after_edit, prop, gameobject->GetInstanceID()));
		}break;
		case ImGuizmo::OPERATION::ROTATE:
		{
			after_edit = transform.GetRotationQuat();
			rttr::property prop = transform.get_type().get_property("Quaternion");
			oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<oo::TransformComponent>(before_edit, after_edit, prop, gameobject->GetInstanceID()));
		}break;
		case ImGuizmo::OPERATION::SCALE:
		{
			after_edit = transform.GetScale();
			rttr::property prop = transform.get_type().get_property("Scaling");
			oo::CommandStackManager::AddCommand(new oo::Component_ActionCommand<oo::TransformComponent>(before_edit, after_edit, prop, gameobject->GetInstanceID()));
		}break;
		}
		before_edit.clear();
	}
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::W)) && ImGui::IsMouseDown(ImGuiMouseButton_Left) == false)
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::TRANSLATE);
		ChangeGizmoEvent e(m_gizmoOperation);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD);
	}
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::E)) && ImGui::IsMouseDown(ImGuiMouseButton_Left) == false)
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::ROTATE);
		ChangeGizmoEvent e(m_gizmoOperation);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD);
	}
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::R)) && ImGui::IsMouseDown(ImGuiMouseButton_Left) == false)
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::SCALE);
		ChangeGizmoEvent e(m_gizmoOperation);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::LOCAL);
	}
}
