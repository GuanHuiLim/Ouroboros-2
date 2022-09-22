#include "pch.h"
#include "InputAxis.h"

#include <Ouroboros/Core/Input.h>

namespace oo
{
    InputAxis::InputAxis()
        : name{ "Axis Name" }, type{InputType::KeyboardButton}, negativeButton{InputAxis::INPUTCODE_INVALID}, positiveButton{InputAxis::INPUTCODE_INVALID}, negativeAltButton{InputAxis::INPUTCODE_INVALID}, positiveAltButton{InputAxis::INPUTCODE_INVALID},
        pressesRequired{ 0 }, maxGapTime{ 0 }, holdDurationRequired{ 0 }
    {

    }

    InputAxis::InputAxis(std::string name, InputType type, InputCode negativeButton, InputCode positiveButton, InputCode negativeAltButton, InputCode positiveAltButton,
        unsigned pressesRequired, float maxGapTime, float holdDurationRequired)
        : name{ name }, type{ type }, negativeButton{ negativeButton }, positiveButton{ positiveButton }, negativeAltButton{ negativeAltButton }, positiveAltButton{ positiveAltButton },
        pressesRequired{ pressesRequired }, maxGapTime{ maxGapTime }, holdDurationRequired{ holdDurationRequired }
    {

    }

    void InputAxis::SetType(InputType newType)
    {
        type = newType;
        switch (type)
        {
        default:
            negativeButton = InputAxis::INPUTCODE_INVALID;
            negativeAltButton = InputAxis::INPUTCODE_INVALID;
            positiveButton = InputAxis::INPUTCODE_INVALID;
            positiveAltButton = InputAxis::INPUTCODE_INVALID;
            pressesRequired = 0;
            maxGapTime = 0.0f;
            holdDurationRequired = 0.0f;
            break;
        }
    }

    InputAxis::Tracker::Tracker(InputAxis const& axis)
        : axis{ axis }, durationHeld{ 0.0f }, pressCount{ 0 }, pressGapTimeLeft{ axis.maxGapTime }, lastPressed{ InputAxis::INPUTCODE_INVALID }
    {

    }

    void InputAxis::Tracker::Update(float deltaTime)
    {
        if (axis.type == InputType::MouseMovement)
            return;
        // update last pressed
        if (lastPressed != axis.positiveButton && IsInputCodePressed(axis.type, axis.positiveButton))
        {
            lastPressed = axis.positiveButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
        else if (lastPressed != axis.positiveAltButton && IsInputCodePressed(axis.type, axis.positiveAltButton))
        {
            lastPressed = axis.positiveAltButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
        else if (lastPressed != axis.negativeButton && IsInputCodePressed(axis.type, axis.negativeButton))
        {
            lastPressed = axis.negativeButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
        else if (lastPressed != axis.negativeAltButton && IsInputCodePressed(axis.type, axis.negativeAltButton))
        {
            lastPressed = axis.negativeAltButton;
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
            pressGapTimeLeft = axis.maxGapTime;
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

    bool InputAxis::Tracker::Satisfied()
    {
        return (axis.pressesRequired <= 0 || pressCount >= axis.pressesRequired)
            && (axis.holdDurationRequired <= 0.0f || durationHeld >= axis.holdDurationRequired);
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
        default:
        {
            if (IsInputCodeHeld(axis.type, axis.GetPositiveButton()) || IsInputCodeHeld(axis.type, axis.GetPositiveAltButton()))
                value += 1.0f;
            if (IsInputCodeHeld(axis.type, axis.GetNegativeButton()) || IsInputCodeHeld(axis.type, axis.GetNegativeAltButton()))
                value += -1.0f;
        }
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
}