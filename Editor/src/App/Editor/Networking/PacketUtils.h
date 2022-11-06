#pragma once
#include <string>
enum class CommandPacketType :int
{
	ActionObject = 1,
	DeleteObject,
	CreateObject,
	ReorderObject,
	AddComponentObject,
	RemoveComponentObject,
	ActionScript,
	AddScript,
	RemoveScript,
	UNDO_command,
	REDO_command,
	Selected_Object,
};
struct PacketHeader
{
	char packetIdentifyer = 100;
	char packetType = 0;
	char name[20] = {0};
};
class PacketUtilts
{
public:
	inline static bool is_connected = false;
	inline static constexpr char SEPERATOR = -100;
	inline static PacketHeader s_personalHeader;
	static std::string ParseCommandData(const std::string& data, size_t& currentPos);
	/*********************************************************************************//*!
	\brief      this function will process the data and populate the header with info
	 
	\param      header ==> will be populated
	\param      data ==> header will be trimmed out
	
	*//**********************************************************************************/
	static void ProcessHeader(PacketHeader& header, std::string& data);
	
	static void PackHeaderIntoPacket(std::string& data, const PacketHeader& header = s_personalHeader);

	static void BroadCastCommand(CommandPacketType cpt, const std::string& data);
};
