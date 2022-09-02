#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include <vector>
#include <string>

#include "Ouroboros/Scene/SceneInfo.h"
//#include <Ouroboros/Scene/RuntimeController.h>

class LoadProjectEvent :public oo::Event
{
public:
	LoadProjectEvent(std::string&& startscene, std::vector<oo::SceneInfo> && list_filename_pathname)
		:m_startScene{ startscene }, m_filename_pathname{ list_filename_pathname } {};
	~LoadProjectEvent() {};
	
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
