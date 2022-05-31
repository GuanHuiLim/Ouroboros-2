/************************************************************************************//*!
\file           EventManager.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           November 3, 2021
\brief          

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Ouroboros/EventSystem/EventSystem.h"
namespace oo
{
	class EventManager : public EventSystem
	{
		static EventManager* GetInstance();
		static EventManager* instance;

		EventSystem m_eventSystem;
	public:
        using FunctionContainer = EventSystem::FunctionContainer;
        using SubscriberContainer = EventSystem::SubscriberContainer;
		EventManager();
		~EventManager();

        template<typename EventType>
        static void Broadcast(EventType* event)
        {
            GetInstance()->m_eventSystem.Broadcast<EventType>(event);
        }
        /*********************************************************************************//*!
        \brief Registers a member function as a callback
        \param instance instance to object on which to invoke the member function on
        \param memberFunction pointer to member function
        *//**********************************************************************************/
        template<class T, class EventType>
        static void Subscribe(T* instance, typename EventMemberFunction<T, EventType>::MemberFunctionPointer memberFunction)
        {
            GetInstance()->m_eventSystem.Subscribe<T,EventType>(instance, memberFunction);
        }
        /*********************************************************************************//*!
        \brief Registers a static/non-member function as a callback
        \param function pointer to static/non-member function
        *//**********************************************************************************/
        template<class EventType>
        static void Subscribe(typename EventFunction<EventType>::FunctionPointer function)
        {
            GetInstance()->m_eventSystem.Subscribe<EventType>(function);
        }
        /*********************************************************************************//*!
        \brief Removes a registered member function aka callback
        \param instance instance to object on which to invoke the member function on
        \param memberFunction pointer to member function
        *//**********************************************************************************/
        template<class T, class EventType>
        static void Unsubscribe(T* instance, typename EventMemberFunction<T, EventType>::MemberFunctionPointer memberFunction)
        {
            GetInstance()->m_eventSystem.Unsubscribe<T, EventType>(instance, memberFunction);
        }
        /*********************************************************************************//*!
        \brief Removes a static/non-member function aka callback
        \param function pointer to static/non-member function
        *//**********************************************************************************/
        template<class EventType>
        static void Unsubscribe(typename EventFunction<EventType>::FunctionPointer function)
        {
            GetInstance()->m_eventSystem.Unsubscribe<EventType>(function);
        }
	};

}