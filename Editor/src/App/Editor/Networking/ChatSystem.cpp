#include "pch.h"
#include "ChatSystem.h"
#include "slikenet/MessageIdentifiers.h"
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "App/Editor/Utility/Windows_Utill.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "App/Editor/Networking/PacketUtils.h"
ChatSystem::ChatSystem()
	:client{SLNet::RakPeerInterface::GetInstance()},
	clientID{ SLNet::UNASSIGNED_SYSTEM_ADDRESS }
{
	oo::EventManager::Subscribe<ChatSystem, NetworkingSendEvent>(this, &ChatSystem::PrepareMessageSend);
}

ChatSystem::~ChatSystem()
{
	// Be nice and let the server know we quit.
	client->Shutdown(300);

	// We're done with the network
	SLNet::RakPeerInterface::DestroyInstance(client);
}

void ChatSystem::Show()
{
	if (connected == false)
	{
		ImGui::Begin("Live Share",0);
		ImGui::InputText("IP", &ip);
		ImGui::InputText("Port", &serverPort);
		ImGui::InputText("User Name", PacketUtilts::s_personalHeader.name, 20);
		if (ImGui::Button("Connect"))
		{
			if ((ip.empty() | serverPort.empty()) == false)
			{
				PacketUtilts::is_connected = true;
				connected = true;
				short client_port = (unsigned short)std::stoul(clientPort.c_str());
				short server_port = (unsigned short)std::stoul(serverPort.c_str());
				socketDescriptor = SLNet::SocketDescriptor(client_port, 0);
				socketDescriptor.socketFamily = AF_INET;
				client->Startup(8, &socketDescriptor, 1);
				client->SetOccasionalPing(true);
				SLNet::ConnectionAttemptResult car = client->Connect(ip.c_str(), server_port, "Rumpelstiltskin", (int)strlen("Rumpelstiltskin"));
				RakAssert(car == SLNet::CONNECTION_ATTEMPT_STARTED);

				m_messages.emplace_back("My Ip Addresses:");
				unsigned int i;
				for (i = 0; i < client->GetNumberOfAddresses(); i++)
				{
					m_messages.emplace_back(std::to_string(i) + "  " + client->GetLocalIP(i));
				}
				m_messages.emplace_back("My GUID :");
				m_messages.emplace_back(client->GetGuidFromSystemAddress(SLNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
			}
		}
		ImGui::End();
	}
	else
	{
		for (p = client->Receive(); p; client->DeallocatePacket(p), p = client->Receive())
		{
			unsigned char packetIdentifier = GetPacketIdentifier(p);
			MessageTypes(packetIdentifier, p);
		}
	}
	if (m_messages.empty() == false)
	{
		SendMsg(m_messages.front());
		m_messages.pop_front();
	}
}

void ChatSystem::PrepareMessageSend(NetworkingSendEvent* e)
{
	if (connected == true)
	{
		PacketUtilts::s_personalHeader.packetType = e->type;
		PacketUtilts::PackHeaderIntoPacket(e->data);
		m_messages.push_back(std::move(e->data));
	}
}

void ChatSystem::SendMsg(const std::string& msg)
{
	client->Send(msg.c_str(), (int)msg.size() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

unsigned char ChatSystem::GetPacketIdentifier(SLNet::Packet* _p)
{
	if (_p == 0)
		return 255;

	return (unsigned char)_p->data[0];
}

void ChatSystem::AddMessage(std::string&& str)
{
	m_messages.emplace_back(str);
	m_scrollToBottom = true;
	//WindowsUtilities::Windows_Beep_Exclaimation();
}

void ChatSystem::ScrollToBottom()
{
	if (m_scrollToBottom)
	{
		ImGui::SetScrollHereY(1.0f);
		m_scrollToBottom = false;
	}
}

void ChatSystem::MessageTypes(unsigned char id, SLNet::Packet* pk)
{
	switch (id)
	{
	case ID_UNCONNECTED_PING:
	{
		std::string temp = p->systemAddress.ToString(true);
		temp += " Ping From";
		m_messages.emplace_back(temp);
		break;
	}
	case 100:
	{
		PacketHeader header;
		std::string data(reinterpret_cast<char*>(pk->data),pk->length);
		PacketUtilts::ProcessHeader(header,data);
		NetworkingReceivedEvent e(std::move(header), std::move(data));
		oo::EventManager::Broadcast(&e);
	}
	};
}

