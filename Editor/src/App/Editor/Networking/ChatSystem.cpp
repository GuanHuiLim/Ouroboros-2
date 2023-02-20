#include "pch.h"
#include "ChatSystem.h"
#include "slikenet/MessageIdentifiers.h"
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include "App/Editor/Utility/Windows_Utill.h"
#include "Ouroboros/EventSystem/EventManager.h"
#include "App/Editor/Networking/PacketUtils.h"

#include "Project.h"
#include "App/Editor/UI/Tools/WarningMessage.h"

#include "slikenet/FileOperations.h"
#include "slikenet/peerinterface.h"
#include "Ouroboros/Core/Timer.h"
#include <algorithm>
#include <fstream>
ChatSystem::ChatSystem()
	:client{SLNet::RakPeerInterface::GetInstance()},
	clientID{ SLNet::UNASSIGNED_SYSTEM_ADDRESS }
{
	oo::EventManager::Subscribe<ChatSystem, NetworkingSendEvent>(this, &ChatSystem::PrepareMessageSend);
	oo::EventManager::Subscribe<ChatSystem, NetworkingFileTransferEvent>(this, &ChatSystem::SendFile);
	oo::EventManager::Subscribe<ChatSystem, ToolbarButtonEvent>(this, &ChatSystem::OpenLiveShareUIEvent);
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
	PopupUI();
	if(connected)
	{
		if (sendFileCountDown)
		{
			countdown -= oo::timer::dt();
			if (countdown <= 0)
			{
				sendFileCountDown = false;
				SendFile(flt_filePath);
			}
		}
		for (p = client->Receive(); p; client->DeallocatePacket(p), p = client->Receive())
		{
			unsigned char packetIdentifier = GetPacketIdentifier(p);
			MessageTypes(packetIdentifier, p);
			if (hosting && packetIdentifier != 100)//101 is file transfer
			{
				char message[2048];
				sprintf_s(message, "%s", p->data);
				client->Send(message, static_cast<int>(strlen(message)), HIGH_PRIORITY, RELIABLE_ORDERED, 0, p->systemAddress, true);
			}
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
	if (connected == true && e->type == 100)
	{
		PacketUtilts::s_personalHeader.packetType = e->type;
		PacketUtilts::PackHeaderIntoPacket(e->data);
		m_messages.push_back(std::move(e->data));
	}
}

void ChatSystem::DownloadComplete(NetworkingSendEvent* e)
{
	if (e->type == 101 && hosting && connected)
	{
		SendFile(e->data);
	}
}

void ChatSystem::OpenLiveShareUIEvent(ToolbarButtonEvent* e)
{
	if (e->m_buttonType == ToolbarButtonEvent::ToolbarButton::OPENLIVESHARE)
	{
		if (open_UI == true)
		{
			ImGui::CloseCurrentPopup();
		}
		open_UI = !open_UI;
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

void ChatSystem::SendFile(const std::filesystem::path& path)
{
	std::ifstream in((Project::GetProjectFolder() / path), std::ios::binary);
	std::stringstream buffer;
	buffer << in.rdbuf();
	std::string file_data = buffer.str();
	unsigned int file_length = static_cast<unsigned int>(file_data.size());
	buffer.clear();
	in.close();
	if (file_length == 0)
	{
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_WARNING, "File Not Found!!");
		return;
	}
	//signal to host
	LOG_TRACE("Sending data");
	char msg = 101;
	client->Send(&msg, 2, HIGH_PRIORITY, RELIABLE_ORDERED, 0, SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);
	
	fileList = SLNet::FileList();
	fileList.Clear();
	fileList.AddFile(path.string().c_str(), (Project::GetProjectFolder() / path).make_preferred().string().c_str(), file_data.data() , file_length, file_length, FileListNodeContext(0, 0, 0, 0), false);
	LOG_TRACE("File Added");
}

void ChatSystem::SendFile(NetworkingFileTransferEvent* e)
{
	if (connected)
	{
		flt_filePath = e->p.make_preferred().string();
		size_t startpos = 0;
		UNREFERENCED_PARAMETER(startpos);
		//procsssing the string from \\ to /
		std::replace(flt_filePath.begin(), flt_filePath.end(), '\\', '/');
		countdown = 3.0f;
		sendFileCountDown = true;
		//SendFile("Assets/ParticleComponent.png");
	}
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
	case ID_NEW_INCOMING_CONNECTION:
	{
		system_addresses.emplace(pk->systemAddress);
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Someone Connected");
		if (hosting)
		{
			char msg = 20;
			client->Send(&msg, 2 , HIGH_PRIORITY, RELIABLE_ORDERED, 0, pk->systemAddress, false);
		}
	}break;
	case 20:
	{
		host_address = pk->systemAddress;
	}break;
	case ID_DISCONNECTION_NOTIFICATION:
	case ID_CONNECTION_LOST:
	{
		system_addresses.erase(pk->systemAddress);
	}break;
	case 100:
	{
		PacketHeader header;
		std::string data(reinterpret_cast<char*>(pk->data),pk->length);
		PacketUtilts::ProcessHeader(header,data);
		NetworkingReceivedEvent e(std::move(header), std::move(data));
		oo::EventManager::Broadcast(&e);
	}break;
	case 101:
	{
		unsigned short setid = flt.SetupReceive(&file_cb,false, pk->systemAddress);
		char msg = 102;
		std::string message;
		message += msg + std::to_string(setid);
		client->Send(message.c_str(), (int)(message.size() + 1), HIGH_PRIORITY, RELIABLE_ORDERED, 0, pk->systemAddress, false);
	}break;
	case 102:
	{
		//starts sending
		WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Sending File");
		SLNet::IncrementalReadInterface incrementalReadInterface;
		std::string msg = reinterpret_cast<char*>(pk->data + 1);
		unsigned short msgID = (unsigned short)std::stoul(msg);
		if (hosting)
		{
			//for (auto& address : system_addresses)
			//{
			flt.Send(&fileList, client, pk->systemAddress, msgID, HIGH_PRIORITY, 0, &incrementalReadInterface);
			/*}*/
		}
		else
		{
			flt.Send(&fileList, client, pk->systemAddress, msgID, HIGH_PRIORITY, 0, &incrementalReadInterface);
		}
	}break;
	};
}

void ChatSystem::PopupUI()
{
	if (open_UI)
	{
		ImGui::OpenPopup("Live Share");
		open_UI = false;
	}
	if (ImGui::BeginPopup("Live Share"))
	{
		if (ImGui::Button(hosting ? "join" : "host"))
			hosting = !hosting;
		if (connected == false)
		{
			if (hosting)
				HostUI();
			else
				JoinUI();

		}
		else
		{
			{
				SLNet::SystemAddress myadd = client->GetInternalID(SLNet::UNASSIGNED_SYSTEM_ADDRESS);
				ImGui::Text("My Addresses: %s \n", myadd.ToString(false));
			}
			{
				int i = 1;
				for (auto& sa : system_addresses)
				{
					ImGui::Text("Addresses %i. %s \n",i++, sa.ToString(false));
				}
			}
			ImGui::Separator();
			ImGui::Text("Port: %s", serverPort.c_str());
			ImGui::Text("Password: %s", password.c_str());
			if (ImGui::Button("Disconnect"))
			{
				//send disconnect message
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("sendgarbage"))
			{
				SendFile("Assets/ParticleComponent.png");
			}
		}
		ImGui::EndPopup();
	}
}

void ChatSystem::HostUI()
{
	ImGui::InputText("Port", &serverPort);
	ImGui::InputText("Password", &password);
	ImGui::InputText("User Name", PacketUtilts::s_personalHeader.name, 20);
	if (ImGui::Button("Host") && serverPort.empty() == false )
	{
		PacketUtilts::is_connected = true;
		connected = true;
		password += '\0';
		client->SetIncomingPassword(password.c_str(), (int)password.size() - 1);
		client->SetTimeoutTime(30000, SLNet::UNASSIGNED_SYSTEM_ADDRESS);

		SLNet::SocketDescriptor socketDescriptors[2];
		socketDescriptors[0].port = static_cast<unsigned short>(atoi(serverPort.c_str()));
		socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
		socketDescriptors[1].port = static_cast<unsigned short>(atoi(serverPort.c_str()));
		socketDescriptors[1].socketFamily = AF_INET6; // Test out IPV6
		bool b = client->Startup(4, socketDescriptors, 2) == SLNet::RAKNET_STARTED;
		if (!b)
		{
			printf("Failed to start dual IPV4 and IPV6 ports. Trying IPV4 only.\n");

			// Try again, but leave out IPV6
			b = client->Startup(4, socketDescriptors, 1) == SLNet::RAKNET_STARTED;
			if (!b)
			{
				puts("Server failed to start.  Terminating.");
				exit(1);
			}
		}
		client->SetMaximumIncomingConnections(4);

		client->AttachPlugin(&flt);
		client->SetSplitMessageProgressInterval(9);

		flt.AddCallback(&filelistprogress);
		flt.StartIncrementalReadThreads(1);

		client->SetOccasionalPing(true);
		client->SetUnreliableTimeout(1000);



		ImGui::CloseCurrentPopup();
	}
}

void ChatSystem::JoinUI()
{
	ImGui::InputText("IP", &ip);
	ImGui::InputText("Client Port", &clientPort);
	ImGui::InputText("Server Port", &serverPort);
	ImGui::InputText("Password", &password);
	ImGui::InputText("User Name", PacketUtilts::s_personalHeader.name, 20);
	if (ImGui::Button("Connect"))
	{
		if ((ip.empty() | serverPort.empty()) == false)
		{
			PacketUtilts::is_connected = true;
			connected = true;

			client->AllowConnectionResponseIPMigration(false);
			short client_port = (unsigned short)std::stoul(clientPort.c_str());
			short server_port = (unsigned short)std::stoul(serverPort.c_str());
			socketDescriptor = SLNet::SocketDescriptor(client_port, 0);
			socketDescriptor.socketFamily = AF_INET;
			client->Startup(8, &socketDescriptor, 1);

			//sending code
			//file list transfer
			client->AttachPlugin(&flt);
			client->SetSplitMessageProgressInterval(9);

			flt.AddCallback(&filelistprogress);
			flt.StartIncrementalReadThreads(1);


			client->SetOccasionalPing(true);
			password += '\0';
			[[maybe_unused]] SLNet::ConnectionAttemptResult car = client->Connect(ip.c_str(), server_port, password.c_str(), (int)password.size() - 1);
			RakAssert(car == SLNet::CONNECTION_ATTEMPT_STARTED);

			//m_messages.emplace_back("My Ip Addresses:");
			//unsigned int i;
			//for (i = 0; i < client->GetNumberOfAddresses(); i++)
			//{
			//	m_messages.emplace_back(std::to_string(i) + "  " + client->GetLocalIP(i));
			//}
			//m_messages.emplace_back("My GUID :");
			//m_messages.emplace_back(client->GetGuidFromSystemAddress(SLNet::UNASSIGNED_SYSTEM_ADDRESS).ToString());
		}
		ImGui::CloseCurrentPopup();
	}
}

bool FileCallback::OnFile(SLNet::FileListTransferCBInterface::OnFileStruct* onFileStruct)
{
	FILE* fp;
	std::string filename = (Project::GetProjectFolder()/onFileStruct->fileName).string();
	m_previousDownloadedFile = onFileStruct->fileName;
	fopen_s(&fp, filename.c_str(), "wb");
	fwrite(onFileStruct->fileData, onFileStruct->byteLengthOfThisFile, 1, fp);
	fclose(fp);
	//not going to verify if the transfer worked
	return true;
}

void FileCallback::OnFileProgress(SLNet::FileListTransferCBInterface::FileProgressStruct* fps)
{
}

bool FileCallback::OnDownloadComplete(SLNet::FileListTransferCBInterface::DownloadCompleteStruct* dcs)
{
	WarningMessage::DisplayWarning(WarningMessage::DisplayType::DISPLAY_LOG, "Asset Downloaded");
	NetworkingSendEvent nse(101, m_previousDownloadedFile);
	oo::EventManager::Broadcast<NetworkingSendEvent>(&nse);
	return false;
}
