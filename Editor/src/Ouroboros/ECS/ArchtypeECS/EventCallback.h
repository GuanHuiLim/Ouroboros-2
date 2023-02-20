/************************************************************************************//*!
\file           EventCallback.h
\project        ECS
\author         Lim Guan Hui, l.guanhui, 2000552
\par            email: l.guanhui\@digipen.edu
\date           October 2, 2022
\brief          The ECS's internal callback event system

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "EventFunction.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <typeindex>
namespace Ecs
{
    class EventCallback
    {
    public:
        using FunctionContainer = std::vector<std::unique_ptr<internal::EventFunctionBase>>;
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
        EventCallback() = default;
        EventCallback(EventCallback const&) = delete;
        EventCallback& operator=(EventCallback const&) = delete;


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
        void Subscribe(T* instance, typename internal::EventMemberFunction<T, EventType>::MemberFunctionPointer memberFunction)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                m_subscribers.emplace(GetTypeIndex<EventType>(), SubscriberContainer::mapped_type{});

            auto& function_container = GetFunctionContainer<EventType>();
            function_container.emplace_back(std::make_unique<internal::EventMemberFunction<T, EventType>>(instance, memberFunction));
        }
        /*********************************************************************************//*!
        \brief Registers a static/non-member function as a callback
        \param function pointer to static/non-member function
        *//**********************************************************************************/
        template<class EventType>
        void Subscribe(typename internal::EventFunction<EventType>::FunctionPointer function)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                m_subscribers.emplace(GetTypeIndex<EventType>(), SubscriberContainer::mapped_type{});

            auto& function_container = GetFunctionContainer<EventType>();
            function_container.emplace_back(std::make_unique<internal::EventFunction<EventType>>(function));
        }
        /*********************************************************************************//*!
        \brief Removes a registered member function aka callback
        \param instance instance to object on which to invoke the member function on
        \param memberFunction pointer to member function
        *//**********************************************************************************/
        template<class T, class EventType>
        void Unsubscribe(T* instance, typename internal::EventMemberFunction<T, EventType>::MemberFunctionPointer memberFunction)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                return;

            auto& function_container = GetFunctionContainer<EventType>();

            internal::EventMemberFunction<T, EventType> callback{ instance, memberFunction };
            std::size_t index = 0;
            for (auto& function : function_container)
            {
                if ((*function) == callback)
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
        void Unsubscribe(typename internal::EventFunction<EventType>::FunctionPointer function)
        {
            if (m_subscribers.find(GetTypeIndex<EventType>()) == m_subscribers.end())
                return;

            auto& function_container = GetFunctionContainer<EventType>();

            internal::EventFunction<EventType> callback{ function };
            std::size_t index = 0;
            for (auto& function : function_container)
            {
                if ((*function) == callback)
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
