#include "pch.h"
#include "NetworkingEvent.h"

NetworkingSelectionEvent::NetworkingSelectionEvent(PacketHeader& _header,const std::string& data)
:header{_header}
{
	size_t offset = 0;
	time_triggered = std::stoull(PacketUtilts::ParseCommandData(data, offset));
	gameobjID = std::stoull(std::string(data.begin() + offset, data.end()));
}
