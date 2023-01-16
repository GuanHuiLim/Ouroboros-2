/************************************************************************************//*!
\file           DelayedDeleter.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares Delayed deleter class to defer deleting gpu buffers to frames which they are not used

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include <functional>
#include <array>
#include <vector>
#include <deque>

class DelayedDeleter
{
public:
	DelayedDeleter();

	static DelayedDeleter* get();
	static void Shutdown();

	void DeleteAfterFrames(std::function<void()> fn);
	//void DeleteAfterSeconds(std::function<void()> fn, float seconds = 3);
	void Update(float deltaTime = 0.0f);
	void Clear();

private:
	static constexpr size_t MAX_FRAMES = 4;
	bool m_isActive = true;

	size_t m_itemsThisFrame{ 0 };
	std::array<std::vector<std::function<void()>>, MAX_FRAMES> m_funcs;
	
	std::deque< size_t > m_countQueue;
	std::deque< std::function<void()> > m_funcQueue;
	inline static DelayedDeleter* s_deleter{nullptr};
};

