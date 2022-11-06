/************************************************************************************//*!
\file           KeyCode.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 24, 2022
\brief          Defines the engine keycodes that are used the input system to map each
                key to a behaviour regardless of what the backend is using.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <ostream>

namespace oo
{
    namespace input
    {
        typedef enum class KeyCode
        {
            A = 4,
            B = 5,
            C = 6,
            D = 7,
            E = 8,
            F = 9,
            G = 10,
            H = 11,
            I = 12,
            J = 13,
            K = 14,
            L = 15,
            M = 16,
            N = 17,
            O = 18,
            P = 19,
            Q = 20,
            R = 21,
            S = 22,
            T = 23,
            U = 24,
            V = 25,
            W = 26,
            X = 27,
            Y = 28,
            Z = 29,

            D1 = 30,
            D2 = 31,
            D3 = 32,
            D4 = 33,
            D5 = 34,
            D6 = 35,
            D7 = 36,
            D8 = 37,
            D9 = 38,
            D0 = 39,

            ENTER = 40,
            ESCAPE = 41,
            BACKSPACE = 42,
            TAB = 43,
            SPACE = 44,

            MINUS = 45,
            EQUALS = 46,
            LEFTBRACKET = 47,
            RIGHTBRACKET = 48,
            BACKSLASH = 49,       /**< Located at the lower left of the return
                                        *   key on ISO keyboards and at the right end
                                        *   of the QWERTY row on ANSI keyboards.
                                        *   Produces REVERSE SOLIDUS (backslash) and
                                        *   VERTICAL LINE in a US layout, REVERSE
                                        *   SOLIDUS and VERTICAL LINE in a UK Mac
                                        *   layout, NUMBER SIGN and TILDE in a UK
                                        *   Windows layout, DOLLAR SIGN and POUND SIGN
                                        *   in a Swiss German layout, NUMBER SIGN and
                                        *   APOSTROPHE in a German layout, GRAVE
                                        *   ACCENT and POUND SIGN in a French Mac
                                        *   layout, and ASTERISK and MICRO SIGN in a
                                        *   French Windows layout.
                                        */
            NONUSHASH = 50,       /**< ISO USB keyboards actually use this code
                                        *   instead of 49 for the same key, but all
                                        *   OSes I've seen treat the two codes
                                        *   identically. So, as an implementor, unless
                                        *   your keyboard generates both of those
                                        *   codes and your OS treats them differently,
                                        *   you should generate SDL_SCANCODE_BACKSLASH
                                        *   instead of this code. As a user, you
                                        *   should not rely on this code because SDL
                                        *   will never generate it with most (all?)
                                        *   keyboards.
                                        */
            SEMICOLON = 51,
            APOSTROPHE = 52,
            GRAVE = 53,       /**< Located in the top left corner (on both ANSI
                                        *   and ISO keyboards). Produces GRAVE ACCENT and
                                        *   TILDE in a US Windows layout and in US and UK
                                        *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                                        *   and NOT SIGN in a UK Windows layout, SECTION
                                        *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                                        *   layouts on ISO keyboards, SECTION SIGN and
                                        *   DEGREE SIGN in a Swiss German layout (Mac:
                                        *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                                        *   DEGREE SIGN in a German layout (Mac: only on
                                        *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                                        *   French Windows layout, COMMERCIAL AT and
                                        *   NUMBER SIGN in a French Mac layout on ISO
                                        *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                                        *   SIGN in a Swiss German, German, or French Mac
                                        *   layout on ANSI keyboards.
                                        */
            COMMA = 54,
            PERIOD = 55,
            SLASH = 56,

            CAPSLOCK = 57,

            F1 = 58,
            F2 = 59,
            F3 = 60,
            F4 = 61,
            F5 = 62,
            F6 = 63,
            F7 = 64,
            F8 = 65,
            F9 = 66,
            F10 = 67,
            F11 = 68,
            F12 = 69,

            PRINTSCREEN = 70,
            SCROLLLOCK = 71,
            PAUSE = 72,
            INSERT = 73, /**< insert on PC, help on some Mac keyboards (but does send code 73, not 117) */
            HOME = 74,
            PAGEUP = 75,
            DEL = 76,/* DELETE <- writing this will conflict with some header defines*/
            END = 77,
            PAGEDOWN = 78,
            RIGHT = 79,
            LEFT = 80,
            DOWN = 81,
            UP = 82,

            NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards*/
            KP_DIVIDE = 84,
            KP_MULTIPLY = 85,
            KP_MINUS = 86,
            KP_PLUS = 87,
            KP_ENTER = 88,
            KP_1 = 89,
            KP_2 = 90,
            KP_3 = 91,
            KP_4 = 92,
            KP_5 = 93,
            KP_6 = 94,
            KP_7 = 95,
            KP_8 = 96,
            KP_9 = 97,
            KP_0 = 98,
            KP_PERIOD = 99,

            LCTRL = 224,
            LSHIFT = 225,
            LALT = 226, /**< alt, option */
            LGUI = 227, /**< windows, command (apple), meta */
            RCTRL = 228,
            RSHIFT = 229,
            RALT = 230, /**< alt gr, option */
            RGUI = 231, /**< windows, command (apple), meta */
        } Key;

        inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
        {
            os << static_cast<int>(keyCode);
            return os;
        }

        inline KeyCode& operator++(KeyCode& keyCode)
        {
            keyCode = static_cast<KeyCode>(static_cast<int>(keyCode) + 1);
            return keyCode;
        }

        inline bool operator<(KeyCode keyCode, int val)
        {
            return static_cast<int>(keyCode) < val;
        }

    }
}

// From SDL keycodes

#define KEY_SPACE           ::oo::input::Key::SPACE
#define KEY_APOSTROPHE      ::oo::input::Key::APOSTROPHE    /* ' */
#define KEY_COMMA           ::oo::input::Key::COMMA         /* , */
#define KEY_MINUS           ::oo::input::Key::MINUS         /* - */
#define KEY_PLUS            ::oo::input::Key::EQUALS        /* + */
#define KEY_PERIOD          ::oo::input::Key::PERIOD        /* . */
#define KEY_SLASH           ::oo::input::Key::SLASH         /* / */
#define KEY_0               ::oo::input::Key::D0
#define KEY_1               ::oo::input::Key::D1
#define KEY_2               ::oo::input::Key::D2
#define KEY_3               ::oo::input::Key::D3
#define KEY_4               ::oo::input::Key::D4
#define KEY_5               ::oo::input::Key::D5
#define KEY_6               ::oo::input::Key::D6
#define KEY_7               ::oo::input::Key::D7
#define KEY_8               ::oo::input::Key::D8
#define KEY_9               ::oo::input::Key::D9
#define KEY_SEMICOLON       ::oo::input::Key::SEMICOLON     /* ; */
#define KEY_EQUAL           ::oo::input::Key::EQUALS        /* = */
#define KEY_A               ::oo::input::Key::A
#define KEY_B               ::oo::input::Key::B
#define KEY_C               ::oo::input::Key::C
#define KEY_D               ::oo::input::Key::D
#define KEY_E               ::oo::input::Key::E
#define KEY_F               ::oo::input::Key::F
#define KEY_G               ::oo::input::Key::G
#define KEY_H               ::oo::input::Key::H
#define KEY_I               ::oo::input::Key::I
#define KEY_J               ::oo::input::Key::J
#define KEY_K               ::oo::input::Key::K
#define KEY_L               ::oo::input::Key::L
#define KEY_M               ::oo::input::Key::M
#define KEY_N               ::oo::input::Key::N
#define KEY_O               ::oo::input::Key::O
#define KEY_P               ::oo::input::Key::P
#define KEY_Q               ::oo::input::Key::Q
#define KEY_R               ::oo::input::Key::R
#define KEY_S               ::oo::input::Key::S
#define KEY_T               ::oo::input::Key::T
#define KEY_U               ::oo::input::Key::U
#define KEY_V               ::oo::input::Key::V
#define KEY_W               ::oo::input::Key::W
#define KEY_X               ::oo::input::Key::X
#define KEY_Y               ::oo::input::Key::Y
#define KEY_Z               ::oo::input::Key::Z
#define KEY_LEFT_BRACKET    ::oo::input::Key::LEFTBRACKET   /* [ */
#define KEY_BACKSLASH       ::oo::input::Key::BACKSLASH     /* \ */
#define KEY_RIGHT_BRACKET   ::oo::input::Key::RIGHTBRACKET  /* ] */
#define KEY_GRAVE_ACCENT    ::oo::input::Key::GRAVE         /* ` */
//#define KEY_WORLD_1         ::oo::input::Key::WORLD1        /* non-US #1 */
//#define KEY_WORLD_2         ::oo::input::Key::WORLD2        /* non-US #2 */

/* Function keys */
#define KEY_ESCAPE          ::oo::input::Key::ESCAPE
#define KEY_ENTER           ::oo::input::Key::ENTER
#define KEY_TAB             ::oo::input::Key::TAB
#define KEY_BACKSPACE       ::oo::input::Key::BACKSPACE
#define KEY_INSERT          ::oo::input::Key::INSERT
#define KEY_DELETE          ::oo::input::Key::DEL
#define KEY_RIGHT           ::oo::input::Key::RIGHT
#define KEY_LEFT            ::oo::input::Key::LEFT
#define KEY_DOWN            ::oo::input::Key::DOWN
#define KEY_UP              ::oo::input::Key::UP
#define KEY_PAGE_UP         ::oo::input::Key::PAGEUP
#define KEY_PAGE_DOWN       ::oo::input::Key::PAGEDOWN
#define KEY_HOME            ::oo::input::Key::HOME
#define KEY_END             ::oo::input::Key::END
#define KEY_CAPS_LOCK       ::oo::input::Key::CAPSLOCK
#define KEY_SCROLL_LOCK     ::oo::input::Key::SCROLLLOCK
#define KEY_NUM_LOCK        ::oo::input::Key::NUMLOCKCLEAR
#define KEY_PRINT_SCREEN    ::oo::input::Key::PRINTSCREEN
#define KEY_PAUSE           ::oo::input::Key::PAUSE
#define KEY_F1              ::oo::input::Key::F1
#define KEY_F2              ::oo::input::Key::F2
#define KEY_F3              ::oo::input::Key::F3
#define KEY_F4              ::oo::input::Key::F4
#define KEY_F5              ::oo::input::Key::F5
#define KEY_F6              ::oo::input::Key::F6
#define KEY_F7              ::oo::input::Key::F7
#define KEY_F8              ::oo::input::Key::F8
#define KEY_F9              ::oo::input::Key::F9
#define KEY_F10             ::oo::input::Key::F10
#define KEY_F11             ::oo::input::Key::F11
#define KEY_F12             ::oo::input::Key::F12
//#define KEY_F13             ::oo::input::Key::F13
//#define KEY_F14             ::oo::input::Key::F14
//#define KEY_F15             ::oo::input::Key::F15
//#define KEY_F16             ::oo::input::Key::F16
//#define KEY_F17             ::oo::input::Key::F17
//#define KEY_F18             ::oo::input::Key::F18
//#define KEY_F19             ::oo::input::Key::F19
//#define KEY_F20             ::oo::input::Key::F20
//#define KEY_F21             ::oo::input::Key::F21
//#define KEY_F22             ::oo::input::Key::F22
//#define KEY_F23             ::oo::input::Key::F23
//#define KEY_F24             ::oo::input::Key::F24
//#define KEY_F25             ::oo::input::Key::F25

/* Keypad */
#define KEY_KP_0            ::oo::input::Key::KP_0
#define KEY_KP_1            ::oo::input::Key::KP_1
#define KEY_KP_2            ::oo::input::Key::KP_2
#define KEY_KP_3            ::oo::input::Key::KP_3
#define KEY_KP_4            ::oo::input::Key::KP_4
#define KEY_KP_5            ::oo::input::Key::KP_5
#define KEY_KP_6            ::oo::input::Key::KP_6
#define KEY_KP_7            ::oo::input::Key::KP_7
#define KEY_KP_8            ::oo::input::Key::KP_8
#define KEY_KP_9            ::oo::input::Key::KP_9
#define KEY_KP_PERIOD       ::oo::input::Key::KP_PERIOD
#define KEY_KP_DIVIDE       ::oo::input::Key::KP_DIVIDE
#define KEY_KP_MULTIPLY     ::oo::input::Key::KP_MULTIPLY
#define KEY_KP_MINUS        ::oo::input::Key::KP_MINUS
#define KEY_KP_PLUS         ::oo::input::Key::KP_PLUS
#define KEY_KP_ENTER        ::oo::input::Key::KP_ENTER
//#define KEY_KP_EQUAL        ::oo::input::Key::KP_EQUALS

#define KEY_LEFT_SHIFT      ::oo::input::Key::LSHIFT
#define KEY_LEFT_CONTROL    ::oo::input::Key::LCTRL
#define KEY_LEFT_ALT        ::oo::input::Key::LALT
#define KEY_LEFT_SUPER      ::oo::input::Key::LGUI
#define KEY_RIGHT_SHIFT     ::oo::input::Key::RSHIFT
#define KEY_RIGHT_CONTROL   ::oo::input::Key::RCTRL
#define KEY_RIGHT_ALT       ::oo::input::Key::RALT
#define KEY_RIGHT_SUPER     ::oo::input::Key::RGUI
//#define KEY_MENU            ::oo::input::Key::MENU
