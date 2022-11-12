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


class EditorViewport
{
public:
	EditorViewport();
	~EditorViewport();
	void Show();

	int m_gizmoOperation = 7;

	// Editor Camera only exist within this viewport
	// default values found in cpp
	static Camera EditorCamera;

private:
	int m_gizmoMode = 1;	// IMGUIZMO::MODE::WORLD <- default
	float m_viewportWidth = 0.f, m_viewportHeight = 0.f;

	inline static CameraController m_cc;
};