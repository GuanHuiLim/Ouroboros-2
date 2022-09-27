/************************************************************************************//*!
\file          WarningView.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution (100%)
\par           email: junxiang.leong\@digipen.edu
\date          October 3, 2022
\brief         UI for displaying the error/warning of the user action

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <string>
class WarningMessage
{
public:
	/*********************************************************************************//*!
	\brief    
	 Warning View UI
	 This function should only be invoked in Editor.cpp
	*//**********************************************************************************/
	void Show();
public:
	enum class DisplayType :int
	{
		DISPLAY_LOG,
		DISPLAY_WARNING,
		DISPLAY_ERROR
	};

	/*********************************************************************************//*!
	\brief    
	 Tells the UI to display the message 
	\param    type
	 type of message (warning / error / normal)
	\param    str 
	 the warning message
	*//**********************************************************************************/
	static void DisplayWarning(DisplayType type,const std::string& str,float time = 2.0f);
	static void DisplayToolTip(const std::string& str);

	static void PreImGuiWindowOperation();
private:

	inline static std::string s_WarningMessage;
	inline static float s_counter = 0.0f;
	inline static float s_position[2];
	inline static DisplayType s_dtype = DisplayType::DISPLAY_LOG;
	inline static bool s_ShowWarning = false;
	
};

