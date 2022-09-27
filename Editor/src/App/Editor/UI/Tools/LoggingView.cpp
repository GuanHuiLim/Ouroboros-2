/************************************************************************************//*!
\file          LoggingView.cpp
\project       Editor
\author        Muhammad Amirul Bin Zaol-kefli, muhammadamirul.b | code contribution (100%)
\par           email: muhammadamirul.b\@digipen.edu
\date          June 25, 2022
\brief         Logging System/UI for Editor 

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "LoggingView.h"
#include "Ouroboros/Core/LogCallbackSink.h"
#include "Utility/Hash.h"
#include "Ouroboros/Core/KeyCode.h"

#include <functional>
#include <Windows.h>
#include <shellapi.h>
#include <filesystem>

#include "App/Editor/Utility/ImGuiManager.h"
#include "App/Editor/Utility/ImGuiStylePresets.h"
#include <Ouroboros/TracyProfiling/OO_TracyProfiler.h>

std::deque<StringHash::size_type> LoggingView::s_messages;
std::vector<LoggingView::MessageData> LoggingView::s_msgCollection;
bool LoggingView::s_newItemAdded = false;
bool LoggingView::s_paused = false;
/**
 * \brief 
 *	default constructor 
 * (should only be called in the editor file)
 */
LoggingView::LoggingView()
{
	CallbackSink_mt::SubscribeToSink(AddItem);
}
void LoggingView::InitAsset()
{
	assets.emplace("LogsIcon", ImGuiManager::s_editorAssetManager.LoadName("LogsIcon.png")[0]);
	assets.emplace("WarningIcon", ImGuiManager::s_editorAssetManager.LoadName("WarningIcon.png")[0]);
	assets.emplace("ErrorIcon", ImGuiManager::s_editorAssetManager.LoadName("ErrorIcon.png")[0]);
}
/**
 * \brief 
 * displaying of UI
 * 
 * \features:
 * - Clear Logs
 * - Pause Logs
 * - Open File upon double clicking on log message
 * - Add [Filename] to indicate where the message comes from
 * - Add Filter to filter log messages by filename
 * - Rearrange collapsed logs based on log insertion
 * 
 * \TODO
 * - Collapsing Logs based on location it was called [ ]
 * - Opening file and going to the line (Currently can only open file, but cannot go to line) [ ]
 * - Export to txt file [ ]
 * 
 * \param active
 * 
 */
void LoggingView::Show()
{
	constexpr const char* const loggingview_update = "Logging View Update";
	TRACY_PROFILE_SCOPE_N(loggingview_update);

	static float imageSize = 50.0f;
	static LoggingView::MessageData msgitem;
	static std::string logmsgtype;
	bool interacted = false;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Clear"))
		{
			s_messages.resize(0);
			s_msgCollection.clear();
		}
		if (ImGui::MenuItem((s_paused)?"UnPause" : "Pause", NULL, s_paused))
		{
			s_paused = !s_paused;
		}
		if (ImGui::MenuItem((m_collapse_similar)? "Expand" : "Collapse", NULL, m_collapse_similar, true))
		{
			m_collapse_similar = !m_collapse_similar;
		}
		//add filter here
		ImGui::SameLine(ImGui::GetWindowWidth() * 0.745f);
		m_filter.Draw("##Filter", ImGui::GetWindowWidth());
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.755f);
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.5f,0.5f,0.5f,1 });
		if (!ImGui::IsItemActive() && !m_filter.CountGrep)
			ImGui::Text("Filter");
		ImGui::PopStyleColor();
		ImGui::EndMenuBar();
	}
	ImVec2 textSize = ImGui::CalcTextSize("a");
	if (ImGui::GetContentRegionAvail().x - imageSize <= 0 || ImGui::GetContentRegionAvail().y - imageSize <= 0)
	{
		return;
	}
	size_t textCount = static_cast<size_t>(std::floorf( ((ImGui::GetContentRegionAvail().x - imageSize)) / (textSize.x) * (imageSize / textSize.y - 1)));
	std::string msg_processor;
	msg_processor.resize(textCount +10);
	//draw ui here
	if (ImGui::BeginChild("LogView Child", { 0, ImGui::GetWindowHeight() - 160.0f }))
	{
		if (m_collapse_similar)
			DrawCollapsed(msgitem, msg_processor, interacted, textCount, textSize, imageSize);
		else
			DrawExpanded(msgitem, msg_processor, interacted, textCount, textSize);

		if (s_newItemAdded && !interacted)
		{
			s_newItemAdded = false;
			ImGui::SetScrollY(ImGui::GetScrollMaxY());//+100 just to force it to the max as max might be a value from the prev iter
		}
	}
	ImGui::EndChild();
	ImGui::SetCursorPosY(ImGui::GetWindowContentRegionMax().y - 100.f);
	ImGui::Separator();
	ImGui::BeginChild("FullLogView", { 0, 100.f }, false);
	ImGui::Text(m_msgitem.msg.c_str());
	ImGui::Text(LogMessageType(m_msgitem.type).c_str());
	ImGui::Text(m_msgitem.filename.c_str());
	ImGui::EndChild();

	TRACY_PROFILE_SCOPE_END();
}
/*********************************************************************************//*!
\brief    A callback function to attach to the spdlog's callback sink
 
\param    str
			the formatted message from spdlog
\param    type
			the logging type
\param    filename
			the file where the log came from

*//**********************************************************************************/
void LoggingView::AddItem(const std::string& str,char type,const std::string& filename)
{
	if (s_paused)
		return;
	s_newItemAdded = true;
	//when added new message set the scroll bar to the newest
	StringHash::size_type hash = StringHash::GenerateFNV1aHash(str);
	//emplace infomation about the logs
	s_messages.emplace_front(hash);
	//to track the count for the logs

	auto foundLogMsgID = [hash](const MessageData& msg) { return msg.id == hash; };
	if (std::find_if(s_msgCollection.begin(), s_msgCollection.end(), foundLogMsgID) == s_msgCollection.end())
		s_msgCollection.push_back({ 1,hash,type,str,filename });
	else
		std::find_if(s_msgCollection.begin(), s_msgCollection.end(), foundLogMsgID)->count += 1;
}

void LoggingView::LogSelectable(MessageData& item,
								std::string msg_processor,
								bool interacted,
								size_t textCount,
								float imageSize)
{

	constexpr const char* const logselectable_update = "Log Selectable";
	TRACY_PROFILE_SCOPE_N(logselectable_update);

	//Begin Draw Selectable Function
	if (ImGui::Selectable("##Item", &item.selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, imageSize)))
	{
		if (ImGui::IsItemHovered())
		{
			std::filesystem::path fpath = item.filename;
			// msg 0-7 => CLIENT:/ENGINE:
			// [fpath.stem().string()] => filename without full directory and extension
			// msg 8-End of string => actual logging message
			std::string msg = item.msg.substr(0, 7) + " [" + fpath.stem().string() + "] " + item.msg.substr(8);
			m_msgitem = item;
			m_msgitem.msg = msg;
			//m_msgtype = LogMessageType(item.type);

			if (item.selected == true)
			{
				if (ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LCTRL)))
				{
					s_selectedMsg.emplace_back(&(item));
				}
				else
				{
					for (auto* selected_msgs : s_selectedMsg)
						selected_msgs->selected = false;
					s_selectedMsg.clear();
					s_selectedMsg.emplace_back(&(item));
				}
			}
			else
			{
				if (ImGui::IsKeyDown(static_cast<int>(oo::input::KeyCode::LCTRL)))
				{
					auto itermsg = std::find(s_selectedMsg.begin(), s_selectedMsg.end(), &(item));
					s_selectedMsg.erase(itermsg);
				}
				else
				{
					for (auto* selected_msgs : s_selectedMsg)
						selected_msgs->selected = false;
					s_selectedMsg.clear();
				}
			}
		}
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			ShellExecuteA(NULL, "open", item.filename.c_str(), NULL, NULL, SW_SHOW);
	}
	ImGui::SameLine();
	switch (item.type)
	{
	case 0://trace
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.2f,0.5f,0.2f,1 });
		ImGui::Image(assets["LogsIcon"].GetData<ImTextureID>(), { imageSize,imageSize });
		break;
	case 1://debug
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.5f,0.5f,0.5f,1 });
		ImGui::Image(assets["LogsIcon"].GetData<ImTextureID>(), { imageSize,imageSize });
		break;
	case 2://info
		ImGui::PushStyleColor(ImGuiCol_Text, { 1,1,1,1 });
		ImGui::Image(assets["LogsIcon"].GetData<ImTextureID>(), { imageSize,imageSize });
		break;
	case 3://warn
		ImGui::PushStyleColor(ImGuiCol_Text, { 1,1,0,1 });
		ImGui::Image(assets["WarningIcon"].GetData<ImTextureID>(), { imageSize,imageSize });
		break;
	case 4://err
		ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f,0.3f,0.3f,1 });
		ImGui::Image(assets["ErrorIcon"].GetData<ImTextureID>(), { imageSize,imageSize });
		break;
	case 5://critical
		ImGui::PushStyleColor(ImGuiCol_Text, { 0.5f,0.5f,1,1 });
		ImGui::Image(assets["ErrorIcon"].GetData<ImTextureID>(), { imageSize,imageSize });
		break;
	}
	ImGui::SameLine();
	//Log Messages UI
	if (ImGui::IsItemHovered())
		interacted = true;
	ImGui::SameLine();
	{
		if (item.msg.size() > textCount)
		{
			std::filesystem::path fpath = item.filename;
			std::string msg = item.msg.substr(0, 7) + " [" + fpath.stem().string() + "] " + item.msg.substr(8);
			msg_processor = msg.substr(0, textCount);
			msg_processor += "...";
			ImGui::TextWrapped(msg_processor.c_str());
		}
		else
		{
			std::filesystem::path fpath = item.filename;
			std::string msg = item.msg.substr(0, 7) + " [" + fpath.stem().string() + "] " + item.msg.substr(8);
			ImGui::TextWrapped(msg.c_str());
		}
	}
	ImGui::PopStyleColor();
	//End Draw Selectable Function
	TRACY_PROFILE_SCOPE_END();
}

void LoggingView::DrawExpanded(MessageData& ,
							   std::string msg_processor, 
							   bool interacted, 
							   size_t textCount, 
							   ImVec2 textSize)
{

	ImGuiListClipper clipper;
	clipper.Begin(static_cast<int>(s_messages.size()), textSize.y);
	while (clipper.Step())
	{
		int distance = clipper.DisplayEnd - clipper.DisplayStart;
		int start = static_cast<int>(s_messages.size()) - 1 - clipper.DisplayStart;
		for (int i = start; i > start - distance; --i)
		{
			auto foundLogMsgID = [i](const MessageData& msg) { return msg.id == s_messages[i]; };
			auto& item = *(std::find_if(s_msgCollection.begin(), s_msgCollection.end(), foundLogMsgID));

			std::filesystem::path filterpath = item.filename;
			if (!m_filter.PassFilter(filterpath.stem().string().c_str()))
				continue;

			ImGui::PushID(i);
			ImGui::BeginGroup();
			//Begin Draw Selectable Function
			LogSelectable(item, msg_processor, interacted, textCount);
			//End Draw Selectable Function
			ImGui::EndGroup();
			ImGui::PopID();
			ImGui::Separator();
		}
	}
}

void LoggingView::DrawCollapsed(MessageData& ,
								std::string msg_processor,
								bool interacted,
								size_t textCount,
								ImVec2 textSize,
								float imageSize)
{
	ImGuiListClipper clipper;
	clipper.Begin(static_cast<int>(s_msgCollection.size()), imageSize + 2.0f * textSize.y);//height of one object = image + repeat count + seperator
	int counter = 0;
	while (clipper.Step())
	{
		for (auto iter = s_msgCollection.begin(); iter != s_msgCollection.end(); ++iter, ++counter)
		{
			std::string msgtype = "";
			std::filesystem::path filterpath = iter->filename;
			if (!m_filter.PassFilter(filterpath.stem().string().c_str()))
				continue;

			if (counter >= clipper.DisplayStart)
			{
				ImGui::PushID(iter->id);
				ImGui::BeginGroup();
				//Begin Draw Selectable Function
				LogSelectable(*iter, msg_processor, interacted, textCount, imageSize);
				//End Draw Selectable Function
				ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 25.f);
				ImGui::Text("%d", iter->count);
				ImGui::EndGroup();
				ImGui::PopID();
				ImGui::Separator();
				if (counter >= clipper.DisplayEnd)
					break;
			}
			else
				continue;
		}
	}
}

std::string LoggingView::LogMessageType(char type)
{
	switch (type)
	{
	case 0://trace
		return "Ouroboros LOG_TRACE(object)";
	case 1://debug
		return "Ouroboros LOG_DEBUG(object)";
	case 2://info
		return "Ouroboros LOG_INFO(object)";
	case 3://warn
		return "Ouroboros LOG_WARN(object)";
	case 4://err
		return "Ouroboros LOG_ERROR(object)";
	case 5://critical
		return "Ouroboros LOG_CRITICAL(object)";
	default:
		return "";
	}
}
