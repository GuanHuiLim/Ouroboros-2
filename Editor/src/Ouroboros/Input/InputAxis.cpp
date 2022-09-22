#include "pch.h"
#include "InputAxis.h"

#include <Ouroboros/Core/Input.h>

namespace oo
{
    InputAxis::InputAxis()
        : name{ "Axis Name" }, type{ InputType::KeyboardButton }, settings{ }, controllerType{ ControllerInputType::Button }, controllerSettings{ }
    {
    }

    InputAxis::InputAxis(std::string name, InputType type, Settings const& settings, ControllerInputType controllerType, Settings const& controllerSettings)
        : name{ name }, type{ type }, settings{ settings }, controllerType{ controllerType }, controllerSettings{ controllerSettings }
    {
    }

    void InputAxis::SetType(InputType newType)
    {
        type = newType;
        switch (type)
        {
        default:
            settings.negativeButton = InputAxis::INPUTCODE_INVALID;
            settings.negativeAltButton = InputAxis::INPUTCODE_INVALID;
            settings.positiveButton = InputAxis::INPUTCODE_INVALID;
            settings.positiveAltButton = InputAxis::INPUTCODE_INVALID;
            settings.pressesRequired = 0;
            settings.maxGapTime = 0.0f;
            settings.holdDurationRequired = 0.0f;
            break;
        }
    }

    InputAxis::Tracker::Tracker(InputAxis const& axis)
        : axis{ axis }, durationHeld{ 0.0f }, pressCount{ 0 }, pressGapTimeLeft{ 0.0f }, lastPressed{ InputAxis::INPUTCODE_INVALID }
    {

    }

    void InputAxis::Tracker::Update(float deltaTime)
    {
        if (axis.type == InputType::MouseMovement)
            return;
        // update last pressed
        if (lastPressed != axis.settings.positiveButton && IsInputCodePressed(axis.type, axis.settings.positiveButton))
        {
            lastPressed = axis.settings.positiveButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
        else if (lastPressed != axis.settings.positiveAltButton && IsInputCodePressed(axis.type, axis.settings.positiveAltButton))
        {
            lastPressed = axis.settings.positiveAltButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
        else if (lastPressed != axis.settings.negativeButton && IsInputCodePressed(axis.type, axis.settings.negativeButton))
        {
            lastPressed = axis.settings.negativeButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
        else if (lastPressed != axis.settings.negativeAltButton && IsInputCodePressed(axis.type, axis.settings.negativeAltButton))
        {
            lastPressed = axis.settings.negativeAltButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }

        if (pressGapTimeLeft > 0.0f)
        {
            pressGapTimeLeft -= deltaTime;
            if (pressGapTimeLeft <= 0.0f)
                pressCount = 0.0f;
        }

        // update the rest of the stats
        if (IsInputCodePressed(axis.type, lastPressed))
        {
            ++pressCount;
            pressGapTimeLeft = axis.settings.maxGapTime;
        }
        if (IsInputCodeHeld(axis.type, lastPressed))
        {
            durationHeld += deltaTime;
        }
        if (IsInputCodeReleased(axis.type, lastPressed))
        {
            durationHeld = 0.0f;
        }
    }

    float InputAxis::Tracker::GetValue()
    {
        float value = 0.0f;
        if (axis.settings.ConditionsMet(pressCount, durationHeld))
        {
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
            default:
                {
                    if (IsInputCodeHeld(axis.type, axis.settings.positiveButton) || IsInputCodeHeld(axis.type, axis.settings.positiveAltButton))
                        value += 1.0f;
                    if (IsInputCodeHeld(axis.type, axis.settings.negativeButton) || IsInputCodeHeld(axis.type, axis.settings.negativeAltButton))
                        value += -1.0f;
                }
            }
        }
        if (axis.controllerSettings.ConditionsMet(pressCount, durationHeld))
        {
            float controllerValue = 0.0f;
            switch (axis.controllerType)
            {
            case ControllerInputType::Trigger_Joystick:
                {
                    float posValue = GetControllerAxisValue(axis.controllerSettings.positiveButton);
                    float temp = GetControllerAxisValue(axis.controllerSettings.positiveAltButton);
                    if (std::fabsf(temp) > std::fabsf(posValue))
                        posValue = temp;
                    float negValue = GetControllerAxisValue(axis.controllerSettings.negativeButton);
                    temp = GetControllerAxisValue(axis.controllerSettings.negativeAltButton);
                    if (std::fabsf(temp) > std::fabsf(negValue))
                        negValue = temp;
                    controllerValue = posValue - negValue;
                }
                break;
            case ControllerInputType::Button:
                {
                    if (IsControllerInputCodeHeld(axis.controllerSettings.positiveButton) || IsControllerInputCodeHeld(axis.controllerSettings.positiveAltButton))
                        controllerValue += 1.0f;
                    if (IsControllerInputCodeHeld(axis.controllerSettings.negativeButton) || IsControllerInputCodeHeld(axis.controllerSettings.negativeAltButton))
                        controllerValue += -1.0f;
                }
                break;
            }
            if (std::fabsf(controllerValue) > std::fabsf(value))
                value = controllerValue;
        }
        return value;
    }

    bool InputAxis::IsInputCodePressed(InputType type, InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        switch (type)
        {
        case InputType::KeyboardButton: return input::IsKeyPressed(static_cast<input::KeyCode>(inputCode));
        case InputType::MouseButton: return input::IsMouseButtonPressed(static_cast<input::MouseCode>(inputCode));
        }
        return false;
    }
    bool InputAxis::IsInputCodeHeld(InputType type, InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        switch (type)
        {
        case InputType::KeyboardButton: return input::IsKeyHeld(static_cast<input::KeyCode>(inputCode));
        case InputType::MouseButton: return input::IsMouseButtonHeld(static_cast<input::MouseCode>(inputCode));
        }
        return false;
    }
    bool InputAxis::IsInputCodeReleased(InputType type, InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        switch (type)
        {
        case InputType::KeyboardButton: return input::IsKeyReleased(static_cast<input::KeyCode>(inputCode));
        case InputType::MouseButton: return input::IsMouseButtonReleased(static_cast<input::MouseCode>(inputCode));
        }
        return false;
    }

    bool InputAxis::IsControllerInputCodePressed(InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        return input::IsControllerButtonPressed(static_cast<input::ControllerButtonCode>(inputCode));
    }
    bool InputAxis::IsControllerInputCodeHeld(InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        return input::IsControllerButtonHeld(static_cast<input::ControllerButtonCode>(inputCode));
    }
    bool InputAxis::IsControllerInputCodeReleased(InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return false;
        return input::IsControllerButtonReleased(static_cast<input::ControllerButtonCode>(inputCode));
    }


    float InputAxis::GetControllerAxisValue(InputCode inputCode)
    {
        if (inputCode == InputAxis::INPUTCODE_INVALID)
            return 0.0f;
        float value = input::GetControllerAxisValue(static_cast<input::ControllerAxisCode>(inputCode));
        if (value >= 0)
            return value / std::numeric_limits<short>().max();
        else
            return -value / std::numeric_limits<short>().min();
    }
}