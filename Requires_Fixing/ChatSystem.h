#pragma once
#include "slikenet/peerinterface.h"
#include "slikenet/statistics.h"
#include <string>
#include <deque>
class ChatSystem
{
public:
	ChatSystem();
	~ChatSystem();

	void Show();
	unsigned char GetPacketIdentifier(SLNet::Packet* p);
private:
	void AddMessage(std::string&& str);
	void ScrollToBottom();
private:
	SLNet::RakNetStatistics* rss = 0;
	SLNet::RakPeerInterface* client = 0;
	SLNet::Packet* p = 0;
	SLNet::SystemAddress clientID;
	SLNet::SocketDescriptor socketDescriptor;
	std::string ip, serverPort, clientPort;
	bool connected = false;
private://imgui stuff
	std::deque<std::string> m_messages;
	std::string m_currentmessage;
	bool m_scrollToBottom = false;
};