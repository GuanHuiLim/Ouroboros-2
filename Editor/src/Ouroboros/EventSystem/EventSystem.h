/************************************************************************************//*!
\file           EventSystem.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           October 31, 2022
\brief          Generic event system implemented with some reference from
                https://www.gamedev.net/articles/programming/general-and-gameplay-programming/effective-event-handling-in-c-r2459/
                and https://medium.com/@savas/nomad-game-engine-part-7-the-event-system-45a809ccb68f

                Events in this system are callback functions that take in a pointer
                to an Event object which functions as the information for the event.

                The events work such that callbacks using member functions or functions
                are added to an EventSystem object by calling Subscribe like so:

                For Static/non-member functions:
                ...
                eventSystem->Subscribe<Event>(&Dosomething);
                ...

                For member functions:
                ...
                void init()
                {
                    eventSystem->Subscribe(this, &CollisionSystem::onCollisionEvent);
                }
                ...

                *Note that functions subscribed should follow the following signature:
                * void DoSomething(Event* event);
                * however the type Event should be your own custom class/struct that 
                * derives from the Event type in oo::event::Event


                These callbacks aka events, are then triggered when calling Broadcast on
                this EventSystem object and the Event passed in matches an Event's type
                among the subscribed callbacks like so:
                //Physics system
                ...
                if (hasCollision(objectA, objectB)
                {
                    CollisionEvent evnt{objectA, objectB};
                    eventSystem->Broadcast(&evnt);
                }
                ...
                *Note that in this scenario, an event object was created on the stack
                * as a local variable and passed in. You may optionally choose to create
                * this object on the heap but you must be responsible for the event object's
                * lifetime to ensure no memory leaks/corruption.

                There may be situations where a return of information from the callbacks
                invoked may be desired. By using the Event object, you may store a reference
                or pointer to a struct/container and output your data there when the subscribed
                callbacks access the event object like so:
                ...
                if (isKeyDown(Key::K) == true)
                {
                    OutputInfo info;
                    KeydownEvent evnt{Key::K, &info};
                    eventSystem->Broadcast(&evnt);
                    //Do something with output info received
                    ...
                }
                ...

                *Note that the same thing applied in this scenario where objects(OutputInfo and KeydownEvent)
                * are created on the stack and the *note above applies here as well.
                * Depending on the usage, there are many ways you can use this fact to pass around information

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "Event.h"
#include "EventFunction.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
namespace oo
{
    class EventSystem 
    {
    public:
        using FunctionContainer = std::vector<std::unique_ptr<EventFunctionBase>>;
        using SubscriberContainer = std::unordered_map<std::type_index, FunctionContainer>;
    protected:
        SubscriberContainer m_subscribers;
    private:
        template<typename T>
        inline std::type_index GetTypeIndex()
        {
            return typeid(T);
        }

        template<typename T>
        SubscriberContainer::mapped_type& GetFunctionContainer()
        {
            if (m_subscribers.find(GetTypeIndex<T>()) == m_subscribers.end())
                m_subscribers.emplace(GetTypeIndex<T>(), SubscriberContainer::mapped_type{});

            return m_subscribers[GetTypeIndex<T>()];
        }

        template<typename T>
        void RemoveFunctionContainer()
        {
            if (m_subscribers.find(GetTypeIndex<T>()) == m_subscribers.end())
                return;
            m_subscribers.erase(m_subscribers.find(GetTypeIndex<T>()));
        }
    public:
        EventSystem() = default;
        EventSystem(EventSystem const&) = delete;
        EventSystem& operator=(EventSystem const&) = delete;


        template<typename EventType>
        void Broadcast(EventType* event) 
        {
            auto& function_container = GetFunctionContainer<EventType>();

            if (function_container.empty()) return;

            for (auto& functions : function_container) 
                functions->Execute(event);
        }
        /*********************************************************************************//*!
        \brief Registers a member function as a callback
        \param instance instance to object on which to invoke the member function on
        \param memberFunction pointer to member function
        *//**********************************************************************************/
        template<class T, class EventType>
        void Subscribe(T* instance, typename EventMemberFunction<T, EventType>::MemberFunctionPointer memberFunction)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                m_subscribers.emplace(GetTypeIndex<EventType>(), SubscriberContainer::mapped_type{});

            auto& function_container = GetFunctionContainer<EventType>();
            function_container.emplace_back(std::make_unique<EventMemberFunction<T, EventType>>(instance, memberFunction));
        }
        /*********************************************************************************//*!
        \brief Registers a static/non-member function as a callback
        \param function pointer to static/non-member function
        *//**********************************************************************************/
        template<class EventType>
        void Subscribe(typename EventFunction<EventType>::FunctionPointer function)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                m_subscribers.emplace(GetTypeIndex<EventType>(), SubscriberContainer::mapped_type{});

            auto& function_container = GetFunctionContainer<EventType>();
            function_container.emplace_back(std::make_unique<EventFunction<EventType>>(function));
        }
        /*********************************************************************************//*!
        \brief Removes a registered member function aka callback
        \param instance instance to object on which to invoke the member function on
        \param memberFunction pointer to member function
        *//**********************************************************************************/
        template<class T, class EventType>
        void Unsubscribe(T* instance, typename EventMemberFunction<T, EventType>::MemberFunctionPointer memberFunction)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                return;

            auto& function_container = GetFunctionContainer<EventType>();

            EventMemberFunction<T, EventType> callback{ instance, memberFunction };
            std::size_t index = 0;
            for (auto& function : function_container)
            {
                if (*(static_cast<EventMemberFunction<T, EventType>*>(function.get())) == callback)
                {
                    function_container.erase(function_container.begin() + index);
                    break;
                }
                ++index;
            }
            if (function_container.empty())
            {
                RemoveFunctionContainer<EventType>();
            }
        }
        /*********************************************************************************//*!
        \brief Removes a static/non-member function aka callback
        \param function pointer to static/non-member function
        *//**********************************************************************************/
        template<class EventType>
        void Unsubscribe(typename EventFunction<EventType>::FunctionPointer function)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                return;

            auto& function_container = GetFunctionContainer<EventType>();

            EventFunction<EventType> callback{ function };
            std::size_t index = 0;
            for (auto& each_function : function_container)
            {
                if (*(static_cast<EventFunction<EventType>*>(each_function.get())) == callback)
                {
                    function_container.erase(function_container.begin() + index);
                    break;
                }
                ++index;
            }
            if (function_container.empty())
            {
                RemoveFunctionContainer<EventType>();
            }
        }
    };
}
