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
#include "Ouroboros/Asset/Asset.h"
class LoggingView
{
public:
	/*********************************************************************************//*!
	\brief Default Constructor for LoggingView Editor Tool.
	
	*//**********************************************************************************/
	LoggingView();

	void InitAsset();

	void Show();
	
protected:

	/*********************************************************************************//*!
	\brief    A callback function to attach to the spdlog's callback sink

	\param    str
				the formatted message from spdlog
	\param    type
				the logging type
	\param    filename
				the file where the log came from

	*//**********************************************************************************/
	static void AddItem(const std::string& str,char type,const std::string& filename);
private:
	std::unordered_map<std::string, oo::Asset> assets;

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

	/*********************************************************************************//*!
	\brief Creates a Selectable Object for the Log Message based on the MessageData 
		   received.
	
	\param item
				Reference to the MessageData that Log possess

	\param msg_processor
				Processes the MessageData into a string to be output onto the Editor

	\param interacted
				Checks if the Log Message Item has been selected/interacted

	\param textCount
				Total number of characters the editor can display,
				if message character is more than the textCount, message will be wrapped

	\param imageSize
				the Size of the Icon Image (Used for Collapsed Logs)

	*//**********************************************************************************/
	void LogSelectable(MessageData& item, 
					   std::string msg_processor, 
					   bool interacted, 
					   size_t textCount,
					   float imageSize = 0.0f);

	/*********************************************************************************//*!
	 \brief Displays the Expanded View of the LoggingView.
	 
	 \param msgitem
				Reference to the MessageData that Log possess

	 \param msg_processor
				Processes the MessageData into a string to be output onto the Editor

	 \param interacted
				Checks if the Log Message Item has been selected/interacted

	 \param textCount
				Total number of characters the editor can display,
				if message character is more than the textCount, message will be wrapped

	 \param textSize
				the Size of the Icon Image (Used for Collapsed Logs)

	*//**********************************************************************************/
	void DrawExpanded(MessageData& msgitem,
					  std::string msg_processor, 
					  bool interacted, 
					  size_t textCount,
					  ImVec2 textSize);

	/*********************************************************************************//*!
	 \brief Displays the Collapsed View of the LoggingView.

	 \param msgitem
				Reference to the MessageData that Log possess

	 \param msg_processor
				Processes the MessageData into a string to be output onto the Editor

	 \param interacted
				Checks if the Log Message Item has been selected/interacted

	 \param textCount
				Total number of characters the editor can display,
				if message character is more than the textCount, message will be wrapped

	 \param textSize
				the Size of the Icon Image (Used for Collapsed View)

	*//**********************************************************************************/
	void DrawCollapsed(MessageData& msgitem,
					   std::string msg_processor,
					   bool interacted,
					   size_t textCount,
					   ImVec2 textSize,
					   float imageSize);

	/*********************************************************************************//*!
	 \brief Function to determine the type of the particular log.
	 
	 \param type
				Type of Message for the Particular Log

	 \return 
				New string that contains the Type of Log together with the log message
				eg. Ouroboros LOG_TRACE(object)

	 *//**********************************************************************************/
	std::string LogMessageType(char type);
};


