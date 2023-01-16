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

#include "Ouroboros/Physics/PhysicsSystem.h"
#include "Ouroboros/Vulkan/RendererSystem.h"


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
	oo::EventManager::Subscribe<ToolbarButtonEvent>(&OnPlayEvent);
	oo::EventManager::Subscribe<ToolbarButtonEvent>(&OnStopEvent);
	oo::EventManager::Subscribe<EditorViewport, FocusButtonEvent>(this, &EditorViewport::OnFocusEvent);
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
	bool cameraFocus = false;
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_::ImGuiFocusedFlags_ChildWindows))
	{
		cameraFocus = true;
	}

	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	//menu bar for the viewport
	MenuBar();

	//gizmo code here
	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImGui::GetWindowContentRegionMax();

	vMin.x += ImGui::GetWindowPos().x;
	vMin.y += ImGui::GetWindowPos().y;
	vMax.x += ImGui::GetWindowPos().x;
	vMax.y += ImGui::GetWindowPos().y;

	ImVec2 vpDim = { vMax.x - vMin.x ,vMax.y - vMin.y };


	m_cc.Update(oo::timer::dt(), cameraFocus);

	auto graphicsworld = scene->GetGraphicsWorld();
	graphicsworld->shouldRenderCamera[1] = true;
	auto& camera_matrices = EditorCamera.matrices;//perspective
	auto& window = oo::Application::Get().GetWindow();
	
	int windowWidth = window.GetSize().first;
	//int windowHeight = window.GetSize().second;	unused var
	

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

	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImVec2 cursor_screenpos = ImGui::GetCursorScreenPos();
	//ImVec2 mainWindowPos = ImGui::GetMainViewport()->Pos;
	if (ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left) && cameraFocus && Hierarchy::GetSelected().empty())
	{
		ImVec2 mousepos = ImGui::GetMousePos();
		mousepos.x -= cursor_screenpos.x;
		mousepos.y -= cursor_screenpos.y;

		mousepos.x = mousepos.x / contentRegion.x;
		mousepos.y = mousepos.y / contentRegion.y;
		if (mousepos.x > 0 && mousepos.y > 0 && mousepos.x < 1 && mousepos.y < 1)
		{
			auto graphicsID = VulkanRenderer::get()->GetPixelValue(0, { mousepos.x, mousepos.y });
			if (graphicsID >= 0)
			{
				LOG_TRACE("valid graphics ID from picking {0}", graphicsID);
				auto uuid = scene->GetUUIDFromGraphicsId(graphicsID); //scene->GetWorld().Get_System<oo::RendererSystem>()->GetUUID(graphicsID);
				ASSERT_MSG(uuid == oo::UUID::Invalid, " Attempting to pick on an object with invalid uuid {0}, this should not occur at this point!!!", uuid);
				Hierarchy::SetItemSelected(uuid);
			}
		}
	}

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
	float* snapping = nullptr;
	float snapping_offset[3] = {0.0,0.0,0.0};
	ImGuizmo::OPERATION op = (ImGuizmo::OPERATION)m_gizmoOperation;
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
	{
		switch (op)
		{
		case ImGuizmo::OPERATION::SCALE:
			snapping_offset[0] = 1.0f;snapping_offset[1] = 1.0f;snapping_offset[2] = 1.0f;
			break;
		case ImGuizmo::OPERATION::ROTATE:
			snapping_offset[0] = 45.0f;snapping_offset[1] = 45.0f;snapping_offset[2] = 45.0f;
			break;
		case ImGuizmo::OPERATION::TRANSLATE:
			snapping_offset[0] = 1.0f;snapping_offset[1] = 1.0f;snapping_offset[2] = 1.0f;
			break;
		}
		snapping = snapping_offset;
	}

	if (ImGuizmo::Manipulate(view, projection, op, (ImGuizmo::MODE)m_gizmoMode, glm::value_ptr(m_matrix),0, snapping))
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
	else
	{
		
		if (ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left) && !ImGuizmo::IsOver() && cameraFocus)
		{
			ImVec2 mousepos = ImGui::GetMousePos();
			
			mousepos.x -= cursor_screenpos.x;
			mousepos.y -= cursor_screenpos.y;

			mousepos.x = mousepos.x / contentRegion.x;
			mousepos.y = mousepos.y / contentRegion.y;
			if (mousepos.x > 0 && mousepos.y > 0 && mousepos.x < 1 && mousepos.y < 1)
			{
				auto graphicsID = VulkanRenderer::get()->GetPixelValue(0, { mousepos.x, mousepos.y });
				if (graphicsID >= 0)
				{
					LOG_TRACE("valid graphics ID from picking {0}", graphicsID);
					auto uuid = scene->GetUUIDFromGraphicsId(graphicsID); //scene->GetWorld().Get_System<oo::RendererSystem>()->GetUUID(graphicsID);
					ASSERT_MSG(uuid == oo::UUID::Invalid, " Attempting to pick on an object with invalid uuid {0}, this should not occur at this point!!!", uuid);
					Hierarchy::SetItemSelected(uuid);
				}
			}
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
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD);
		ChangeGizmoEvent e(m_gizmoOperation,m_gizmoMode);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
	}
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::E)) && ImGui::IsMouseDown(ImGuiMouseButton_Left) == false)
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::ROTATE);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD);
		ChangeGizmoEvent e(m_gizmoOperation, m_gizmoMode);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
	}

	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::R)) && ImGui::IsMouseDown(ImGuiMouseButton_Left) == false)
	{
		m_gizmoOperation = static_cast<int>(ImGuizmo::OPERATION::SCALE);
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::LOCAL);
		ChangeGizmoEvent e(m_gizmoOperation, m_gizmoMode);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
	}

	if (ImGui::IsKeyDown(ImGuiKey_::ImGuiKey_LeftShift))
	{
		m_gizmoMode = static_cast<int>(ImGuizmo::MODE::LOCAL);
		ChangeGizmoEvent e(m_gizmoOperation, m_gizmoMode);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
	}
	else if (ImGui::IsKeyReleased(ImGuiKey_::ImGuiKey_LeftShift))
	{
		switch (m_gizmoOperation)
		{
		case ImGuizmo::OPERATION::TRANSLATE:
			m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD); break;
		case ImGuizmo::OPERATION::ROTATE:
			m_gizmoMode = static_cast<int>(ImGuizmo::MODE::WORLD); break;
		case ImGuizmo::OPERATION::SCALE:
			m_gizmoMode = static_cast<int>(ImGuizmo::MODE::LOCAL); break;
		}
		ChangeGizmoEvent e(m_gizmoOperation, m_gizmoMode);
		oo::EventManager::Broadcast<ChangeGizmoEvent>(&e);
	}
	
	//wrong but it helps 
	if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(oo::input::KeyCode::F)) && 
		ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
	{
		FocusButtonEvent e;
		e.item_globalPosition = transform.GetGlobalPosition();
		oo::EventManager::Broadcast<FocusButtonEvent>(&e);
	}
}

void EditorViewport::UpdateWhenNotShown()
{
	auto scene = ImGuiManager::s_scenemanager->GetActiveScene<oo::Scene>();
	auto graphicsworld = scene->GetGraphicsWorld();
	graphicsworld->shouldRenderCamera[1] = false;
}

void EditorViewport::OnPlayEvent(ToolbarButtonEvent* e)
{
	if (e->m_buttonType == ToolbarButtonEvent::ToolbarButton::PLAY)
	{
		if (s_maximizeOnPlay)
		{
			if (s_windowStates.size())
				return;
			s_windowStates.clear();
			for (auto& window : ImGuiManager::s_GUIContainer)
			{
				s_windowStates.push_back(window.second.m_enabled);
				window.second.m_enabled = false;
			}
			ImGuiManager::GetItem("Toolbar").m_enabled = true;
			ImGuiManager::GetItem("Preview Window").m_enabled = true;
		}
		else
		{
			ImGui::SetWindowFocus("Logger");
			ImGui::SetWindowFocus("Preview Window");
		}
	}
}

void EditorViewport::OnStopEvent(ToolbarButtonEvent* e)
{
	if (e->m_buttonType == ToolbarButtonEvent::ToolbarButton::STOP)
	{
		if (s_maximizeOnPlay)
		{
			if (s_windowStates.empty() == true)
				return;
			int i = 0;
			for (auto& window : ImGuiManager::s_GUIContainer)
			{
				window.second.m_enabled = s_windowStates[i++];
			}
			s_windowStates.clear();
		}
		ImGui::SetWindowFocus("Editor Viewport");
	}
}

void EditorViewport::OnFocusEvent(FocusButtonEvent* e)
{
	glm::vec3 target = EditorCamera.GetFront();
	EditorCamera.SetPosition(e->item_globalPosition - (target * 10.0f));
}

void EditorViewport::MenuBar()
{
	if (ImGui::BeginMenuBar())
	{

		if (ImGui::BeginMenu("Debugging"))
		{
			if (ImGui::MenuItem("Colliders Debug Draw", 0, oo::PhysicsSystem::ColliderDebugDraw))
			{
				oo::PhysicsSystem::ColliderDebugDraw = !oo::PhysicsSystem::ColliderDebugDraw;
			}
			if (ImGui::MenuItem("Physics Debug Messages", 0, oo::PhysicsSystem::DebugMessages))
			{
				oo::PhysicsSystem::DebugMessages = !oo::PhysicsSystem::DebugMessages;
			}
			if (ImGui::MenuItem("Camera Debug Draw", 0, oo::RendererSystem::CameraDebugDraw))
			{
				oo::RendererSystem::CameraDebugDraw = !oo::RendererSystem::CameraDebugDraw;
			}
			if (ImGui::MenuItem("Light Debug Draw", 0, oo::RendererSystem::LightsDebugDraw))
			{
				oo::RendererSystem::LightsDebugDraw = !oo::RendererSystem::LightsDebugDraw;
			}

			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Maximize on Play",0, &s_maximizeOnPlay, true))
		{
			//s_maximizeOnPlay = !s_maximizeOnPlay;
		}
		ImGui::EndMenuBar();
	}
}
