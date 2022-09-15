#include "pch.h"
#include "InputManager.h"

#include <Ouroboros/Core/Input.h>

namespace oo
{
    InputAxis::Tracker::Tracker(InputAxis const& axis) : axis{ axis }, durationHeld{ 0 }, pressCount{ 0 }, lastPressed{ InputAxis::INPUTCODE_INVALID }
    {

    }

    void InputAxis::Tracker::Update(float deltaTime)
    {
        //bool isPressed = false;;
        //bool isHeld = false;
        //switch (axis.type)
        //{
        //case InputType::KeyboardButton:
        //{

        //}
        //break;
        //}
    }

    bool InputAxis::Tracker::Satisfied()
    {
        return (axis.pressesRequired <= 0 || axis.pressesRequired >= pressCount)
            && (axis.holdDurationRequired <= 0.0f || axis.holdDurationRequired >= durationHeld);
    }

    float InputAxis::Tracker::GetValue()
    {
        if (!Satisfied())
            return 0.0f;
        float value = 0.0f;
        switch (axis.type)
        {
        case InputType::MouseMovement:
        {
            if (axis.GetName() == "Mouse X")
                value = input::GetMouseDelta().first;
            else if (axis.GetName() == "Mouse Y")
                value = input::GetMouseDelta().second;
        }
        break;
        case InputType::MouseButton:
        {
            if (IsMouseButtonHeld(axis.GetPositiveButton()) || IsMouseButtonHeld(axis.GetPositiveAltButton()))
                value = 1.0f;
            else if (IsMouseButtonHeld(axis.GetNegativeButton()) || IsMouseButtonHeld(axis.GetNegativeAltButton()))
                value = -1.0f;
        }
        break;
        case InputType::KeyboardButton:
        {
            if (IsKeyboardKeyHeld(axis.GetPositiveButton()) || IsKeyboardKeyHeld(axis.GetPositiveAltButton()))
                value = 1.0f;
            else if (IsKeyboardKeyHeld(axis.GetNegativeButton()) || IsKeyboardKeyHeld(axis.GetNegativeAltButton()))
                value = -1.0f;
        }
        break;
        }
        return value;
    }

    bool InputAxis::IsKeyboardKeyPressed(InputAxis::InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        input::KeyCode keyCode = static_cast<input::KeyCode>(inputCode);
        return input::IsKeyPressed(keyCode);
    }
    bool InputAxis::IsKeyboardKeyHeld(InputAxis::InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        input::KeyCode keyCode = static_cast<input::KeyCode>(inputCode);
        return input::IsKeyHeld(keyCode);
    }
    bool InputAxis::IsKeyboardKeyReleased(InputAxis::InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        input::KeyCode keyCode = static_cast<input::KeyCode>(inputCode);
        return input::IsKeyReleased(keyCode);
    }

    bool InputAxis::IsMouseButtonHeld(InputAxis::InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        input::MouseCode mouseCode = static_cast<input::MouseCode>(inputCode);
        return input::IsMouseButtonHeld(mouseCode);
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