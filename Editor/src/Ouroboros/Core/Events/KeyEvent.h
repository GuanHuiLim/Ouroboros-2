/************************************************************************************//*!
\file           KeyEvent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 15, 2022
\brief          Implements an event related to the keyboard. Used to extract info from
                the event such as what key was pressed and released.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Core/Events/AppEvent.h"
#include "Ouroboros/Core/KeyCode.h"

namespace oo
{
    /********************************************************************************//*!
     @brief     Implements a basic Key AppEvent. Stores the basic keycode
                and is categorised under a keyboard and input category
    *//*********************************************************************************/
    class KeyEvent : public AppEvent
    {
    public:
        inline input::KeyCode GetKeyCode() const { return m_keycode; }

        EVENT_CLASS_CATEGORY(utility::bitmask{ EVENT_CATEGORY::KEYBOARD } | EVENT_CATEGORY::INPUT)
    protected:
        KeyEvent(input::KeyCode keycode)
            : m_keycode{ keycode } {}

        input::KeyCode m_keycode;
    };

    /********************************************************************************//*!
     @brief     Implements the Key Pressed AppEvent. Stores the keycode and whether
                the key currently pressed is a repeatedly held down key.
    *//*********************************************************************************/
    class KeyPressedEvent final : public KeyEvent
    {
    public:
        KeyPressedEvent(const input::KeyCode keycode, int repeatCount)
            : KeyEvent{ keycode }, m_repeatCount{ repeatCount } {}

        inline int GetRepeatCount() const { return m_repeatCount; }

        std::string ToString() const override final
        {
            std::stringstream ss;
            ss << "KeyPressedEvent : " << m_keycode << "(" << m_repeatCount << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KEYPRESSED)
    private:
        int m_repeatCount;
    };

    /********************************************************************************//*!
     @brief     Implements the Key Released AppEvent.
    *//*********************************************************************************/
    class KeyReleasedEvent final : public KeyEvent
    {
    public:
        KeyReleasedEvent(const input::KeyCode keycode)
            : KeyEvent{ keycode } {}

        std::string ToString() const override final
        {
            std::stringstream ss;
            ss << "KeyReleasedEvent : " << m_keycode;
            return ss.str();
        }

        EVENT_CLASS_TYPE(KEYRELEASED)
    };

    /********************************************************************************//*!
     @brief     Implements the Key Typed AppEvent.
    *//*********************************************************************************/
    class KeyTypedEvent : public KeyEvent
    {
    public:
        KeyTypedEvent(const input::KeyCode keycode)
            : KeyEvent(keycode) {}

        std::string ToString() const override final
        {
            std::stringstream ss;
            ss << "KeyTypedEvent : " << m_keycode;
            return ss.str();
        }

        EVENT_CLASS_TYPE(KEYTYPED)
    };

}