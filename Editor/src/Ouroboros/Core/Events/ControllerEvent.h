/************************************************************************************//*!
\file           ControllerEvent.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Nov 25, 2022
\brief          Implements an event related to the controller. Used to extract info from
                the event such as what controller button was pressed and released.

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
     @brief     Implements a Controller Added AppEvent. AppEvent that will be created when
                a new controller is detected to be added.
    *//*********************************************************************************/
    class ControllerAddedEvent : public AppEvent
    {
    public:
        EVENT_CLASS_CATEGORY(utility::bitmask{ EVENT_CATEGORY::DEVICE } | EVENT_CATEGORY::CONTROLLER | EVENT_CATEGORY::INPUT)

        ControllerAddedEvent(int32_t index) : m_index{ index } {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "ControllerAddedEvent: " << m_index;
            return ss.str();
        }

        EVENT_CLASS_TYPE(CONTROLLERADDED);

    private:
        int32_t m_index;
    };

    /********************************************************************************//*
     @brief     Implements a Controller Removed AppEvent. AppEvent that will be created when
                a controller is removed to be added.
    *//*********************************************************************************/
    class ControllerRemovedEvent : public AppEvent
    {
    public:
        EVENT_CLASS_CATEGORY(utility::bitmask{ EVENT_CATEGORY::DEVICE } | EVENT_CATEGORY::CONTROLLER | EVENT_CATEGORY::INPUT)

        ControllerRemovedEvent(int32_t index) : m_index{ index } {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "ControllerAddedEvent: " << m_index;
            return ss.str();
        }

        EVENT_CLASS_TYPE(CONTROLLERADDED);

    private:
        int32_t m_index;
    };

    /********************************************************************************//*!
     @brief     Implements a basic Key AppEvent. Stores the basic keycode
                and is categorised under a keyboard and input category
    *//*********************************************************************************/
    class ControllerMovedEvent : public AppEvent
    {
    public:
        EVENT_CLASS_CATEGORY(utility::bitmask{ EVENT_CATEGORY::CONTROLLER } | EVENT_CATEGORY::INPUT)

        ControllerMovedEvent(uint8_t button, int16_t value) : m_button{ button }, m_value{ value } {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "ControllerMovedEvent: " << m_button << " of value " << m_value;
            return ss.str();
        }

        EVENT_CLASS_TYPE(CONTROLLERMOVED);

    private:
        uint8_t m_button;
        int16_t m_value;
    };

    /********************************************************************************//*!
     @brief     Implements a basic Key AppEvent. Stores the basic keycode
                and is categorised under a keyboard and input category
    *//*********************************************************************************/
    class ControllerButtonEvent : public AppEvent
    {
    public:
        EVENT_CLASS_CATEGORY(utility::bitmask{ EVENT_CATEGORY::CONTROLLER } | EVENT_CATEGORY::INPUT)
    protected:
        ControllerButtonEvent(uint8_t button) : m_button{ button } {}

        uint8_t m_button;
    };

    /********************************************************************************//*!
     @brief     Implements the Key Pressed AppEvent. Stores the keycode and whether
                the key currently pressed is a repeatedly held down key.
    *//*********************************************************************************/
    class ControllerPressedEvent : public ControllerButtonEvent
    {
    public:
        ControllerPressedEvent(uint8_t button) : ControllerButtonEvent{ button } {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "ControllerPressedEvent: " << m_button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(CONTROLLERPRESSED);
    };

    /********************************************************************************//*!
     @brief     Implements the Key Released AppEvent.
    *//*********************************************************************************/
    class ControllerReleasedEvent : public ControllerButtonEvent
    {
    public:
        ControllerReleasedEvent(uint8_t button) : ControllerButtonEvent{ button } {}

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "ControllerReleasedEvent: " << m_button;
            return ss.str();
        }

        EVENT_CLASS_TYPE(CONTROLLERRELEASED);
    };


}