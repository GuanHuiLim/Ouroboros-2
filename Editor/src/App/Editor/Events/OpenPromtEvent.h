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
	};
	/**
	 * _event - will trigger if yes is pressed
	 * actionAfterPrompt - will trigger as long a button is pressed
	 */
	OpenPromptEvent(DestructiveEvent _event, std::function<void()> actionAfterPrompt) :nextAction{ _event,actionAfterPrompt} {};
	~OpenPromptEvent() {};
	OpenPromptAction nextAction;
};
