#include "pch.h"
#include "PacketUtils.h"
#include "NetworkingEvent.h"
#include "Ouroboros/EventSystem/EventManager.h"
std::string PacketUtilts::ParseCommandData(const std::string& data, size_t& currentPos)
{
	size_t newpos = data.find(PacketUtilts::SEPERATOR, currentPos);
	std::string temp(data.begin() + currentPos, data.begin() + newpos);
	currentPos = newpos + 1;
	return std::move(temp);
}

void PacketUtilts::ProcessHeader(PacketHeader& header, std::string& data)
{
	size_t newpos = data.find(PacketUtilts::SEPERATOR);
	std::string temp(data.begin(), data.begin() + newpos);
	memcpy(&header, data.c_str(), newpos);
	data = std::string(data.begin() + (newpos + 1),data.end());
}

void PacketUtilts::PackHeaderIntoPacket(std::string& data,const PacketHeader& header)
{
	char buffer[sizeof(PacketHeader)];
	memcpy(buffer, &header, sizeof(PacketHeader));
	std::string convert_buff = buffer;
	convert_buff += PacketUtilts::SEPERATOR;
	data.insert(0, convert_buff);
}

void PacketUtilts::BroadCastCommand(CommandPacketType cpt, const std::string& data)
{
	if (PacketUtilts::is_connected == true)
	{
		NetworkingSendEvent e((char)cpt, data);
		oo::EventManager::Broadcast(&e);
	}
}
