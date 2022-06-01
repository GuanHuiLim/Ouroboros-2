/************************************************************************************//*!
\file           EventManager.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           November 3, 2022
\brief          

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "EventManager.h"
namespace oo
{
	EventManager* EventManager::instance = nullptr;

	EventManager* EventManager::GetInstance()
	{
		static EventManager g_instance;
		if (instance == nullptr)
		{
			//"EventManager instance is not created, please initialize it before use."
			throw std::bad_alloc{};
		}
		return instance;
	}
	EventManager::EventManager()
	{
		if (instance != nullptr)
		{
			throw std::bad_alloc{};
			return;
		}

		instance = this;

	}
	EventManager::~EventManager()
	{
	}
}