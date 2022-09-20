/************************************************************************************//*!
\file           ApplicationEvent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 15, 2022
\brief          Implements an event related to the window. Used to extract info from
                the event such as what is the screen resized to etc.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Core/Events/AppEvent.h"

namespace oo
{
    /********************************************************************************//*!
     @brief     Implements a Window Resize AppEvent
    *//*********************************************************************************/
    class WindowResizeEvent final : public AppEvent
    {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_width{ width }, m_height{ height } {}

        unsigned int GetWidth() const { return m_width; }
        unsigned int GetHeight() const { return m_height; }

        std::string ToString() const override final
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_width << ", " << m_height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WINDOWRESIZE)
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::APPLICATION })

    private:
        unsigned int m_width, m_height;
    };

    /********************************************************************************//*!
     @brief     Implements a Window Close AppEvent
    *//*********************************************************************************/
    class WindowCloseEvent final : public AppEvent
    {
    public:
        WindowCloseEvent() = default;
        EVENT_CLASS_TYPE(WINDOWCLOSE)
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::APPLICATION })
    };


    /********************************************************************************//*!
     @brief     Implements a Window Focus AppEvent
    *//*********************************************************************************/
    class WindowFocusEvent final : public AppEvent
    {
    public:
        WindowFocusEvent() = default;
        EVENT_CLASS_TYPE(WINDOWFOCUS)
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::APPLICATION })
    };

    /********************************************************************************//*!
     @brief     Implements a Window Lose Focus AppEvent
    *//*********************************************************************************/
    class WindowLoseFocusEvent final : public AppEvent
    {
    public:
        WindowLoseFocusEvent() = default;
        EVENT_CLASS_TYPE(WINDOWLOSEFOCUS)
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::APPLICATION })
    };

    class WindowMovedEvent final : public AppEvent
    {
    public:
        WindowMovedEvent() = default;
        EVENT_CLASS_TYPE(WINDOWMOVED)
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::APPLICATION })
    };



}