/************************************************************************************//*!
\file          OpenPromtEvent.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         An Event based redirection that takes in another event but triggers
			   Prompt and base on the options selected by the user on the prompt will
			   decides whether the event triggers.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include <functional>
template <typename DestructiveEvent>
class OpenPromptEvent : public oo::Event
{
public:
	struct OpenPromptAction
	{
		DestructiveEvent nextEvent;
		std::function<void()> nextAction;
		bool launchEventDesipteNo = true;
	};
	/**
	 * _event - will trigger if yes is pressed
	 * actionAfterPrompt - will trigger as long a button is pressed
	 */
	OpenPromptEvent(DestructiveEvent _event, std::function<void()> actionAfterPrompt,bool launchDesipteNo = true) :nextAction{ _event,actionAfterPrompt,launchDesipteNo } {};
	~OpenPromptEvent() {};
	OpenPromptAction nextAction;
};
