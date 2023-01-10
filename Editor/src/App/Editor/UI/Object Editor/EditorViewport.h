/************************************************************************************//*!
\file          EditorViewport.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         An Editor Viewport for the users to edit the objects using gizmo 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "OO_Vulkan/src/Camera.h"
#include "Ouroboros/Core/CameraController.h"
#include "App/Editor/Events/ToolbarButtonEvent.h"
#include "App/Editor/Events/FocusButtonEvent.h"
class EditorViewport
{
public:
	EditorViewport();
	~EditorViewport();
	void Show();

	int m_gizmoOperation = 7;
	int m_gizmoMode = 1;	// IMGUIZMO::MODE::WORLD <- default

	// Editor Camera only exist within this viewport
	// default values found in cpp
	static Camera EditorCamera;
private:
	void MenuBar();

	float m_viewportWidth = 0.f, m_viewportHeight = 0.f;

	inline static CameraController m_cc;
private://maximize on play
	inline static bool s_maximizeOnPlay = false;
	inline static std::vector<bool> s_windowStates;
public: //maximize on play
	static void OnPlayEvent(ToolbarButtonEvent* e);
	static void OnStopEvent(ToolbarButtonEvent* e);
	void OnFocusEvent(FocusButtonEvent* e);
};