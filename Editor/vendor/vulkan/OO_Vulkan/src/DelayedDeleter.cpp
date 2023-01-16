/************************************************************************************//*!
\file           DelayedDeleter.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines Delayed deleter class to defer deleting gpu buffers to frames which they are not used

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "DelayedDeleter.h"
#include <cassert>
#include <algorithm>

DelayedDeleter::DelayedDeleter()
{
    for (size_t i = 0; i < MAX_FRAMES; i++)
    {
        m_countQueue.push_back(0);
    }
    m_isActive = true;
}

DelayedDeleter* DelayedDeleter::get()
{
    if (s_deleter == nullptr)
    {
        s_deleter = new DelayedDeleter();
    }
    return s_deleter;
}

void DelayedDeleter::Shutdown()
{
    auto* p = get();
    p->Clear();
    delete p;
}

void DelayedDeleter::DeleteAfterFrames(std::function<void()> fn)
{
    m_itemsThisFrame++;
    m_funcQueue.push_back(fn);
}

//void DelayedDeleter::DeleteAfterSeconds(std::function<void()> fn, float seconds)
//{
//    // TODO
//}

void DelayedDeleter::Update(float deltaTime)
{
    m_countQueue.emplace_back(m_itemsThisFrame);
    m_itemsThisFrame = 0;

    auto funcsToInvoke = m_countQueue.front();
    m_countQueue.pop_front();
     
    for (size_t i = 0; i < funcsToInvoke; i++)
    {
        assert(m_funcQueue.size()); // paranoia
        m_funcQueue.front()();
        m_funcQueue.pop_front();
    }
}

void DelayedDeleter::Clear()
{
    while (m_funcQueue.size())
    {
        m_funcQueue.front()();
        m_funcQueue.pop_front();
    }
}


