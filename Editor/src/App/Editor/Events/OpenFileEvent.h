/************************************************************************************//*!
\file          OpenFileEvent.h
\project       Sandbox
\author        Leong Jun Xiang, junxiang.leong , 390007920
\par           email: junxiang.leong\@digipen.edu
\date          March 16, 2022
\brief          

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include <string>
#include <filesystem>
#include <unordered_map>
class OpenFileEvent :public oo::Event
{
public:
	enum class FileType
	{
		PREFAB,
		SCENE,
		IMAGE,
		AUDIO,
		ANIMATION,
		ANIMATION_CONTROLLER,
		CODE,
		FOLDER,
		OTHERS,
	};
	OpenFileEvent() :m_filepath{ "" }, m_type{0} {};
	OpenFileEvent(const std::filesystem::path& path) : m_filepath{ path }, m_type{ PathToFileType(path) } {};
	OpenFileEvent(const OpenFileEvent&  ofe) : m_filepath{ ofe.m_filepath }, m_type{ ofe.m_type } {};
	OpenFileEvent operator= (OpenFileEvent const& ofe) { this->m_filepath = ofe.m_filepath; this->m_type = ofe.m_type; return *this; };
	~OpenFileEvent() {};
	FileType m_type;
	std::filesystem::path m_filepath;
private:
	const FileType PathToFileType(const std::filesystem::path& path)
	{
		std::string ext = path.extension().string();
		if (ext.empty())
			return FileType::FOLDER;

		auto iter = s_typelist.find(ext);
		if (iter == s_typelist.end())
			return FileType::OTHERS;
		else
			return iter->second;

	}
	inline static std::unordered_map<std::string,FileType> s_typelist = 
	{
		{".png",FileType::IMAGE},
		{".prefab",FileType::PREFAB},
		{".scn",FileType::SCENE},
		{".anim",FileType::ANIMATION},
		{".controller",FileType::ANIMATION_CONTROLLER},
	};
};
