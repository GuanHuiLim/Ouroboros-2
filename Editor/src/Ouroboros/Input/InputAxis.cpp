/************************************************************************************//*!
\file           InputAxis.cpp
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 26, 2022
\brief          defines the classes and enums needed to create an axis
                used to obtain player input

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include "InputAxis.h"

#include <Ouroboros/Core/Input.h>
#include <rttr/registration>
namespace oo
{
	RTTR_REGISTRATION
	{
		using namespace rttr;
	registration::class_<InputAxis::Settings>("Setting")
		.property("Negative Btn", &InputAxis::Settings::negativeButton)
		.property("Positive Btn", &InputAxis::Settings::positiveButton)
		.property("Negative Alt Btn", &InputAxis::Settings::negativeAltButton)
		.property("Positive Alt Btn", &InputAxis::Settings::positiveAltButton)
		.property("Presses Required", &InputAxis::Settings::pressesRequired)
		.property("Max Gap Time", &InputAxis::Settings::maxGapTime)
		.property("Hold Duration Required", &InputAxis::Settings::holdDurationRequired)
        .property("Invert Value", &InputAxis::Settings::invert)
        .property("On Press Only", &InputAxis::Settings::onPressOnly);

	registration::class_<InputAxis>("Input Axis")
		.property("Name", &InputAxis::GetName, &InputAxis::SetName)
		.property("Type", &InputAxis::GetType_U, &InputAxis::SetType_U)
		.property("Settings", &InputAxis::GetSettings, &InputAxis::SetSettings)
		.property("Controller Type", &InputAxis::GetControllerType_U, &InputAxis::SetControllerType_U)
		.property("Controller Settings" , &InputAxis::GetControllerSettings, &InputAxis::SetControllerSettings);

	}
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

        settings.negativeButton = InputAxis::INPUTCODE_INVALID;
        settings.negativeAltButton = InputAxis::INPUTCODE_INVALID;
        settings.positiveButton = InputAxis::INPUTCODE_INVALID;
        settings.positiveAltButton = InputAxis::INPUTCODE_INVALID;
        settings.pressesRequired = 0;
        settings.maxGapTime = 0.0f;
        settings.holdDurationRequired = 0.0f;
    }

	void oo::InputAxis::SetType_U(unsigned newType)
	{
		type = (oo::InputAxis::InputType)newType;

		settings.negativeButton = InputAxis::INPUTCODE_INVALID;
		settings.negativeAltButton = InputAxis::INPUTCODE_INVALID;
		settings.positiveButton = InputAxis::INPUTCODE_INVALID;
		settings.positiveAltButton = InputAxis::INPUTCODE_INVALID;
		settings.pressesRequired = 0;
		settings.maxGapTime = 0.0f;
		settings.holdDurationRequired = 0.0f;
	}

    void InputAxis::SetControllerType(ControllerInputType newType)
    {
		controllerType = newType;

        controllerSettings.negativeButton = InputAxis::INPUTCODE_INVALID;
        controllerSettings.negativeAltButton = InputAxis::INPUTCODE_INVALID;
        controllerSettings.positiveButton = InputAxis::INPUTCODE_INVALID;
        controllerSettings.positiveAltButton = InputAxis::INPUTCODE_INVALID;
        controllerSettings.pressesRequired = 0;
        controllerSettings.maxGapTime = 0.0f;
        controllerSettings.holdDurationRequired = 0.0f;
    }

	void oo::InputAxis::SetControllerType_U(unsigned newType)
	{
		controllerType = (oo::InputAxis::ControllerInputType)newType;

		controllerSettings.negativeButton = InputAxis::INPUTCODE_INVALID;
		controllerSettings.negativeAltButton = InputAxis::INPUTCODE_INVALID;
		controllerSettings.positiveButton = InputAxis::INPUTCODE_INVALID;
		controllerSettings.positiveAltButton = InputAxis::INPUTCODE_INVALID;
		controllerSettings.pressesRequired = 0;
		controllerSettings.maxGapTime = 0.0f;
		controllerSettings.holdDurationRequired = 0.0f;
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
        UpdateLastPressed(axis.settings.positiveButton);
        UpdateLastPressed(axis.settings.positiveAltButton);
        UpdateLastPressed(axis.settings.negativeButton);
        UpdateLastPressed(axis.settings.negativeAltButton);

        if (axis.controllerType == ControllerInputType::Button)
        {
            UpdateLastPressedController(axis.controllerSettings.positiveButton);
            UpdateLastPressedController(axis.controllerSettings.positiveAltButton);
            UpdateLastPressedController(axis.controllerSettings.negativeButton);
            UpdateLastPressedController(axis.controllerSettings.negativeAltButton);
        }

        if (pressGapTimeLeft > 0.0f)
        {
            pressGapTimeLeft -= deltaTime;
            if (pressGapTimeLeft <= 0.0f)
                pressCount = 0;
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
        if (axis.controllerType == ControllerInputType::Button)
        {
            if (IsControllerInputCodePressed(lastPressed))
            {
                ++pressCount;
                pressGapTimeLeft = axis.settings.maxGapTime;
            }
            if (IsControllerInputCodeHeld(lastPressed))
            {
                durationHeld += deltaTime;
            }
            if (IsControllerInputCodeReleased(lastPressed))
            {
                durationHeld = 0.0f;
            }
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
                        value = static_cast<float>(-input::GetMouseDelta().first);
                    else if (axis.GetName() == "Mouse Y")
                        value = static_cast<float>(input::GetMouseDelta().second);
                }
                break;
            default:
                {
                    if (axis.settings.onPressOnly)
                    {
                        if (IsInputCodePressed(axis.type, axis.settings.positiveButton) || IsInputCodePressed(axis.type, axis.settings.positiveAltButton))
                            value += 1.0f;
                        if (IsInputCodePressed(axis.type, axis.settings.negativeButton) || IsInputCodePressed(axis.type, axis.settings.negativeAltButton))
                            value += -1.0f;
                    }
                    else
                    {
                        if (IsInputCodeHeld(axis.type, axis.settings.positiveButton) || IsInputCodeHeld(axis.type, axis.settings.positiveAltButton))
                            value += 1.0f;
                        if (IsInputCodeHeld(axis.type, axis.settings.negativeButton) || IsInputCodeHeld(axis.type, axis.settings.negativeAltButton))
                            value += -1.0f;
                    }
                }
            }
            if (axis.settings.invert)
                value *= -1;
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
                    if(axis.controllerSettings.onPressOnly)
                    {
                        if (IsControllerInputCodePressed(axis.controllerSettings.positiveButton) || IsControllerInputCodePressed(axis.controllerSettings.positiveAltButton))
                            controllerValue += 1.0f;
                        if (IsControllerInputCodePressed(axis.controllerSettings.negativeButton) || IsControllerInputCodePressed(axis.controllerSettings.negativeAltButton))
                            controllerValue += -1.0f;
                    }
                    else
                    {
                        if (IsControllerInputCodeHeld(axis.controllerSettings.positiveButton) || IsControllerInputCodeHeld(axis.controllerSettings.positiveAltButton))
                            controllerValue += 1.0f;
                        if (IsControllerInputCodeHeld(axis.controllerSettings.negativeButton) || IsControllerInputCodeHeld(axis.controllerSettings.negativeAltButton))
                            controllerValue += -1.0f;
                    }
                }
                break;
            }
            if (axis.controllerSettings.invert)
                controllerValue *= -1;
            if (std::fabsf(controllerValue) > std::fabsf(value))
                value = controllerValue;
        }
        return value;
    }

    void InputAxis::Tracker::UpdateLastPressed(InputCode potentialButton)
    {
        if (lastPressed != potentialButton && IsInputCodePressed(axis.type, potentialButton))
        {
            lastPressed = potentialButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
    }
    void InputAxis::Tracker::UpdateLastPressedController(InputCode potentialButton)
    {
        if (lastPressed != potentialButton && IsControllerInputCodePressed(potentialButton))
        {
            lastPressed = potentialButton;
            pressCount = 0;
            durationHeld = 0.0f;
        }
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
        input::ControllerAxisCode axisCode = static_cast<input::ControllerAxisCode>(inputCode);
        return input::GetControllerAxisValue(axisCode);
    }
}