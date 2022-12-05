#pragma once

#include "App/Editor/Networking/NetworkingEvent.h"
#include "App/Editor/Events/ToolbarButtonEvent.h"
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
	void OpenLiveShareUIEvent(ToolbarButtonEvent* e);
	void SendMsg(const std::string& msg);
	unsigned char GetPacketIdentifier(SLNet::Packet* p);
private://old code
	void ScrollToBottom();
private:
	void AddMessage(std::string&& str);
	void MessageTypes(unsigned char id , SLNet::Packet* pk);
private://Imgui
	void PopupUI();
private:
	SLNet::RakNetStatistics* rss = 0;
	SLNet::RakPeerInterface* client = 0;
	SLNet::Packet* p = 0;
	SLNet::SystemAddress clientID;
	SLNet::SocketDescriptor socketDescriptor;
	std::string ip, serverPort, clientPort;
	bool connected = false;
	bool open_UI = false;
private://imgui stuff
	std::deque<std::string> m_messages;
	std::string m_currentmessage;
	bool m_scrollToBottom = false;
};