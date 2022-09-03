/************************************************************************************//*!
\file          ToolbarView.h
\project       Ouroboros
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
class Toolbar
{
public:
	/*********************************************************************************//*!
	\brief
	 Ctor
	*//**********************************************************************************/
	Toolbar(float w = 30.0f, float h = 30.f) :btn_width(w),btn_height(h) {};
	/*********************************************************************************//*!
	\brief    
	 Displays the UI for toolbar
	 This function should only be invoked in Editor.cpp
	*//**********************************************************************************/
	void Show();
private:
	float btn_width;
	float btn_height;

	bool docking = false;
};

