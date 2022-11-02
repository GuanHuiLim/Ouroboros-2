#pragma once

#include "App/Editor/Networking/NetworkingEvent.h"
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
	void PrepareMessageSend(NetworkingSendEvent* e);
	void SendMsg(const std::string& msg);
	unsigned char GetPacketIdentifier(SLNet::Packet* p);
private:
	void AddMessage(std::string&& str);
	void ScrollToBottom();
	void MessageTypes(unsigned char id , unsigned char* data);
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