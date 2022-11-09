/************************************************************************************//*!
\file           InputAPI.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 28, 2022
\brief          Defines the exported helper functions that the C# scripts will use
                to check for player input

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "Ouroboros/Scripting/ExportAPI.h"
#include "Ouroboros/Core/Input.h"

#include "Ouroboros/Input/InputSystem.h"
#include "Ouroboros/Scripting/ScriptManager.h"

#include "Project.h"

namespace oo
{
    /*-----------------------------------------------------------------------------*/
    /* InputManager Functions for C#                                               */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API void Project_SaveInputs(const char* filePath)
    {
        Project::SaveInputs(Project::GetProjectFolder() / filePath);
    }

    SCRIPT_API void Project_LoadInputs(const char* filePath)
    {
        Project::LoadInputs(Project::GetProjectFolder() / filePath);
    }

    SCRIPT_API float GetAxis(const char* axisName)
    {
        return ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxisValue(axisName);
    }

    SCRIPT_API void InputManager_SetPositiveKeyCode(const char* axisName, input::KeyCode keyCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::KeyboardButton)
            axis.SetType(InputAxis::InputType::KeyboardButton);
        axis.GetSettings().positiveButton = static_cast<InputAxis::InputCode>(keyCode);
    }
    SCRIPT_API void InputManager_SetPositiveMouseCode(const char* axisName, input::MouseCode mouseCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::MouseButton)
            axis.SetType(InputAxis::InputType::MouseButton);
        axis.GetSettings().positiveButton = static_cast<InputAxis::InputCode>(mouseCode);
    }
    SCRIPT_API void InputManager_SetPositiveControllerButtonCode(const char* axisName, input::ControllerButtonCode buttonCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetControllerType() != InputAxis::ControllerInputType::Button)
            axis.SetControllerType(InputAxis::ControllerInputType::Button);
        axis.GetControllerSettings().positiveButton = static_cast<InputAxis::InputCode>(buttonCode);
    }
    SCRIPT_API void InputManager_SetPositiveControllerAxisCode(const char* axisName, input::ControllerAxisCode axisCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetControllerType() != InputAxis::ControllerInputType::Trigger_Joystick)
            axis.SetControllerType(InputAxis::ControllerInputType::Trigger_Joystick);
        axis.GetControllerSettings().positiveButton = static_cast<InputAxis::InputCode>(axisCode);
    }

    SCRIPT_API void InputManager_SetNegativeKeyCode(const char* axisName, input::KeyCode keyCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::KeyboardButton)
            axis.SetType(InputAxis::InputType::KeyboardButton);
        axis.GetSettings().negativeButton = static_cast<InputAxis::InputCode>(keyCode);
    }
    SCRIPT_API void InputManager_SetNegativeMouseCode(const char* axisName, input::MouseCode mouseCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::MouseButton)
            axis.SetType(InputAxis::InputType::MouseButton);
        axis.GetSettings().negativeButton = static_cast<InputAxis::InputCode>(mouseCode);
    }
    SCRIPT_API void InputManager_SetNegativeControllerButtonCode(const char* axisName, input::ControllerButtonCode buttonCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetControllerType() != InputAxis::ControllerInputType::Button)
            axis.SetControllerType(InputAxis::ControllerInputType::Button);
        axis.GetControllerSettings().negativeButton = static_cast<InputAxis::InputCode>(buttonCode);
    }

    /*-----------------------------------------------------------------------------*/
    /* Input Functions for C#                                                      */
    /*-----------------------------------------------------------------------------*/
    SCRIPT_API bool IsAnyKeyPressed()
    {
        return input::IsAnyKeyPressed();
    }

    SCRIPT_API bool IsAnyKeyHeld()
    {
        return input::IsAnyKeyHeld();
    }

    SCRIPT_API bool IsAnyKeyReleased()
    {
        return input::IsAnyKeyReleased();
    }

    SCRIPT_API bool IsKeyPressed(int key)
    {
        return input::IsKeyPressed(static_cast<input::KeyCode>(key));
    }

    SCRIPT_API bool IsKeyHeld(int key)
    {
        return input::IsKeyHeld(static_cast<input::KeyCode>(key));
    }

    SCRIPT_API bool IsKeyReleased(int key)
    {
        return input::IsKeyReleased(static_cast<input::KeyCode>(key));
    }

    SCRIPT_API void GetMousePosition(int* x, int* y)
    {
        std::pair<int, int> mousePos = input::GetMousePosition();
        *x = mousePos.first;
        *y = mousePos.second;
    }

    SCRIPT_API void GetMouseDelta(int* x, int* y)
    {
        std::pair<int, int> mouseDelta = input::GetMouseDelta();
        *x = mouseDelta.first;
        *y = mouseDelta.second;
    }

    SCRIPT_API bool IsAnyMouseButtonPressed()
    {
        return input::IsAnyMouseButtonPressed();
    }

    SCRIPT_API bool IsAnyMouseButtonHeld()
    {
        return input::IsAnyMouseButtonHeld();
    }

    SCRIPT_API bool IsAnyMouseButtonReleased()
    {
        return input::IsAnyMouseButtonReleased();
    }

    SCRIPT_API bool IsMouseButtonPressed(int button)
    {
        return input::IsMouseButtonPressed(static_cast<input::MouseCode>(button));
    }

    SCRIPT_API bool IsMouseButtonHeld(int button)
    {
        return input::IsMouseButtonHeld(static_cast<input::MouseCode>(button));
    }

    SCRIPT_API bool IsMouseButtonReleased(int button)
    {
        return input::IsMouseButtonReleased(static_cast<input::MouseCode>(button));
    }

    SCRIPT_API bool IsControllerButtonPressed(int button)
    {
        return input::IsControllerButtonPressed(static_cast<input::ControllerButtonCode>(button));
    }

    SCRIPT_API bool IsControllerButtonHeld(int button)
    {
        return input::IsControllerButtonHeld(static_cast<input::ControllerButtonCode>(button));
    }

    SCRIPT_API bool IsControllerButtonReleased(int button)
    {
        return input::IsControllerButtonReleased(static_cast<input::ControllerButtonCode>(button));
    }

    SCRIPT_API float GetControllerAxisValue(int axis)
    {
        return input::GetControllerAxisValue(static_cast<input::ControllerAxisCode>(axis));
    }
}