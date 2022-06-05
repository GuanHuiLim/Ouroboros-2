/************************************************************************************//*!
\file           WindowsInput.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 15, 2021
\brief          Describes a Windows(Platform) specific input that ements
                the generic Input interface.
                Currently using SDL as the backend abstraction.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#include "pch.h"
#include <sdl2/SDL.h>

#include "Ouroboros/Core/Input.h"

#include "Ouroboros/Core/Application.h"
#include "ControllerCode.h"

namespace input
{
    // Windows Platform currently using SDL
    // SDL Specific Input Requirements

    int m_keyLength;
    const Uint8* m_keyboardState;
    Uint8* m_prevKeyboardState;

    Uint32 m_prevMouseState;
    Uint32 m_mouseState;

    int m_mouseXPos;
    int m_mouseYPos;

    int m_mouseXDelta;
    int m_mouseYDelta;
    
    // Controller information
    SDL_GameController* m_pGameController;
    int m_controllerIndex;

    // Information about the state of the controller
    Uint8 m_uButtonStates[(size_t)ControllerButtonCode::MAX];
    Uint8 m_uButtonStatesPrev[(size_t)ControllerButtonCode::MAX];
    float m_fAxisValues[(size_t)ControllerAxisCode::MAX];

    void Init()
    {
        m_prevMouseState = m_mouseState = m_mouseXPos = m_mouseYPos = 0;

        m_keyboardState = SDL_GetKeyboardState(&m_keyLength);
        m_prevKeyboardState = new Uint8[m_keyLength];
        memcpy(m_prevKeyboardState, m_keyboardState, m_keyLength);

        //Controller
        {
            // Set the controller to NULL
            m_pGameController = nullptr;
            m_controllerIndex = -1;

            // Set the buttons and axis to 0
            memset(m_uButtonStates, 0, sizeof(Uint8) * (size_t)ControllerButtonCode::MAX);
            memset(m_uButtonStatesPrev, 0, sizeof(Uint8) * (size_t)ControllerButtonCode::MAX);
            memset(m_fAxisValues, 0, sizeof(float) * (size_t)ControllerAxisCode::MAX);
        }
    }

    void Update()
    {
        memcpy(m_prevKeyboardState, m_keyboardState, m_keyLength);
        m_prevMouseState = m_mouseState;

        m_mouseState = SDL_GetMouseState(&m_mouseXPos, &m_mouseYPos);

        SDL_GetRelativeMouseState(&m_mouseXDelta, &m_mouseYDelta);

        //int state = SDL_GameControllerEventState(SDL_QUERY);

        //Controller
        // If there is no controllers attached exit
        if (m_pGameController != nullptr)
        {
            // Copy the buttons state to the previous frame info
            memcpy(&m_uButtonStatesPrev, &m_uButtonStates, sizeof(Uint8) * (size_t)ControllerButtonCode::MAX);

            // Update the controller SDL info
            //SDL_GameControllerUpdate();

            // Obtain the current button values
            for (size_t b = 0; b < static_cast<size_t>(ControllerButtonCode::MAX); ++b)
            {
                m_uButtonStates[b] = SDL_GameControllerGetButton(m_pGameController, (SDL_GameControllerButton)b);
            }
            // Obtain the current axis value
            for (size_t a = 0; a < static_cast<size_t>(ControllerAxisCode::MAX); ++a)
            {
                m_fAxisValues[a] = SDL_GameControllerGetAxis(m_pGameController, (SDL_GameControllerAxis)a);
            }
        }
    }

    void ShutDown()
    {
        delete[] m_prevKeyboardState;
        m_prevKeyboardState = nullptr;
    }

    void AddController(int index)
    {
        ///--- If there is no controller attached
        if (m_pGameController == nullptr)
        {
            ///--- Open the controller
            m_controllerIndex = index;
            m_pGameController = SDL_GameControllerOpen(m_controllerIndex);
            ///--- Set the memory to 0 to avoid problems with previous added controllers
            memset(m_uButtonStates, 0, sizeof(Uint8) * (size_t)ControllerButtonCode::MAX);
            memset(m_uButtonStatesPrev, 0, sizeof(Uint8) * (size_t)ControllerButtonCode::MAX);
            memset(m_fAxisValues, 0, sizeof(float) * (size_t)ControllerAxisCode::MAX);
        }
    }

    void RemoveController(int index)
    {
        ///--- Check if is the same controller
        if (m_controllerIndex == index)
        {
            m_controllerIndex = -1;
            m_pGameController = nullptr;
        }
    }

    bool IsKeyHeld(const KeyCode keycode)
    {
        return m_keyboardState[static_cast<int>(keycode)];
    }

    bool IsKeyPressed(const KeyCode keycode)
    {
        return !m_prevKeyboardState[static_cast<int>(keycode)] && m_keyboardState[static_cast<int>(keycode)];
    }

    bool IsKeyReleased(const KeyCode keycode)
    {
        return m_prevKeyboardState[static_cast<int>(keycode)] && !m_keyboardState[static_cast<int>(keycode)];
    }

    bool IsAnyKeyHeld()
    {
        for (KeyCode keycode{ 0 }; keycode < m_keyLength; ++keycode)
        {
            if (IsKeyHeld(keycode)) return true;
        }

        return false;
    }

    bool IsAnyKeyPressed()
    {
        for (KeyCode keycode{ 0 }; keycode < m_keyLength; ++keycode)
        {
            if (IsKeyPressed(keycode)) return true;
        }

        return false;
    }

    bool IsAnyKeyReleased()
    {
        for (KeyCode keycode{ 0 }; keycode < m_keyLength; ++keycode)
        {
            if (IsKeyReleased(keycode)) return true;
        }

        return false;
    }

    std::vector<KeyCode> GetKeysHeld()
    {
        std::vector<KeyCode> keys;

        for (KeyCode keycode{ 0 }; keycode < m_keyLength; ++keycode)
        {
            if (IsKeyHeld(keycode)) keys.emplace_back(keycode);
        }

        return keys;
    }

    std::vector<KeyCode> GetKeysPressed()
    {
        std::vector<KeyCode> keys;

        for (KeyCode keycode{ 0 }; keycode < m_keyLength; ++keycode)
        {
            if (IsKeyPressed(keycode)) keys.emplace_back(keycode);
        }

        return keys;
    }

    std::vector<KeyCode> GetKeysReleased()
    {
        std::vector<KeyCode> keys;

        for (KeyCode keycode{ 0 }; keycode < m_keyLength; ++keycode)
        {
            if (IsKeyReleased(keycode)) keys.emplace_back(keycode);
        }

        return keys;
    }


    bool IsMouseButtonHeld(const MouseCode button)
    {
        Uint32 mask = 0;

        switch (button)
        {
        case Mouse::ButtonLeft:
            mask = SDL_BUTTON_LMASK;
            break;

        case Mouse::ButtonRight:
            mask = SDL_BUTTON_RMASK;
            break;

        case Mouse::ButtonMiddle:
            mask = SDL_BUTTON_MMASK;
            break;

        case Mouse::ButtonLast:
            mask = SDL_BUTTON_X1MASK;
            break;

        }

        return (m_mouseState & mask);
    }

    bool IsMouseButtonPressed(const MouseCode button)
    {
        Uint32 mask = 0;

        switch (button)
        {
        case Mouse::ButtonLeft:
            mask = SDL_BUTTON_LMASK;
            break;

        case Mouse::ButtonRight:
            mask = SDL_BUTTON_RMASK;
            break;

        case Mouse::ButtonMiddle:
            mask = SDL_BUTTON_MMASK;
            break;

        case Mouse::ButtonLast:
            mask = SDL_BUTTON_X1MASK;
            break;
        }

        return !(m_prevMouseState & mask) && (m_mouseState & mask);
    }

    bool IsMouseButtonReleased(const MouseCode button)
    {
        Uint32 mask = 0;

        switch (button)
        {
        case Mouse::ButtonLeft:
            mask = SDL_BUTTON_LMASK;
            break;

        case Mouse::ButtonRight:
            mask = SDL_BUTTON_RMASK;
            break;

        case Mouse::ButtonMiddle:
            mask = SDL_BUTTON_MMASK;
            break;

        case Mouse::ButtonLast:
            mask = SDL_BUTTON_X1MASK;
            break;
        }

        return (m_prevMouseState & mask) && !(m_mouseState & mask);
    }

    bool IsAnyMouseButtonHeld()
    {
        for (MouseCode mousecode{ 0 }; mousecode <= Mouse::ButtonLast; ++mousecode)
        {
            if (IsMouseButtonHeld(mousecode)) return true;
        }

        return false;
    }

    bool IsAnyMouseButtonPressed()
    {

        for (MouseCode mousecode{ 0 }; mousecode <= Mouse::ButtonLast; ++mousecode)
        {
            if (IsMouseButtonPressed(mousecode)) return true;
        }

        return false;
    }

    bool IsAnyMouseButtonReleased()
    {

        for (MouseCode mousecode{ 0 }; mousecode <= Mouse::ButtonLast; ++mousecode)
        {
            if (IsMouseButtonReleased(mousecode)) return true;
        }

        return false;
    }

    std::vector<MouseCode> GetMouseButtonsHeld()
    {
        std::vector<MouseCode> mouseButtons;

        for (MouseCode mousecode{ 0 }; mousecode <= Mouse::ButtonLast; ++mousecode)
        {
            if (IsMouseButtonHeld(mousecode)) mouseButtons.emplace_back(mousecode);
        }

        return mouseButtons;
    }

    std::vector<MouseCode> GetMouseButtonsPressed()
    {
        std::vector<MouseCode> mouseButtons;

        for (MouseCode mousecode{ 0 }; mousecode <= Mouse::ButtonLast; ++mousecode)
        {
            if (IsMouseButtonPressed(mousecode)) mouseButtons.emplace_back(mousecode);
        }

        return mouseButtons;
    }

    std::vector<MouseCode> GetMouseButtonsReleased()
    {
        std::vector<MouseCode> mouseButtons;

        for (MouseCode mousecode{ 0 }; mousecode <= Mouse::ButtonLast; ++mousecode)
        {
            if (IsMouseButtonReleased(mousecode)) mouseButtons.emplace_back(mousecode);
        }

        return mouseButtons;
    }


    std::pair<int, int> GetMousePosition()
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return { x, y };
    }

    std::pair<int, int> GetMouseDelta()
    {
        return { m_mouseXDelta, m_mouseYDelta };   // invert y so that you get bottom left as 0,0 instead of top left as 0,0 (default is window space)
    }

    int GetMouseX()
    {
        return GetMousePosition().first;
    }

    int GetMouseY()
    {
        return GetMousePosition().second;
    }


    bool IsControllerButtonPressed(ControllerButtonCode iButton)
    {
        return (m_uButtonStates[(size_t)iButton] == 1
            && m_uButtonStatesPrev[(size_t)iButton] == 0);
    }

    bool IsControllerButtonHeld(ControllerButtonCode iButton)
    {
        return (m_uButtonStates[(size_t)iButton] == 1);
    }

    bool IsControllerButtonReleased(ControllerButtonCode iButton)
    {
        return (m_uButtonStates[(size_t)iButton] == 0
            && m_uButtonStatesPrev[(size_t)iButton] == 1);
    }

    float GetControllerAxisValue(ControllerAxisCode iAxis)
    {
        auto result = m_fAxisValues[(size_t)iAxis];
        //Deadzone. can be improved.
        if (result > -8000 && result < 8000) return 0.f;
        return result;
    }


    bool IsAnyControllerButtonHeld()
    {
        for (ControllerButtonCode controllerBtnCode{ 0 }; controllerBtnCode < ControllerButtonCode::MAX; ++controllerBtnCode)
        {
            if (IsControllerButtonHeld(controllerBtnCode)) return true;
        }

        return false;
    }

    bool IsAnyControllerButtonPressed()
    {
        for (ControllerButtonCode controllerBtnCode{ 0 }; controllerBtnCode < ControllerButtonCode::MAX; ++controllerBtnCode)
        {
            if (IsControllerButtonPressed(controllerBtnCode)) return true;
        }

        return false;
    }

    bool IsAnyControllerButtonReleased()
    {
        for (ControllerButtonCode controllerBtnCode{ 0 }; controllerBtnCode < ControllerButtonCode::MAX; ++controllerBtnCode)
        {
            if (IsControllerButtonReleased(controllerBtnCode)) return true;
        }

        return false;
    }

    std::vector<ControllerButtonCode> GetControllerButtonsHeld()
    {
        std::vector<ControllerButtonCode> controllerButtons;

        for (ControllerButtonCode controllerBtnCode{ 0 }; controllerBtnCode < ControllerButtonCode::MAX; ++controllerBtnCode)
        {
            if (IsControllerButtonHeld(controllerBtnCode)) controllerButtons.emplace_back(controllerBtnCode);
        }

        return controllerButtons;
    }

    std::vector<ControllerButtonCode> GetControllerButtonsPressed()
    {
        std::vector<ControllerButtonCode> controllerButtons;

        for (ControllerButtonCode controllerBtnCode{ 0 }; controllerBtnCode < ControllerButtonCode::MAX; ++controllerBtnCode)
        {
            if (IsControllerButtonPressed(controllerBtnCode)) controllerButtons.emplace_back(controllerBtnCode);
        }

        return controllerButtons;
    }

    std::vector<ControllerButtonCode> GetControllerButtonsReleased()
    {
        std::vector<ControllerButtonCode> controllerButtons;

        for (ControllerButtonCode controllerBtnCode{ 0 }; controllerBtnCode < ControllerButtonCode::MAX; ++controllerBtnCode)
        {
            if (IsControllerButtonReleased(controllerBtnCode)) controllerButtons.emplace_back(controllerBtnCode);
        }

        return controllerButtons;
    }


    bool IsAnyControllerAxis()
    {
        for (ControllerAxisCode controllerAxisCode{ 0 }; controllerAxisCode < ControllerAxisCode::MAX; ++controllerAxisCode)
        {
            if (GetControllerAxisValue(controllerAxisCode) != 0.f) return true;
        }

        return false;
    }

    std::vector<std::tuple<ControllerAxisCode, float>> GetControllerAxis()
    {
        std::vector<std::tuple<ControllerAxisCode, float>> controllerAxis;

        for (ControllerAxisCode controllerAxisCode{ 0 }; controllerAxisCode < ControllerAxisCode::MAX; ++controllerAxisCode)
        {
            float val = GetControllerAxisValue(controllerAxisCode);
            if (val != 0.f) controllerAxis.emplace_back(controllerAxisCode, val);
        }

        return controllerAxis;
    }

}