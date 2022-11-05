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

class EditorViewport
{
public:
	EditorViewport();
	~EditorViewport();
	void Show();

	int m_gizmoOperation = 7;
private:
	int m_gizmoMode = 1;	// IMGUIZMO::MODE::WORLD <- default

	float m_viewportWidth, m_viewportHeight;
};