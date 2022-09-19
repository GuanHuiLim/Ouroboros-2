#include "pch.h"
#include "InputManager.h"
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

			  value("Enter Key", input::KeyCode::ENTER),
			  value("Escape Key", input::KeyCode::ESCAPE),
			  value("Backspace", input::KeyCode::BACKSPACE),
			  value("TAB key", input::KeyCode::TAB),
			  value("Space key", input::KeyCode::SPACE),

			  value("Right Arrow", input::KeyCode::RIGHT),
			  value("Left Arrow", input::KeyCode::LEFT),
			  value("Down Arrow", input::KeyCode::DOWN),
			  value("Up Arrow", input::KeyCode::UP)
		);		 
	}
    void InputManager::LoadDefault()
    {
        axes.clear();
        axes.emplace("Mouse X",
            InputAxis
            {
                "Mouse X",
                InputAxis::InputType::MouseMovement,
                InputAxis::INPUTCODE_INVALID,
                InputAxis::INPUTCODE_INVALID,
                InputAxis::INPUTCODE_INVALID,
                InputAxis::INPUTCODE_INVALID,
                0U, 0.0f, 0.0f
            }
        );
        axes.emplace("Mouse Y",
            InputAxis
            {
                "Mouse Y",
                InputAxis::InputType::MouseMovement,
                InputAxis::INPUTCODE_INVALID,
                InputAxis::INPUTCODE_INVALID,
                InputAxis::INPUTCODE_INVALID,
                InputAxis::INPUTCODE_INVALID,
                0U, 0.0f, 0.0f
            }
        );
        axes.emplace("Horizontal",
            InputAxis
            {
                "Horizontal",
                InputAxis::InputType::KeyboardButton,
                static_cast<InputAxis::InputCode>(input::KeyCode::LEFT),
                static_cast<InputAxis::InputCode>(input::KeyCode::RIGHT),
                static_cast<InputAxis::InputCode>(input::KeyCode::A),
                static_cast<InputAxis::InputCode>(input::KeyCode::D),
                0U, 0.0f, 0.0f
            }
        );
        axes.emplace("Vertical",
            InputAxis
            {
                "Vertical",
                InputAxis::InputType::KeyboardButton,
                static_cast<InputAxis::InputCode>(input::KeyCode::DOWN),
                static_cast<InputAxis::InputCode>(input::KeyCode::UP),
                static_cast<InputAxis::InputCode>(input::KeyCode::S),
                static_cast<InputAxis::InputCode>(input::KeyCode::W),
                0U, 0.0f, 0.0f
            }
        );
    }
}