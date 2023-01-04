/************************************************************************************//*!
\file           InputManager.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 26, 2022
\brief          Defines the manager responsible for handling all possible input axes
                available to the player

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "InputManager.h"

#include <Ouroboros/Core/Input.h>

#include <rttr/registration>
namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
	  registration::enumeration<InputAxis::InputType>("Input Type")
		(
			value("Mouse Movement",InputAxis::InputType::MouseMovement),
			value("Mouse Button", InputAxis::InputType::MouseButton),
			value("Keyboard Button", InputAxis::InputType::KeyboardButton)
		);
      registration::enumeration<InputAxis::ControllerInputType>("Controller Input Type")
          (
              value("Trigger/Joystick", InputAxis::ControllerInputType::Trigger_Joystick),
              value("Button", InputAxis::ControllerInputType::Button)
          );
	  registration::enumeration<input::KeyCode>("KeyCode")
		  (
			  value("Key A", input::KeyCode::A),
			  value("Key B", input::KeyCode::B),
			  value("Key C", input::KeyCode::C),
			  value("Key D", input::KeyCode::D),
			  value("Key E", input::KeyCode::E),
			  value("Key F", input::KeyCode::F),
			  value("Key G", input::KeyCode::G),
			  value("Key H", input::KeyCode::H),
			  value("Key I", input::KeyCode::I),
			  value("Key J", input::KeyCode::J),
			  value("Key K", input::KeyCode::K),
			  value("Key L", input::KeyCode::L),
			  value("Key M", input::KeyCode::M),
			  value("Key N", input::KeyCode::N),
			  value("Key O", input::KeyCode::O),
			  value("Key P", input::KeyCode::P),
			  value("Key Q", input::KeyCode::Q),
			  value("Key R", input::KeyCode::R),
			  value("Key S", input::KeyCode::S),
			  value("Key T", input::KeyCode::T),
			  value("Key U", input::KeyCode::U),
			  value("Key V", input::KeyCode::V),
			  value("Key W", input::KeyCode::W),
			  value("Key X", input::KeyCode::X),
			  value("Key Y", input::KeyCode::Y),
			  value("Key Z", input::KeyCode::Z),

              value("Key 1", input::KeyCode::D1),
              value("Key 2", input::KeyCode::D2),
              value("Key 3", input::KeyCode::D3),
              value("Key 4", input::KeyCode::D4),
              value("Key 5", input::KeyCode::D5),
              value("Key 6", input::KeyCode::D6),
              value("Key 7", input::KeyCode::D7),
              value("Key 8", input::KeyCode::D8),
              value("Key 9", input::KeyCode::D9),

			  value("Enter Key", input::KeyCode::ENTER),
			  value("Escape Key", input::KeyCode::ESCAPE),
			  value("Backspace", input::KeyCode::BACKSPACE),
			  value("TAB key", input::KeyCode::TAB),
			  value("Space key", input::KeyCode::SPACE),

              value("Left Control key", input::KeyCode::LCTRL),
              value("Right Control key", input::KeyCode::RCTRL),
              value("Left Shift key", input::KeyCode::LSHIFT),
              value("Right Shift key", input::KeyCode::RSHIFT),
              value("Left Alt key", input::KeyCode::LALT),
              value("Right Alt key", input::KeyCode::RALT),

			  value("Right Arrow", input::KeyCode::RIGHT),
			  value("Left Arrow", input::KeyCode::LEFT),
			  value("Down Arrow", input::KeyCode::DOWN),
			  value("Up Arrow", input::KeyCode::UP)
		);		 
	registration::enumeration<input::MouseCode>("MouseCode")
		(
			value("Left Click", input::MouseCode::Button0),
			value("Right Click", input::MouseCode::Button1),
			value("Middle Click", input::MouseCode::Button2)
		);
    registration::enumeration<input::ControllerButtonCode>("ControllerButtonCode")
        (
            value("A", input::ControllerButtonCode::A),
            value("B", input::ControllerButtonCode::B),
            value("X", input::ControllerButtonCode::X),
            value("Y", input::ControllerButtonCode::Y),
            value("Back", input::ControllerButtonCode::BACK),
            value("Guide", input::ControllerButtonCode::GUIDE),
            value("Start", input::ControllerButtonCode::START),
            value("Left Stick", input::ControllerButtonCode::LEFTSTICK),
            value("Right Stick", input::ControllerButtonCode::RIGHTSTICK),
            value("Left Shoulder", input::ControllerButtonCode::LEFTSHOULDER),
            value("Right Shoulder", input::ControllerButtonCode::RIGHTSHOULDER),
            value("D-Pad Up", input::ControllerButtonCode::DPAD_UP),
            value("D-Pad Down", input::ControllerButtonCode::DPAD_DOWN),
            value("D-Pad Left", input::ControllerButtonCode::DPAD_LEFT),
            value("D-Pad Right", input::ControllerButtonCode::DPAD_RIGHT)
        );
    registration::enumeration<input::ControllerAxisCode>("ControllerAxisCode")
        (
            value("Left Joystick X", input::ControllerAxisCode::LEFTX),
            value("Left Joystick Y", input::ControllerAxisCode::LEFTY),
            value("Right Joystick X", input::ControllerAxisCode::RIGHTX),
            value("Right Joystick Y", input::ControllerAxisCode::RIGHTY),
            value("Left Trigger", input::ControllerAxisCode::TRIGGERLEFT),
            value("Right Trigger", input::ControllerAxisCode::TRIGGERRIGHT)
        );
	}
    void InputManager::LoadDefault()
    {
        axes.clear();
        axes.emplace_back(
            InputAxis
            {
                "Mouse X",
                InputAxis::InputType::MouseMovement,
                InputAxis::Settings
                {
                },
                InputAxis::ControllerInputType::Trigger_Joystick,
                InputAxis::Settings
                {
                }
            }
        );
        axes.emplace_back(
            InputAxis
            {
                "Mouse Y",
                InputAxis::InputType::MouseMovement,
                InputAxis::Settings
                {
                },
                InputAxis::ControllerInputType::Trigger_Joystick,
                InputAxis::Settings
                {
                }
            }
        );
        axes.emplace_back(
            InputAxis
            {
                "Horizontal",
                InputAxis::InputType::KeyboardButton,
                InputAxis::Settings
                {
                    static_cast<InputAxis::InputCode>(input::KeyCode::RIGHT),
                    static_cast<InputAxis::InputCode>(input::KeyCode::LEFT),
                    static_cast<InputAxis::InputCode>(input::KeyCode::D),
                    static_cast<InputAxis::InputCode>(input::KeyCode::A),
                    0U, 0.0f, 0.0f,
                    false, false
                },
                InputAxis::ControllerInputType::Trigger_Joystick,
                InputAxis::Settings
                {
                    InputAxis::INPUTCODE_INVALID,
                    static_cast<InputAxis::InputCode>(input::ControllerAxisCode::LEFTX),
                    InputAxis::INPUTCODE_INVALID,
                    InputAxis::INPUTCODE_INVALID,
                    0U, 0.0f, 0.0f,
                    false, false
                }
            }
        );
        axes.emplace_back(
            InputAxis
            {
                "Vertical",
                InputAxis::InputType::KeyboardButton,
                InputAxis::Settings
                {
                    static_cast<InputAxis::InputCode>(input::KeyCode::DOWN),
                    static_cast<InputAxis::InputCode>(input::KeyCode::UP),
                    static_cast<InputAxis::InputCode>(input::KeyCode::S),
                    static_cast<InputAxis::InputCode>(input::KeyCode::W),
                    0U, 0.0f, 0.0f,
                    false, false
                },
                InputAxis::ControllerInputType::Trigger_Joystick,
                InputAxis::Settings
                {
                    InputAxis::INPUTCODE_INVALID,
                    static_cast<InputAxis::InputCode>(input::ControllerAxisCode::LEFTY),
                    InputAxis::INPUTCODE_INVALID,
                    InputAxis::INPUTCODE_INVALID,
                    0U, 0.0f, 0.0f,
                    false, false
                }
            }
        );
        axes.emplace_back(
            InputAxis
            {
                "Fire",
                InputAxis::InputType::MouseButton,
                InputAxis::Settings
                {
                    InputAxis::INPUTCODE_INVALID,
                    static_cast<InputAxis::InputCode>(input::MouseCode::Button0),
                    InputAxis::INPUTCODE_INVALID,
                    InputAxis::INPUTCODE_INVALID,
                    0U, 0.0f, 0.0f,
                    false, false
                },
                InputAxis::ControllerInputType::Trigger_Joystick,
                InputAxis::Settings
                {
                    InputAxis::INPUTCODE_INVALID,
                    static_cast<InputAxis::InputCode>(input::ControllerAxisCode::TRIGGERRIGHT),
                    InputAxis::INPUTCODE_INVALID,
                    InputAxis::INPUTCODE_INVALID,
                    0U, 0.0f, 0.0f,
                    false, false
                }
            }
        );
    }
}