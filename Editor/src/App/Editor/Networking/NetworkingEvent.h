#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include "App/Editor/Networking/PacketUtils.h"
#include <string>
struct NetworkingSendEvent : public oo::Event
{
	char type = 0;
	std::string data;
	NetworkingSendEvent(char t, const std::string& _data) :type{ t }, data{ _data } {};
};

struct NetworkingReceivedEvent : public oo::Event
{
	PacketHeader header;
	std::string data;
	NetworkingReceivedEvent(const PacketHeader& _header, const std::string& _data) :header{ _header }, data{ _data } {};
	NetworkingReceivedEvent(PacketHeader&& _header, std::string&& _data) :header{ _header }, data{ _data } {};
};