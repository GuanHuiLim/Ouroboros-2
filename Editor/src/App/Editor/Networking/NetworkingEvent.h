#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include "App/Editor/Networking/PacketUtils.h"
#include <string>
#include <chrono>
#include <filesystem>
struct NetworkingSendEvent : public oo::Event
{
	char type = 0;
	std::string data;
	NetworkingSendEvent(char t, const std::string& _data) :type{ t }, data{ _data } {};
};
struct NetworkingFileTransferEvent : public oo::Event
{
	std::filesystem::path p;
};
struct NetworkingReceivedEvent : public oo::Event
{
	PacketHeader header;
	std::string data;
	NetworkingReceivedEvent(const PacketHeader& _header, const std::string& _data) :header{ _header }, data{ _data } {};
	NetworkingReceivedEvent(PacketHeader&& _header, std::string&& _data) :header{ _header }, data{ _data } {};
};
//received selection
struct NetworkingSelectionEvent : public oo::Event
{
	PacketHeader header;
	uint64_t gameobjID = 0;
	uint64_t time_triggered = 0;
	NetworkingSelectionEvent(PacketHeader& header,const std::string& data);
};
