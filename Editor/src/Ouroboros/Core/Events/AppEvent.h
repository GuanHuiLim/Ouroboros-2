/************************************************************************************//*!
\file           AppEvent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 15, 2022
\brief          Describes the various units that describes an events, from
                enum to determine an events type and category to what a basic event is
                and what a dispatcher does.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/EventSystem/Event.h"
#include "Utility/Bitmask.h"
#include <sstream>

namespace oo
{
    /********************************************************************************//*!
     @brief     Describes the different types of events that the engine caters for.
    *//*********************************************************************************/
    enum class EVENT_TYPE : int
    {
        NONE = 0,
        WINDOWCLOSE, WINDOWRESIZE, WINDOWFOCUS, WINDOWLOSEFOCUS, WINDOWMOVED, WINDOWMAXIMIZE, WINDOWMINIMIZE, WINDOWRESTORED,
        KEYPRESSED, KEYRELEASED, KEYTYPED,
        MOUSEBUTTONPRESSED, MOUSEBUTTONRELEASED, MOUSEMOVED, MOUSESCROLLED,
        CONTROLLERPRESSED, CONTROLLERRELEASED, CONTROLLERMOVED,
        CONTROLLERADDED, CONTROLLERREMOVED,
        FILEDROPPED,
    };

    /********************************************************************************//*!
     @brief     Describes the various categories used to classify each event type.

     @note      An event can have more then 1 category. Used to simplify when looking
                for inputs
    *//*********************************************************************************/
    enum class EVENT_CATEGORY : int
    {
        NONE,
        APPLICATION,
        INPUT,
        KEYBOARD,
        MOUSE,
        MOUSEBUTTON,
        CONTROLLER,
        DEVICE,
        FILEDROP,
    };

    /********************************************************************************//*!
     @brief     typesafe enum that still has normal enum operations.
    *//*********************************************************************************/
    using EventCategoryType = bitmask<EVENT_CATEGORY>;

    #define EVENT_CLASS_CATEGORY(category) \
    virtual EventCategoryType GetCategoryFlag() const override final { return category; }

    #define EVENT_CLASS_TYPE(type) \
    static EVENT_TYPE GetStaticType() { return EVENT_TYPE::##type; }\
    virtual EVENT_TYPE GetEventType() const override final { return GetStaticType(); }\
    virtual const char* GetName() const override final { return #type; }

    /********************************************************************************//*!
     @brief     Describes a basic event.
    *//*********************************************************************************/
    class AppEvent : public Event
    {
    public:
        //bool Handled = false;

        virtual EVENT_TYPE GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual EventCategoryType GetCategoryFlag() const = 0;
        virtual std::string ToString() const { return GetName(); }

        inline bool IsInCategory(EVENT_CATEGORY categories)
        {
            return GetCategoryFlag() & categories;
        }

    };

    /********************************************************************************//*!
     @brief     Ostream operator overload for outputting events
    *//*********************************************************************************/
    inline std::ostream& operator<<(std::ostream& os, const AppEvent& e)
    {
        return os << e.ToString();
    }

}