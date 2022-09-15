#pragma once

#include <string>
#include <unordered_map>
#include <exception>
#include <limits>

#include <Ouroboros/Core/KeyCode.h>
#include <Ouroboros/Core/MouseCode.h>

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

    private:
        std::string name;

        //float sensitivity;
        //float gravity;
        //float deadZone;
        //bool snap;
        //bool invert;

        // Keyboard & Mouse
        InputType type;
        InputCode negativeButton;
        InputCode positiveButton;
        InputCode negativeAltButton;
        InputCode positiveAltButton;
        unsigned pressesRequired;
        float maxGapTime;
        float holdDurationRequired;

    public:
        InputAxis() = default;
        ~InputAxis() = default;

        InputAxis(std::string name, InputType type, InputCode negativeButton, InputCode positiveButton, InputCode negativeAltButton, InputCode positiveAltButton,
            unsigned pressesRequired, float maxGapTime, float holdDurationRequired)
            : name{ name }, type{ type }, negativeButton{ negativeButton }, positiveButton{ positiveButton }, negativeAltButton{ negativeAltButton }, positiveAltButton{ positiveAltButton },
            pressesRequired{ pressesRequired }, maxGapTime{ maxGapTime }, holdDurationRequired{ holdDurationRequired }
        {

        }

        inline std::string const& GetName() const { return name; }
        inline void SetName(std::string const& newName) { name = newName; }

        inline InputType const GetType() const { return type; }
        inline void SetType(InputType newType)
        {
            type = newType;
            switch (type)
            {
            case InputType::KeyboardButton: break;
            case InputType::MouseButton: break;
            }
        }

        inline InputCode const GetNegativeButton() const { return negativeButton; }
        inline void SetNegativeButton(InputCode newCode) { negativeButton = newCode; }

        inline InputCode const GetPositiveButton() const { return positiveButton; }
        inline void SetPositiveButton(InputCode newCode) { positiveButton = newCode; }

        inline InputCode const GetNegativeAltButton() const { return negativeAltButton; }
        inline void SetNegativeAltButton(InputCode newCode) { negativeAltButton = newCode; }

        inline InputCode const GetPositiveAltButton() const { return positiveAltButton; }
        inline void SetPositiveAltButton(InputCode newCode) { positiveAltButton = newCode; }

        inline unsigned const GetPressesRequired() const { return pressesRequired; }
        inline void SetPressesRequired(unsigned requiredCount) { pressesRequired = requiredCount; }

        inline float const GetMaxGapTime() const { return maxGapTime; }
        inline void SetMaxGapTime(float gapTime) { pressesRequired = gapTime; }

        inline float const GetHoldDurationRequired() const { return holdDurationRequired; }
        inline void SetHoldDurationRequired(float holdDuration) { holdDurationRequired = holdDuration; }

    public:
        class Tracker
        {
        public:
            Tracker(InputAxis const& axis);
            ~Tracker() = default;

            void Update(float deltaTime);
            bool Satisfied();
            float GetValue();

        private:
            InputAxis const& axis;
            float durationHeld;
            unsigned pressCount;
            InputAxis::InputCode lastPressed;
        };

    private:
        static bool IsKeyboardKeyPressed(InputCode inputCode);
        static bool IsKeyboardKeyHeld(InputCode inputCode);
        static bool IsKeyboardKeyReleased(InputCode inputCode);

        static bool IsMouseButtonHeld(InputCode inputCode);
    };

    class InputManager final
    {
    public:
        static void LoadDefault();

        static inline void Load(std::unordered_map<std::string, InputAxis> loadedAxes)
        {
            axes = loadedAxes;
        }

        static inline void InitializeTrackers(std::unordered_map<std::string, InputAxis::Tracker>& trackers)
        {
            for (auto const& [key, axis] : axes)
            {
                trackers.emplace(key, InputAxis::Tracker{ axis });
            }
        }

        static inline InputAxis const& GetAxis(std::string const& axisName)
        {
            auto search = axes.find(axisName);
            if (search == axes.end())
                throw std::exception{ (std::string{ "Input Axis not found: " } + axisName).c_str() };
            return search->second;
        }

    private:
        static inline std::unordered_map<std::string, InputAxis> axes;
    };
}