/************************************************************************************//*!
\file          LoggingView.h
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b
\par           email: muhammadamirul.b\@digipen.edu
\date          June 25, 2022
\brief         Logging System/UI for Editor  

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <deque>
#include <unordered_map>
#include <string>
#include <imgui/imgui.h>
class LoggingView
{
public:
	LoggingView();
	void Show();
	
protected:
	static void AddItem(const std::string&,char,const std::string&);
private:
	struct MessageData
	{
		int count = 0;
		std::uint32_t id = 0;
		char type = {'7'};
		std::string msg;
		std::string filename;
		bool selected = false;
	};
	static std::deque<std::uint32_t> s_messages;
	static std::vector<MessageData> s_msgCollection;
	static bool s_newItemAdded;
	static bool s_paused;

	std::vector<MessageData*> s_selectedMsg;

	bool m_collapse_similar = false;
	ImGuiTextFilter m_filter;
	MessageData m_msgitem;
	std::string m_msgtype;

	void LogSelectable(MessageData& item, 
					   std::string msg_processor, 
					   bool interacted, 
					   size_t textCount,
					   float imageSize = 0.0f);

	void DrawExpanded(MessageData& msgitem,
					  std::string msg_processor, 
					  bool interacted, 
					  size_t textCount,
					  ImVec2 textSize);

	void DrawCollapsed(MessageData& msgitem,
					   std::string msg_processor,
					   bool interacted,
					   size_t textCount,
					   ImVec2 textSize,
					   float imageSize);

	std::string LogMessageType(char type);
};


