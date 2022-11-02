#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include <string>
struct NetworkingSendEvent : public oo::Event
{
	char type = 0;
	std::string data;
	NetworkingSendEvent(char t, const std::string& _data) :type{ t }, data{ _data } {};
};

struct NetworkingReceivedEvent : public oo::Event
{
	char type = 0;
	std::string data;
	NetworkingReceivedEvent(char t, const std::string& _data) :type{ t }, data{ _data } {};
};