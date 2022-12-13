/************************************************************************************//*!
\file          ToolbarButtonEvent.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Event for the toolbar buttons. 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include <string>
class ToolbarButtonEvent : public oo::Event
{
public:
	enum class ToolbarButton
	{
		TRANSFORM,
		ROTATE,
		SCALE,
		COMPILE,
		PLAY,
		PAUSE,
		STOP,
		OPENLIVESHARE,
	};
	ToolbarButtonEvent(ToolbarButton button) :m_buttonType{ button } {  };
	~ToolbarButtonEvent() {};
	ToolbarButton m_buttonType;
private:

};