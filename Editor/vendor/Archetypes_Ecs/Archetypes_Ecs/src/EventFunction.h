#pragma once

#include "EcsUtils.h"
#include <functional>
namespace Ecs::internal
{
class EventFunctionBase {
    public:
        // Call the member function
        inline void Execute(event::Event* event){ Invoke(event); }
        virtual ~EventFunctionBase() = default;
    protected:
        // Implemented by MemberFunctionHandler
        virtual void Invoke(event::Event* event) = 0;
    };


    template<class T, class EventType>
    class EventMemberFunction : public EventFunctionBase
    {
    public:
        using ValueType = EventType;
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
        using ValueType = EventType;
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
