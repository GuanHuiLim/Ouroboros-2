#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include "glm/vec3.hpp"
class FocusButtonEvent : public oo::Event
{
public:
	glm::vec3 item_globalPosition;
};
