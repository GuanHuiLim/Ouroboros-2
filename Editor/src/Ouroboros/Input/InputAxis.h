#pragma once

#include <limits>
#include <string>

namespace oo
{
    class InputAxis
    {
    public:
        using InputCode = unsigned;
        static const InputCode INPUTCODE_INVALID = std::numeric_limits<InputCode>::max();

        enum class InputType
        {
            MouseMovement,
            MouseButton,
            KeyboardButton,
        };

        enum class ControllerInputType
        {
            Trigger_Joystick,
            Button,
        };

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

            void Update(float deltaTime);
            float GetValue();

        private:
            InputAxis const& axis;
            float durationHeld;
            unsigned pressCount;
            float pressGapTimeLeft;
            InputAxis::InputCode lastPressed;

        private:
            void UpdateLastPressed(InputCode potentialButton);
            void UpdateLastPressedController(InputCode potentialButton);
        };

    private:
        static bool IsInputCodePressed(InputType type, InputCode inputCode);
        static bool IsInputCodeHeld(InputType type, InputCode inputCode);
        static bool IsInputCodeReleased(InputType type, InputCode inputCode);

        static bool IsControllerInputCodePressed(InputCode inputCode);
        static bool IsControllerInputCodeHeld(InputCode inputCode);
        static bool IsControllerInputCodeReleased(InputCode inputCode);
        static float GetControllerAxisValue(InputCode inputCode);
    };
}
