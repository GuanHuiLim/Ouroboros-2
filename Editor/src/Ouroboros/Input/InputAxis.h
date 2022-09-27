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
                pressesRequired{ 0 }, maxGapTime{ 0 }, holdDurationRequired{ 0 }
            {
            }

            Settings(InputCode negativeButton, InputCode positiveButton, InputCode negativeAltButton, InputCode positiveAltButton,
                unsigned pressesRequired, float maxGapTime, float holdDurationRequired)
                : negativeButton{ negativeButton }, positiveButton{ positiveButton },
                negativeAltButton{ negativeAltButton }, positiveAltButton{ positiveAltButton },
                pressesRequired{ pressesRequired }, maxGapTime{ maxGapTime }, holdDurationRequired{ holdDurationRequired }
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

        inline std::string const& GetName() const { return name; }
        inline void SetName(std::string const& newName) { name = newName; }

        inline InputType const GetType() const { return type; }
        void SetType(InputType newType);

		inline ControllerInputType const GetControllerType() const { return controllerType; }
		void SetControllerType(ControllerInputType newType);

        inline Settings& GetSettings() { return settings; }
        inline Settings const& GetSettings() const { return settings; }

        inline Settings& GetControllerSettings() { return controllerSettings; }
        inline Settings const& GetControllerSettings() const { return controllerSettings; }

    public:
        class Tracker
        {
        public:
            Tracker(InputAxis const& axis);
            ~Tracker() = default;

            /*********************************************************************************//*!
            \brief      Used to update all tracked variables of the input axis the tracker is looking at

            \param      deltaTime
                    the amount of time passed since this was last called
            *//**********************************************************************************/
            void Update(float deltaTime);

            /*********************************************************************************//*!
            \brief      Used to get the current value of the input axis the tracker is looking at
                        based on how the tracked variables meet the input axis' conditions

            \return     the value of the input axis the tracker is looking at, usually with a range of [-1, 1]
            *//**********************************************************************************/
            float GetValue();

        private:
            InputAxis const& axis;
            float durationHeld;
            unsigned pressCount;
            float pressGapTimeLeft;
            InputAxis::InputCode lastPressed;

        private:
            /*********************************************************************************//*!
            \brief      Helper function to check if the conditions for input detection, if any, has been met

            \param      potentialButton
                    
            *//**********************************************************************************/
            void UpdateLastPressed(InputCode potentialButton);
            /*********************************************************************************//*!
            \brief      Helper function to check if the conditions for input detection, if any, has been met

            \param      potentialButton
                    
            *//**********************************************************************************/
            void UpdateLastPressedController(InputCode potentialButton);
        };

    private:
        /*********************************************************************************//*!
        \brief      Helper function used to 

        \param      type
                

        \param      inputCode
                

        \return     
        *//**********************************************************************************/
        static bool IsInputCodePressed(InputType type, InputCode inputCode);
        static bool IsInputCodeHeld(InputType type, InputCode inputCode);
        static bool IsInputCodeReleased(InputType type, InputCode inputCode);

        static bool IsControllerInputCodePressed(InputCode inputCode);
        static bool IsControllerInputCodeHeld(InputCode inputCode);
        static bool IsControllerInputCodeReleased(InputCode inputCode);
        static float GetControllerAxisValue(InputCode inputCode);
    };
}
