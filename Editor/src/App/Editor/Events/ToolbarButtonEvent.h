#pragma once
#include "Ouroboros/EventSystem/Event.h"

class ToolbarButtonEvent
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
	};
	ToolbarButtonEvent(ToolbarButton button) :m_buttonType{ button } {  };
	~ToolbarButtonEvent() {};
	ToolbarButton m_buttonType;
private:

};