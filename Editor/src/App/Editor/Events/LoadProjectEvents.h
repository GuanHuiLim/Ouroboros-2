/************************************************************************************//*!
\file          LoadProjectEvents.h
\project       Editor
\author        Leong Jun Xiang, junxiang.leong , 390007920 | code contribution 100%
\par           email: junxiang.leong\@digipen.edu
\date          September 26, 2022
\brief         Event triggered when loading a project.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include <vector>
#include <string>

#include "Ouroboros/Scene/SceneInfo.h"
//#include <Ouroboros/Scene/RuntimeController.h>

class LoadProjectEvent :public oo::Event
{
public:
	LoadProjectEvent(std::string&& startscene, std::vector<oo::SceneInfo> && list_filename_pathname, std::string&& projectPath)
		:m_startScene{ startscene }, m_filename_pathname{ list_filename_pathname }, m_projectPath{ projectPath }{};
	~LoadProjectEvent() {};
	std::string m_projectPath;
	std::string m_startScene;
	std::vector<oo::SceneInfo> m_filename_pathname;
	//std::vector<std::pair<std::string, std::string>> m_filename_pathname;
private:
	
};

class CloseProjectEvent :public oo::Event
{
public:
	CloseProjectEvent() {};
	~CloseProjectEvent() {};
private:

};
