/************************************************************************************//*!
\file          ToolbarView.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution (100%)
\par           email: junxiang.leong\@digipen.edu
\date          October 3, 2021
\brief         Toolbar UI at the top side of the application

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <string>
#include <unordered_map>
#include "Ouroboros/Asset/Asset.h"
#include "App/Editor/Events/GizmoOperationEvent.h"
class Toolbar
{
public:
	/*********************************************************************************//*!
	\brief
	 Ctor
	*//**********************************************************************************/

	Toolbar() {};
	void InitAssets();
	/*********************************************************************************//*!
	\brief    
	 Displays the UI for toolbar
	 This function should only be invoked in Editor.cpp
	*//**********************************************************************************/
	void Show();
	void OnGizmoChange(ChangeGizmoEvent* e);
private:
private:
	int currGizmoOperation = 7;
	std::unordered_map<std::string,oo::Asset> m_iconsSaved;
	bool docking = false;
};

