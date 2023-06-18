/************************************************************************************//*!
\file           Input.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 24, 2022
\brief          Implementation of the general abstracted input system that will be used
                by the client.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "MouseCode.h"
#include "KeyCode.h"
#include "ControllerCode.h"

#include <vector>
#include <tuple>

/****************************************************************************//*!
 @brief     Interface of the Engine's Input system that will be used by
            by the client.
 
 @note      Check out KeyCode, MouseCode, ControllerButtonCode and
            ControllerAxisCode for more information on supported instructions.
*//*****************************************************************************/
namespace oo
{
    namespace input
    {
        void Init();
        void Update();
        void ShutDown();
        void AddController(int index);
        void RemoveController(int index);

        // % Percentage [0.0f to 1.0f]
        static float DeadZonePercent = 0.1f;

        /*-----------------------------------------------------------------------------*/
        /* Interface Functions                                                         */
        /*-----------------------------------------------------------------------------*/
        /****************************************************************************//*!
            @brief     Determine if a key by the given keycode is currently held down

            @note      This function will return true even when keypressed is true too.

            @return    Whether the key is currently down.
        *//*****************************************************************************/
        bool IsKeyHeld(KeyCode keycode);
        /****************************************************************************//*!
            @brief     Determine if a key by the given keycode has been triggered within
                    this frame.

            @return    Whether the key is triggered this frame .
        *//*****************************************************************************/
        bool IsKeyPressed(KeyCode keycode);
        /****************************************************************************//*!
            @brief     Determine if a key by the given keycode has been triggered within
                    this frame.

            @note      KeyRelease returns true once when a key that was pressed is released.
                    To check if a key is not pressed all the time
                    use !(IsKeyDown(keycode)) instead.

            @return    Whether the key is Released this frame.
        *//*****************************************************************************/
        bool IsKeyReleased(KeyCode keycode);
        /****************************************************************************//*!
            @brief     Determine if any supported key is currently held down

            @note      This function will return true even when keypressed is true too.

            @return    Whether any key is pressed down
        *//*****************************************************************************/
        bool IsAnyKeyHeld();
        /****************************************************************************//*!
            @brief     Determine if any supported key has been triggered within
                    this frame.

            @return    Whether any key is triggered this frame
        *//*****************************************************************************/
        bool IsAnyKeyPressed();
        /****************************************************************************//*!
            @brief     Determine if any supported key by the given keycode has
                    been released within this frame.

            @note      KeyRelease returns true once when a key that was pressed is released.

            @return    Whether any key is Released this frame.
        *//*****************************************************************************/
        bool IsAnyKeyReleased();
        /****************************************************************************//*!
            @brief     Retrieve a vector of supported keys that are currently pressed down.

            @return    Returns a vector to all supported keys that is released
        *//*****************************************************************************/
        std::vector<KeyCode> GetKeysHeld();
        /****************************************************************************//*!
            @brief     Retrieve a vector of supported keys that are currently pressed.

            @return    Returns a vector to all supported keys that is pressed down this
                    frame
        *//*****************************************************************************/
        std::vector<KeyCode> GetKeysPressed();
        /****************************************************************************//*!
            @brief     Retrieve a vector of supported keys that are currently released.

            @return    Returns a vector of supported keys that is released this frame.
        *//*****************************************************************************/
        std::vector<KeyCode> GetKeysReleased();

        /****************************************************************************//*!
            @brief     Determine if a mouse button by the given mouse code is
                    currently held down

            @note      This function will return true even when mousepressed is true too.

            @return    Whether the mouse button is currently down.
        *//*****************************************************************************/
        bool IsMouseButtonHeld(MouseCode button);
        /****************************************************************************//*!
            @brief     Determine if a mouse button by the given mouse code is
                    has been triggered this frame.

            @return    Whether the mouse button is being clicked this frame.
        *//*****************************************************************************/
        bool IsMouseButtonPressed(MouseCode button);
        /****************************************************************************//*!
            @brief     Determine if a mouse button by the given mouse code is
                    has been released this frame.

            @return    Whether the mouse button is being released this frame.
        *//*****************************************************************************/
        bool IsMouseButtonReleased(MouseCode button);
        /****************************************************************************//*!
            @brief     Determine if any supported mouse button is currently held down

            @note      This function will return true even when mousepressed is true too.

            @return    Whether any supported mouse button is currently down.
        *//*****************************************************************************/
        bool IsAnyMouseButtonHeld();
        /****************************************************************************//*!
            @brief     Determine if any supported mouse button is has
                    been triggered this frame.

            @return    Whether any supported mouse button has been clicked this frame.
        *//*****************************************************************************/
        bool IsAnyMouseButtonPressed();
        /****************************************************************************//*!
            @brief     Determine if any supported mouse button is has
                    been released this frame.

            @return    Whether any supported mouse button has been released this frame.
        *//*****************************************************************************/
        bool IsAnyMouseButtonReleased();
        /****************************************************************************//*!
            @brief     Retrieve all supported mouse button that is currently held down

            @return    Return a vector to all supported mouse buttons that are currently
                    held down.
        *//*****************************************************************************/
        std::vector<MouseCode> GetMouseButtonsHeld();
        /****************************************************************************//*!
            @brief     Retrieve all supported mouse button that is pressed this frame

            @return    Return a vector to all supported mouse buttons that is currently
                    pressed this frame.
        *//*****************************************************************************/
        std::vector<MouseCode> GetMouseButtonsPressed();
        /****************************************************************************//*!
            @brief     Retrieve all supported mouse button that is are released this frame

            @return    Return a vector to all supported mouse buttons that are released
                    this frame.
        *//*****************************************************************************/
        std::vector<MouseCode> GetMouseButtonsReleased();


        /****************************************************************************//*!
            @brief     Retrieve the current mouse position in screen coordinates
                    (top left 0,0)

            @return    a pair containing the x and y position of the mouse
        *//*****************************************************************************/
        std::pair<int, int> GetMousePosition();

        /****************************************************************************//*!
            @brief     Retrieve the change in mouse position in the current frame
                    in screen coordinates (top left 0,0)

            @return    a pair containing the x and y delta position of the mouse
        *//*****************************************************************************/
        std::pair<int, int> GetMouseDelta();

        /****************************************************************************//*!
            @brief     Retrieve the current X-axis mouse position in screen coordinates

            @return    X-axis position of where the mouse current is at
        *//*****************************************************************************/
        int GetMouseX();
        /****************************************************************************//*!
            @brief     Retrieve the current Y-axis mouse position in screen coordinates

            @return    Y-axis position of where the mouse current is at
        *//*****************************************************************************/
        int GetMouseY();


        /****************************************************************************//*!
            @brief     Determine if controller button by the given
                    ControllerButtonCode has been triggered within this frame.

            @note      ButtonPressed returns true once when a controller button
                    that was pressed.

            @return    Whether any controller button is Pressed this frame.
        *//*****************************************************************************/
        bool IsControllerButtonPressed(ControllerButtonCode iButton);
        /****************************************************************************//*!
            @brief     Determine if controller button by the given ControllerButtonCode
                    is currently held down

            @note      This function will return true even when buttonPressed is true too.

            @return    Whether any controller button is currently down.
        *//*****************************************************************************/
        bool IsControllerButtonHeld(ControllerButtonCode iButton);
        /****************************************************************************//*!
            @brief     Determine if controller button by the given
                    ControllerButtonCode has been released within this frame.

            @note      ButtonRelease returns true once when a controller button
                    that was pressed and is released currently.

            @return    Whether any controller button is Released this frame.
        *//*****************************************************************************/
        bool IsControllerButtonReleased(ControllerButtonCode iButton);
        /****************************************************************************//*!
            @brief     Retrieve the value of the supported Controller Stick and Axis
                    of given ControllerAxisValue.

            @return    Value of the supported controller axis this frame.
        *//*****************************************************************************/
        float GetControllerAxisValue(ControllerAxisCode iAxis);

        /****************************************************************************//*!
            @brief     Determine if any controller button is currently held down

            @note      This function will return true even when buttonPressed is true too.

            @return    Whether any supported controller button is currently down.
        *//*****************************************************************************/
        bool IsAnyControllerButtonHeld();
        /****************************************************************************//*!
            @brief     Determine if any supported controller button is has
                    been triggered this frame.

            @return    Whether any supported controller button has been clicked this frame.
        *//*****************************************************************************/
        bool IsAnyControllerButtonPressed();
        /****************************************************************************//*!
            @brief     Determine if any supported controller button is has
                    been released this frame.

            @return    Whether any supported controller button has been released this frame.
        *//*****************************************************************************/
        bool IsAnyControllerButtonReleased();
        /****************************************************************************//*!
            @brief     Retrieve all supported controller button that is currently held down

            @return    Return a vector to all supported controller buttons that
                    are currently held down.
        *//*****************************************************************************/
        std::vector<ControllerButtonCode> GetControllerButtonsHeld();
        /****************************************************************************//*!
            @brief     Retrieve all supported controller button that is pressed this frame

            @return    Return a vector to all supported controller buttons that is currently
                    pressed this frame.
        *//*****************************************************************************/
        std::vector<ControllerButtonCode> GetControllerButtonsPressed();
        /****************************************************************************//*!
            @brief     Retrieve all supported controller button that is are released
                    this frame

            @return    Return a vector to all supported controller buttons that are released
                    this frame.
        *//*****************************************************************************/
        std::vector<ControllerButtonCode> GetControllerButtonsReleased();

        /****************************************************************************//*!
            @brief     Determine if any controller axis is currently being used

            @return    Whether any supported controller axis is currently being used.
        *//*****************************************************************************/
        bool IsAnyControllerAxis();
        /****************************************************************************//*!
            @brief     Retrieve all supported controller axis that is currently being used

            @return    Return a vector of tuples containing
                    all supported controller axis and its values.
        *//*****************************************************************************/
        std::vector<std::tuple<ControllerAxisCode, float>> GetControllerAxis();

        /****************************************************************************//*!
			@brief     Makes the current controller vibrate for a given time and 
                       intensity(ranges from 0 - 1)
					   Setting the intensity to 0 will stop the vibration

            @return    True if successful, false otherwise
        *//*****************************************************************************/
		bool SetControllerVibration(float time, float intensity);
        /****************************************************************************//*!
            @brief     Makes the current controller vibrate for a given time and
                       intensity(ranges from 0 - 1) for the high and low frequency motors
                       Setting the intensity to 0 will stop the vibration

            @return    True if successful, false otherwise
        *//*****************************************************************************/
		bool SetControllerVibration(float time, float low_frequency_intensity, float high_frequency_intensity);
        /****************************************************************************//*!
            @brief     Stops the current controller vibrations via setting the intensity to 0

        *//*****************************************************************************/
        void StopControllerVibration();
        /*********************************************************************************//*
		\brief
			merge the collection of simulated keypresses
		*//**********************************************************************************/
		void SimulatedInputUpdate();
		/*********************************************************************************//*!
		\brief
			insert keypresses
		*//**********************************************************************************/
		void SimulateKeyInput(KeyCode key, bool pressed);
		/*********************************************************************************//*!
		\brief
			insert keypresses
		*//**********************************************************************************/
		void SimulatedMouse();
		/*********************************************************************************//*!
		\brief
			insert keypresses
		*//**********************************************************************************/
		void SimulatedMousePosition(short x, short y,short dx,short dy);
		/*********************************************************************************//*!
		\brief
			set simulation status
		*//**********************************************************************************/
		void SetSimulation(bool start);
		/*********************************************************************************//*!
		\brief
			set simulation mouse status
		*//**********************************************************************************/
		void SimulatedMouseButton(MouseCode mousebutton);
    }
}

