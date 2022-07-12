#include "ChatSystem.h"
#include "slikenet/MessageIdentifiers.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "Editor/Utility/Windows_Utill.h"

ChatSystem::ChatSystem()
	:client{SLNet::RakPeerInterface::GetInstance()},
	clientID{ SLNet::UNASSIGNED_SYSTEM_ADDRESS }
{
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
		ImGui::InputText("IP", &ip);
		ImGui::InputText("Port", &serverPort);
		if (ImGui::Button("Connect"))
		{
			if ((ip.empty() | serverPort.empty()) == false)
			{
				connected = true;
				socketDescriptor = SLNet::SocketDescriptor(atoi(clientPort.c_str()), 0);
				socketDescriptor.socketFamily = AF_INET;
				client->Startup(8, &socketDescriptor, 1);
				client->SetOccasionalPing(true);
				SLNet::ConnectionAttemptResult car = client->Connect(ip.c_str(), atoi(serverPort.c_str()), "Rumpelstiltskin", (int)strlen("Rumpelstiltskin"));
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
	}
	else
	{
		ImVec2 content_size = ImGui::GetContentRegionAvail();
		ImGui::BeginChild("Text Window", { content_size.x,content_size.y * 0.8f });
		for (auto& message : m_messages)
		{
			ImGui::Text(message.c_str());
		}
		ScrollToBottom();
		ImGui::EndChild();
		if (ImGui::InputText("##sendmessage", &m_currentmessage, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			client->Send(m_currentmessage.c_str(), (int)m_currentmessage.size() + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			AddMessage(std::move(m_currentmessage));
			m_currentmessage.clear();
		}
		for (p = client->Receive(); p; client->DeallocatePacket(p), p = client->Receive())
		{
			unsigned char packetIdentifier = GetPacketIdentifier(p);
			switch (packetIdentifier)
			{
			case ID_UNCONNECTED_PING:
			{
					std::string temp = p->systemAddress.ToString(true);
					temp += " Ping From";
					m_messages.emplace_back(temp);
					break;
			}
			default:
			{
				AddMessage(reinterpret_cast<char*>(p->data));
				break;
			}
			}
		}
	}
}

unsigned char ChatSystem::GetPacketIdentifier(SLNet::Packet* _p)
{
	if (_p == 0)
		return 255;

	if ((unsigned char)_p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(_p->length > sizeof(SLNet::MessageID) + sizeof(SLNet::Time));
		return (unsigned char)_p->data[sizeof(SLNet::MessageID) + sizeof(SLNet::Time)];
	}
	else
		return (unsigned char)_p->data[0];
}

void ChatSystem::AddMessage(std::string&& str)
{
	m_messages.emplace_back(str);
	m_scrollToBottom = true;
	WindowsUtilities::Windows_Beep_Exclaimation();
}

void ChatSystem::ScrollToBottom()
{
	if (m_scrollToBottom)
	{
		ImGui::SetScrollHereY(1.0f);
		m_scrollToBottom = false;
	}
}
