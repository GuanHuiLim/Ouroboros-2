/************************************************************************************//*!
\file           Assert.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 10, 2022
\brief          Asserts that are used in the engine.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Core/Log.h"

/****************************************************************************//*!
 @brief     Windows only, visual studios only debug definition.
*//*****************************************************************************/
#if not defined (ASSERTS_DISABLED) //OO_DEBUG || OO_RELEASE

/****************************************************************************//*!
 @brief     Implements the functionality when to test asserts
            when engine is in debug mode.

 @note      Asserts are completely stripped upon not in debug
            whereas verify still calls the original function (x).
            Use at own discretion.
*//*****************************************************************************/
#define ASSERT(x)                        { if(!(!(x))) {LOG_CORE_DEBUG_CRITICAL("Assertion Failed!"); oo::log::ShutdownDebugLogger(); __debugbreak();} }
#define ASSERT_MSG(x, ...)               { if(!(!(x))) {LOG_CORE_DEBUG_CRITICAL("Assertion Failed : {0}", __VA_ARGS__); oo::log::ShutdownDebugLogger(); __debugbreak();} }
#define ASSERT_CUSTOM_MSG(x, msg, ...)   { if(!(!(x))) {LOG_CORE_DEBUG_CRITICAL("Assertion Failed! " + std::string(msg), __VA_ARGS__); oo::log::ShutdownDebugLogger(); __debugbreak();} }

#define VERIFY(x)                        { if(!(!(x))) {LOG_CORE_DEBUG_CRITICAL("Verification Failed!"); oo::log::ShutdownDebugLogger(); __debugbreak();} }
#define VERIFY_MSG(x, ...)               { if(!(!(x))) {LOG_CORE_DEBUG_CRITICAL("Verification Failed : {0}", __VA_ARGS__); oo::log::ShutdownDebugLogger(); __debugbreak();} }
#define VERIFY_CUSTOM_MSG(x, msg, ...)   { if(!(!(x))) {LOG_CORE_DEBUG_CRITICAL("Verification Failed! " + std::string(msg), __VA_ARGS__); oo::log::ShutdownDebugLogger(); __debugbreak();} }

#else

#define ASSERT(x)                        
#define ASSERT_MSG(x, ...)               
#define ASSERT_CUSTOM_MSG(x, msg, ...)   

#define VERIFY(x)                        x
#define VERIFY_MSG(x, ...)               x
#define VERIFY_CUSTOM_MSG(x, msg, ...)   x

#endif