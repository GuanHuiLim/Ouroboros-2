/************************************************************************************//*!
\file           EventFunction.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 2000552 | code contribution (100%)
\par            email: l.guanhui\@digipen.edu
\date           October 31, 2021
\brief          Contains Event function classes that are used by the event system

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "EventUtils.h"
#include <functional>
namespace oo
{
    class EventFunctionBase 
    {
    public:
        // Call the member function
        void Execute(event::Event* event);
        virtual ~EventFunctionBase() = default;
    protected:
        // Implemented by MemberFunctionHandler
        virtual void Invoke(event::Event* event) = 0;
    };


    template<class T, class EventType>
    class EventMemberFunction : public EventFunctionBase
    {
    public:
        using MemberFunctionPointer = void(T::*)(EventType*);

        EventMemberFunction(T* instance, MemberFunctionPointer memberFunction) : m_instance{ instance }, m_memberFunction{ memberFunction } {};

    protected:
        void Invoke(event::Event* event) override
        {
            // Cast event to the correct type and call member function
            std::invoke(m_memberFunction, m_instance, static_cast<EventType*>(event));
        }

        // Pointer to class instance
        T* m_instance;

        // Pointer to member function
        MemberFunctionPointer m_memberFunction;
    };

    template<class EventType>
    class EventFunction : public EventFunctionBase
    {
    public:
        using FunctionPointer = void(*)(EventType*);
        EventFunction(FunctionPointer function) : m_function{ function } {};
    private:
        void Invoke(event::Event* event) override 
        {
            // Cast event to the correct type and call function
            std::invoke(m_function, static_cast<EventType*>(event));
        }
        // Pointer to member function
        FunctionPointer m_function;
    };
}
