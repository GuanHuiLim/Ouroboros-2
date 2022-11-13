/************************************************************************************//*!
\file           InputAxis.h
\project        Ouroboros
\author         Solomon Tan Teng Shue, t.tengshuesolomon, 620010020 | code contribution (100%)
\par            email: t.tengshuesolomon\@digipen.edu
\date           Sept 26, 2022
\brief          Declares and defines the classes and enums needed to create an axis
                used to obtain player input

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <limits>
#include <string>
#include <rttr/type>

#include <Ouroboros/Core/Timer.h>

namespace oo
{
    class InputAxis
    {
    public:
        using InputCode = unsigned; // main type to cast all input enums to, so InputAxis can check for all types of input
        static const InputCode INPUTCODE_INVALID = std::numeric_limits<InputCode>::max(); // used to check if InputCode is not assigned any valid input code

        // enum used to indicate the type of keyboard/mouse input to be used
        enum class InputType
        {
            MouseMovement,
            MouseButton,
            KeyboardButton,
        };
        // enum used to indicate the type of controller input to be used
        enum class ControllerInputType
        {
            Trigger_Joystick = 0,
            Button,
        };

        // Generalized Settings that both keyboard/mouse & controller input needs
        struct Settings
        {
        public:
            Settings() : negativeButton{ InputAxis::INPUTCODE_INVALID }, positiveButton{ InputAxis::INPUTCODE_INVALID },
                negativeAltButton{ InputAxis::INPUTCODE_INVALID }, positiveAltButton{ InputAxis::INPUTCODE_INVALID },
                pressesRequired{ 0 }, maxGapTime{ 0 }, holdDurationRequired{ 0 }, invert{ false }, onPressOnly{ false }
            {
            }

            Settings(InputCode negativeButton, InputCode positiveButton, InputCode negativeAltButton, InputCode positiveAltButton,
                unsigned pressesRequired, float maxGapTime, float holdDurationRequired, bool invert, bool onPressOnly)
                : negativeButton{ negativeButton }, positiveButton{ positiveButton },
                negativeAltButton{ negativeAltButton }, positiveAltButton{ positiveAltButton },
                pressesRequired{ pressesRequired }, maxGapTime{ maxGapTime }, holdDurationRequired{ holdDurationRequired },
                invert{ invert }, onPressOnly{ onPressOnly }
            {
            }

        public:
            InputCode negativeButton;
            InputCode positiveButton;
            InputCode negativeAltButton;
            InputCode positiveAltButton;
            unsigned pressesRequired;
            float maxGapTime;
            float holdDurationRequired;
            bool invert;
            bool onPressOnly;

        public:
            /*********************************************************************************//*!
            \brief      Helper function to check if the conditions for input detection, if any, has been met

            \param      pressCount
                    the number of times one of the setting's keys have been pressed in a row

            \param      durationHeld
                    the amount of time, in seconds, that one of the setting's keys have been held without interruption

            \return     true if the conditions have been met, else false
            *//**********************************************************************************/
            inline bool ConditionsMet(unsigned pressCount, float durationHeld) const
            {
                return (pressesRequired <= 0 || pressCount >= pressesRequired) && (holdDurationRequired <= 0.0f || durationHeld >= holdDurationRequired);
            }
        };
		RTTR_ENABLE();
    private:
        std::string name;
        // Keyboard & Mouse
        InputType type;
        Settings settings;

        // Controller
        ControllerInputType controllerType;
        Settings controllerSettings;

    public:
        InputAxis();
        ~InputAxis() = default;

        InputAxis(std::string name, InputType type, Settings const& settings, ControllerInputType controllerType, Settings const& controllerSettings);

        inline std::string GetName() const { return name; }
        inline void SetName(std::string newName) { name = newName; }

        inline InputType const GetType() const { return type; }
        void SetType(InputType newType);

		inline unsigned GetType_U() const { return unsigned(type); }
		void SetType_U(unsigned newType);


		inline ControllerInputType const GetControllerType() const { return controllerType; }
		void SetControllerType(ControllerInputType newType);

		inline unsigned GetControllerType_U() const { return (unsigned)controllerType; }
		void SetControllerType_U(unsigned newType);

        inline Settings& GetSettings() { return settings; }
		void SetSettings(Settings& setting) { settings = setting; }
        //inline Settings  GetSettings()  { return settings; }

        inline Settings& GetControllerSettings() { return controllerSettings; }
		void SetControllerSettings(Settings& setting) { controllerSettings = setting; }
        //inline Settings const GetControllerSettings()  { return controllerSettings; }

    public:
        class Tracker
        {
        public:
            Tracker(InputAxis& axis);
            ~Tracker() = default;

            /*********************************************************************************//*!
            \brief      Used to update all tracked variables of the input axis the tracker is looking at

            \param      deltaTime
                    the amount of time passed since this was last called
            *//**********************************************************************************/
            void Update(float deltaTime);

            inline InputAxis& GetAxis()
            {
                return axis;
            }

            /*********************************************************************************//*!
            \brief      Used to get the current value of the input axis the tracker is looking at
                        based on how the tracked variables meet the input axis' conditions

            \return     the value of the input axis the tracker is looking at, usually with a range of [-1, 1]
            *//**********************************************************************************/
            float GetValue();

        private:
            InputAxis& axis;
            float durationHeld;
            unsigned pressCount;
            float pressGapTimeLeft;
            InputAxis::InputCode lastPressed;
            bool isController;

        private:
            /*********************************************************************************//*!
            \brief      Used to get the current value of the input axis' keyboard or mouse input that the tracker is
                        looking at based on how the tracked variables meet the input axis' conditions

            \return     the value of the input axis the tracker is looking at, usually with a range of [-1, 1]
            *//**********************************************************************************/
            float GetKeyboardMouseValue();

            /*********************************************************************************//*!
            \brief      Used to get the current value of the input controller input that the tracker is
                        looking at based on how the tracked variables meet the input axis' conditions

            \return     the value of the input axis the tracker is looking at, usually with a range of [-1, 1]
            *//**********************************************************************************/
            float GetControllerValue();

            /*********************************************************************************//*!
            \brief      Helper function used to get the controller trigger or joystick axis value,
                        taking into consideration if only on press is wanted

            \param      inputCode
                    the input code of the joystick or trigger input to get the value of

            \return     the value of the input axis the tracker is looking at, usually with a range of [-1, 1]
            *//**********************************************************************************/
            inline float GetTriggerJoystickValue(InputCode inputCode)
            {
                if (axis.controllerSettings.onPressOnly && lastPressed == inputCode && durationHeld > timer::dt())
                    return 0.0f;
                return GetControllerAxisValue(inputCode);
            }

            /*********************************************************************************//*!
            \brief      Helper function to check if any keyboard/mouse input that is different
                        from the currently tracked last pressed input code is detected, and if so,
                        update the tracked variables accordingly

            \param      potentialButton
                    the potential input code to check for input detection
            *//**********************************************************************************/
            void UpdateLastPressed(InputCode potentialButton);
            /*********************************************************************************//*!
            \brief      Helper function to check if any controller button input that is different
                        from the currently tracked last pressed input code is detected, and if so,
                        update the tracked variables accordingly

            \param      potentialButton
                    the potential input code to check for input detection
            *//**********************************************************************************/
            void UpdateLastPressedControllerButton(InputCode potentialButton);
            /*********************************************************************************//*!
            \brief      Helper function to check if any controller axis input that is different
                        from the currently tracked last pressed input code is detected, and if so,
                        update the tracked variables accordingly

            \param      potentialAxis
                    the potential input code to check for input detection
            *//**********************************************************************************/
            void UpdateLastPressedControllerAxis(InputCode potentialAxis);
        };

    private:
        /*********************************************************************************//*!
        \brief      Helper function used to detect if a specific keyboard/mouse input has been
                    pressed in the current frame

        \param      type
                the type of keyboard/mouse input (e.g. mouse button, keyboard button)

        \param      inputCode
                the input code of the keyboard/mouse input to check

        \return     true if the specific keyboard/mouse input has been pressed in the current frame, else false
        *//**********************************************************************************/
        static bool IsInputCodePressed(InputType type, InputCode inputCode);
        /*********************************************************************************//*!
        \brief      Helper function used to detect if a specific keyboard/mouse input is being held

        \param      type
                the type of keyboard/mouse input (e.g. mouse button, keyboard button)

        \param      inputCode
                the input code of the keyboard/mouse input to check

        \return     true if the specific keyboard/mouse input is being held, else false
        *//**********************************************************************************/
        static bool IsInputCodeHeld(InputType type, InputCode inputCode);
        /*********************************************************************************//*!
        \brief      Helper function used to detect if a specific keyboard/mouse input has been
                    released in the current frame

        \param      type
                the type of keyboard/mouse input (e.g. mouse button, keyboard button)

        \param      inputCode
                the input code of the keyboard/mouse input to check

        \return     true if the specific keyboard/mouse input has been released in the current frame, else false
        *//**********************************************************************************/
        static bool IsInputCodeReleased(InputType type, InputCode inputCode);

        /*********************************************************************************//*!
        \brief      Helper function used to detect if a specific controller button has been
                    pressed in the current frame

        \param      inputCode
                the input code of the controller button to check

        \return     true if the specific controller button has been pressed in the current frame, else false
        *//**********************************************************************************/
        static bool IsControllerInputCodePressed(InputCode inputCode);
        /*********************************************************************************//*!
        \brief      Helper function used to detect if a specific controller button is being held

        \param      inputCode
                the input code of the controller button to check

        \return     true if the specific controller button is being held, else false
        *//**********************************************************************************/
        static bool IsControllerInputCodeHeld(InputCode inputCode);
        /*********************************************************************************//*!
        \brief      Helper function used to detect if a specific controller button has been
                    released in the current frame

        \param      inputCode
                the input code of the controller button to check

        \return     true if the specific controller button has been released in the current frame, else false
        *//**********************************************************************************/
        static bool IsControllerInputCodeReleased(InputCode inputCode);
        /*********************************************************************************//*!
        \brief      Helper function used to get a specific controller input value, from [-1, 1],
                    mainly used for joystick and trigger input

        \param      inputCode
                the input code of the controller axis to check

        \return     the specific controller input value, from [-1, 1]
        *//**********************************************************************************/
        static float GetControllerAxisValue(InputCode inputCode);
    };
}
