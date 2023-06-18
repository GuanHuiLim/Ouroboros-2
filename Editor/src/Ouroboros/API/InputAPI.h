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

    SCRIPT_API bool Project_HasInputsFile(const char* filePath)
    {
        return std::filesystem::exists(Project::GetProjectFolder() / filePath);
    }

    SCRIPT_API float GetAxis(const char* axisName)
    {
        return ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxisValue(axisName);
    }

    SCRIPT_API MonoArray* GetAxesAll()
    {
        auto const& trackers = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetTrackers();
        
        MonoClass* axisKlass = ScriptEngine::GetClass("ScriptCore", "Ouroboros", "InputManager/Axis");
        MonoArray* arr = ScriptEngine::CreateArray(axisKlass, trackers.size());
        
        unsigned index = 0;
        for (auto const& [key, tracker] : trackers)
        {
            MonoObject* axis = ScriptEngine::CreateObject(axisKlass);

            MonoString* nameString = ScriptEngine::CreateString(key.c_str());
            MonoClassField* nameField = mono_class_get_field_from_name(axisKlass, "name");
            mono_field_set_value(axis, nameField, nameString);

            mono_array_set(arr, MonoObject*, index, axis);
            ++index;
        }
        return arr;
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

    SCRIPT_API bool InputManager_IsPositiveKeyCode(const char* axisName, input::KeyCode keyCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::KeyboardButton)
            return false;
        return axis.GetSettings().positiveButton == static_cast<InputAxis::InputCode>(keyCode);
    }
    SCRIPT_API bool InputManager_IsPositiveMouseCode(const char* axisName, input::MouseCode mouseCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::MouseButton)
            return false;
        return axis.GetSettings().positiveButton == static_cast<InputAxis::InputCode>(mouseCode);
    }
    SCRIPT_API bool InputManager_IsPositiveControllerButtonCode(const char* axisName, input::ControllerButtonCode buttonCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetControllerType() != InputAxis::ControllerInputType::Button)
            return false;
        return axis.GetControllerSettings().positiveButton == static_cast<InputAxis::InputCode>(buttonCode);
    }
    SCRIPT_API bool InputManager_IsPositiveControllerAxisCode(const char* axisName, input::ControllerAxisCode axisCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetControllerType() != InputAxis::ControllerInputType::Trigger_Joystick)
            return false;
        return axis.GetControllerSettings().positiveButton == static_cast<InputAxis::InputCode>(axisCode);
    }

    SCRIPT_API bool InputManager_IsNegativeKeyCode(const char* axisName, input::KeyCode keyCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::KeyboardButton)
            return false;
        return axis.GetSettings().negativeButton == static_cast<InputAxis::InputCode>(keyCode);
    }
    SCRIPT_API bool InputManager_IsNegativeMouseCode(const char* axisName, input::MouseCode mouseCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetType() != InputAxis::InputType::MouseButton)
            return false;
        return axis.GetSettings().negativeButton == static_cast<InputAxis::InputCode>(mouseCode);
    }
    SCRIPT_API bool InputManager_IsNegativeControllerButtonCode(const char* axisName, input::ControllerButtonCode buttonCode)
    {
        InputAxis& axis = ScriptManager::s_SceneManager->GetActiveScene<Scene>()->GetWorld().Get_System<InputSystem>()->GetAxis(axisName);
        if (axis.GetControllerType() != InputAxis::ControllerInputType::Button)
            return false;
        return axis.GetControllerSettings().negativeButton == static_cast<InputAxis::InputCode>(buttonCode);
    }

    /*-----------------------------------------------------------------------------*/
    /* Keyboard Input Functions for C#                                             */
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

    SCRIPT_API int GetKeyPressed()
    {
        std::vector<input::KeyCode> keys = input::GetKeysPressed();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    SCRIPT_API int GetKeyHeld()
    {
        std::vector<input::KeyCode> keys = input::GetKeysHeld();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    SCRIPT_API int GetKeyReleased()
    {
        std::vector<input::KeyCode> keys = input::GetKeysReleased();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    /*-----------------------------------------------------------------------------*/
    /* Mouse Input Functions for C#                                                */
    /*-----------------------------------------------------------------------------*/

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

    SCRIPT_API int GetMouseButtonPressed()
    {
        std::vector<input::MouseCode> keys = input::GetMouseButtonsPressed();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    SCRIPT_API int GetMouseButtonHeld()
    {
        std::vector<input::MouseCode> keys = input::GetMouseButtonsHeld();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    SCRIPT_API int GetMouseButtonReleased()
    {
        std::vector<input::MouseCode> keys = input::GetMouseButtonsReleased();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    /*-----------------------------------------------------------------------------*/
    /* Controller Input Functions for C#                                           */
    /*-----------------------------------------------------------------------------*/

    SCRIPT_API bool IsAnyControllerButtonPressed()
    {
        return input::IsAnyControllerButtonPressed();
    }

    SCRIPT_API bool IsAnyControllerButtonHeld()
    {
        return input::IsAnyControllerButtonHeld();
    }

    SCRIPT_API bool IsAnyControllerButtonReleased()
    {
        return input::IsAnyControllerButtonReleased();
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

    SCRIPT_API int GetControllerButtonPressed()
    {
        std::vector<input::ControllerButtonCode> keys = input::GetControllerButtonsPressed();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    SCRIPT_API int GetControllerButtonHeld()
    {
        std::vector<input::ControllerButtonCode> keys = input::GetControllerButtonsHeld();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    SCRIPT_API int GetControllerButtonReleased()
    {
        std::vector<input::ControllerButtonCode> keys = input::GetControllerButtonsReleased();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]);
    }

    SCRIPT_API bool IsAnyControllerAxis()
    {
        return input::IsAnyControllerAxis();
    }

    SCRIPT_API float GetControllerAxisValue(int axis)
    {
        return input::GetControllerAxisValue(static_cast<input::ControllerAxisCode>(axis));
    }

    SCRIPT_API int GetControllerAxis()
    {
        std::vector<std::tuple<input::ControllerAxisCode, float>> keys = input::GetControllerAxis();
        if (keys.size() <= 0)
            ScriptEngine::ThrowNullException();
        return static_cast<int>(keys[0]._Myfirst._Val);
    }

    SCRIPT_API bool SetControllerVibration(float time, float intensity)
    {
        return input::SetControllerVibration(time, intensity);
    }

    SCRIPT_API bool SetControllerVibration_HighLow(float time, float low_frequency_intensity, float high_frequency_intensity)
    {
        return input::SetControllerVibration(time, low_frequency_intensity, high_frequency_intensity);
    }

    SCRIPT_API void StopControllerVibration()
    {
        input::StopControllerVibration();
    }
}

