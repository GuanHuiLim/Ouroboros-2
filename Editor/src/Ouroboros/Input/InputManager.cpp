#include "pch.h"
#include "InputManager.h"

namespace oo
{
    void InputManager::LoadDefault()
    {
        axes.clear();
        axes.emplace_back(
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
        axes.emplace_back(
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
        axes.emplace_back(
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
        axes.emplace_back(
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