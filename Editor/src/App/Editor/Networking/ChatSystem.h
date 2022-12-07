#pragma once

#include "App/Editor/Networking/NetworkingEvent.h"
#include "App/Editor/Events/ToolbarButtonEvent.h"
#include "slikenet/peerinterface.h"
#include "slikenet/statistics.h"
#include <string>
#include <deque>

#include "slikenet/SuperFastHash.h"
#include "slikenet/FileListTransfer.h"
#include "slikenet/FileListTransferCBInterface.h"
#include "slikenet/IncrementalReadInterface.h"

class FileCallback : public SLNet::FileListTransferCBInterface
{
public:
	bool OnFile(OnFileStruct* onFileStruct);
	virtual void OnFileProgress(FileProgressStruct* fps);
	virtual bool OnDownloadComplete(DownloadCompleteStruct* dcs);

};
class OuroFileListProgress : public SLNet::FileListProgress
{
	virtual void OnFilePush(const char* fileName, unsigned int fileLengthBytes, unsigned int offset, unsigned int bytesBeingSent, bool done, SLNet::SystemAddress targetSystem, unsigned short setID)
	{
		printf("Sending %s bytes=%u offset=%u\n", fileName, bytesBeingSent, offset);
	}

	virtual void OnFilePushesComplete(SLNet::SystemAddress systemAddress, unsigned short setID)
	{
		char str[32];
		systemAddress.ToString(true, (char*)str, static_cast<size_t>(32));
		RAKNET_DEBUG_PRINTF("File pushes complete to %s\n", str);
	}
	virtual void OnSendAborted(SLNet::SystemAddress systemAddress)
	{
		char str[32];
		systemAddress.ToString(true, (char*)str, static_cast<size_t>(32));
		RAKNET_DEBUG_PRINTF("Send aborted to %s\n", str);
	}
};
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
	/*********************************************************************************//*!
	\brief      Sends file to host
	\param      p ==> path of file relative to project
	
	*//**********************************************************************************/
	void SendFile(std::filesystem::path& p);
private://old code
	void ScrollToBottom();
private:
	void AddMessage(std::string&& str);
	void MessageTypes(unsigned char id , SLNet::Packet* pk);
private://Imgui
	void PopupUI();
	void HostUI();
	void JoinUI();
private:
	FileCallback file_cb;
	OuroFileListProgress filelistprogress;

	std::set<SLNet::SystemAddress> system_addresses;

	SLNet::RakNetStatistics* rss = 0;
	SLNet::RakPeerInterface* client = 0;

	SLNet::FileListTransfer flt;

	

	SLNet::Packet* p = 0;
	SLNet::SystemAddress clientID;
	SLNet::SocketDescriptor socketDescriptor;
	std::string ip, serverPort, clientPort,password;
	bool connected = false;
	bool open_UI = false;
	bool hosting = false;
private://imgui stuff
	std::deque<std::string> m_messages;
	std::string m_currentmessage;
	bool m_scrollToBottom = false;
};