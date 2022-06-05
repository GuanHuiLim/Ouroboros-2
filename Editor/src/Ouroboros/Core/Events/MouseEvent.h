/************************************************************************************//*!
\file           MouseEvent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 15, 2022
\brief          Implements an event related to the mouse. Used to extract info from
                the event such as what mouse button was pressed and how much the mouse
                has scrolled.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Core/Events/AppEvent.h"
#include "Ouroboros/Core/MouseCode.h"

namespace oo
{
    /********************************************************************************//*!
     @brief     Implements a MOUSE Moved Event.
    *//*********************************************************************************/
    class MouseMovedEvent final : public AppEvent
    {
    public:
        MouseMovedEvent(const float x, const float y)
            : m_mouseX{ x }, m_mouseY{ y } {}

        float GetX() const { return m_mouseX; }
        float GetY() const { return m_mouseY; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MOUSEMOVED)
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::MOUSE } | EVENT_CATEGORY::INPUT)
    private:
        float m_mouseX, m_mouseY;
    };

    /********************************************************************************//*!
     @brief     Implements a MOUSE Scrolled Event.
    *//*********************************************************************************/
    class MouseScrolledEvent final : public AppEvent
    {
    public:
        MouseScrolledEvent(const float xOffset, const float yOffset)
            : m_xOffset{ xOffset }, m_yOffset{ yOffset } {}

        float GetX() const { return m_xOffset; }
        float GetY() const { return m_yOffset; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << m_xOffset << ", " << m_yOffset;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MOUSESCROLLED)
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::MOUSE } | EVENT_CATEGORY::INPUT)
    private:
        float m_xOffset, m_yOffset;
    };

    /********************************************************************************//*!
     @brief     Implements a MOUSE Button Event.
    *//*********************************************************************************/
    class MouseButtonEvent : public AppEvent
    {
    public:
        input::MouseCode GetMouseButton() const { return m_Button; }

        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::MOUSE } | EVENT_CATEGORY::INPUT | EVENT_CATEGORY::MOUSEBUTTON)
    protected:
        MouseButtonEvent(const input::MouseCode button)
            : m_Button(button) {}

        input::MouseCode m_Button;
    };

    /********************************************************************************//*!
     @brief     Implements a MOUSE Button Pressed Event.
    *//*********************************************************************************/
    class MouseButtonPressedEvent final : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(const input::MouseCode button)
            : MouseButtonEvent(button) {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << m_Button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MOUSEBUTTONPRESSED)
    };

    /********************************************************************************//*!
     @brief     Implements a MOUSE Button Released Event.
    *//*********************************************************************************/
    class MouseButtonReleasedEvent final : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(const input::MouseCode button)
            : MouseButtonEvent(button) {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << m_Button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MOUSEBUTTONRELEASED)
    };
}