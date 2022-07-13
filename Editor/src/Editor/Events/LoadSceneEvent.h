#pragma once
#include "Ouroboros/EventSystem/Event.h"
#include <string>
class LoadSceneEvent :public oo::Event
{
public:
	LoadSceneEvent(const std::string& scenePath) : m_scenePath{ scenePath } {};
	LoadSceneEvent(std::string&& scenePath) : m_scenePath{ scenePath } {};
	~LoadSceneEvent() {};
	std::string m_scenePath;
};

