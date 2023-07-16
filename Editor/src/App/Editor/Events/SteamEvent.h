#pragma once
#include "Ouroboros/EventSystem/Event.h"
struct SteamEvent : public oo::Event
{
public:
	SteamEvent(bool o_active) :overlay_active{ o_active } {};

	bool overlay_active = false;
};