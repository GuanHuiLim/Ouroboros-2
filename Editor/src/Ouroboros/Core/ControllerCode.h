/************************************************************************************//*!
\file           ControllerCode.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Nov 25, 2022
\brief          Defines the engine controller codes that are used the input system to map each
                controller input to a behaviour regardless of what the backend is using.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <ostream>

namespace input
{
    typedef enum class ControllerButtonCode : size_t
    {
        // TODO : Check if invalid is being useful here.
        //INVALID = -1,
        A,
        B,
        X,
        Y,
        BACK,
        GUIDE,
        START,
        LEFTSTICK,
        RIGHTSTICK,
        LEFTSHOULDER,
        RIGHTSHOULDER,
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button */
        PADDLE1,  /* Xbox Elite paddle P1 */
        PADDLE2,  /* Xbox Elite paddle P3 */
        PADDLE3,  /* Xbox Elite paddle P2 */
        PADDLE4,  /* Xbox Elite paddle P4 */
        TOUCHPAD, /* PS4/PS5 touchpad button */
        MAX
    } ControllerButton;

    inline std::ostream& operator<<(std::ostream& os, ControllerButton controllerCode)
    {
        os << static_cast<int>(controllerCode);
        return os;
    }

    inline ControllerButton& operator++(ControllerButton& controllerCode)
    {
        controllerCode = static_cast<ControllerButton>(static_cast<int>(controllerCode) + 1);
        return controllerCode;
    }

    inline bool operator<(ControllerButton controllerCode, int val)
    {
        return static_cast<int>(controllerCode) < val;
    }

    typedef enum class ControllerAxisCode : size_t
    {
        // TODO : Check if invalid is being useful here.
        //INVALID = -1,
        LEFTX,
        LEFTY,
        RIGHTX,
        RIGHTY,
        TRIGGERLEFT,
        TRIGGERRIGHT,
        MAX
    } ControllerAxis;

    inline std::ostream& operator<<(std::ostream& os, ControllerAxis controllerCode)
    {
        os << static_cast<int>(controllerCode);
        return os;
    }

    inline ControllerAxis& operator++(ControllerAxis& controllerCode)
    {
        controllerCode = static_cast<ControllerAxis>(static_cast<int>(controllerCode) + 1);
        return controllerCode;
    }

    inline bool operator<(ControllerAxis controllerCode, int val)
    {
        return static_cast<int>(controllerCode) < val;
    }
}

// Controller Button codes 
#define CONTROLLER_BUTTON_A          ::input::ControllerButtonCode::A
#define CONTROLLER_BUTTON_B          ::input::ControllerButtonCode::B
#define CONTROLLER_BUTTON_X          ::input::ControllerButtonCode::X
#define CONTROLLER_BUTTON_Y          ::input::ControllerButtonCode::Y
#define CONTROLLER_BUTTON_BACK       ::input::ControllerButtonCode::BACK
#define CONTROLLER_BUTTON_OPTION     ::input::ControllerButtonCode::GUIDE
#define CONTROLLER_BUTTON_START      ::input::ControllerButtonCode::START
#define CONTROLLER_BUTTON_L3         ::input::ControllerButtonCode::LEFTSTICK
#define CONTROLLER_BUTTON_R3         ::input::ControllerButtonCode::RIGHTSTICK
#define CONTROLLER_BUTTON_L1         ::input::ControllerButtonCode::LEFTSHOULDER
#define CONTROLLER_BUTTON_R1         ::input::ControllerButtonCode::RIGHTSHOULDER
#define CONTROLLER_BUTTON_UP         ::input::ControllerButtonCode::DPAD_UP
#define CONTROLLER_BUTTON_DOWN       ::input::ControllerButtonCode::DPAD_DOWN
#define CONTROLLER_BUTTON_LEFT       ::input::ControllerButtonCode::DPAD_LEFT
#define CONTROLLER_BUTTON_RIGHT      ::input::ControllerButtonCode::DPAD_RIGHT

// Controller Axis codes 
#define CONTROLLER_BUTTON_LEFT_X     ::input::ControllerAxisCode::LEFTX
#define CONTROLLER_BUTTON_LEFT_Y     ::input::ControllerAxisCode::LEFTY
#define CONTROLLER_BUTTON_RIGHT_X    ::input::ControllerAxisCode::RIGHTX
#define CONTROLLER_BUTTON_RIGHT_Y    ::input::ControllerAxisCode::RIGHTY
#define CONTROLLER_BUTTON_TRIGGER_X  ::input::ControllerAxisCode::TRIGGERLEFT
#define CONTROLLER_BUTTON_TRIGGER_Y  ::input::ControllerAxisCode::TRIGGERRIGHT

