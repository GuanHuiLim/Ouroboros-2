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
#define ENGINE_ASSERT(x)                        { if(!(!(x))) {LOG_ENGINE_DEBUG_CRITICAL("Assertion Failed!");oo::Log::ShutdownDebugLogger(); __debugbreak();} }
#define ENGINE_ASSERT_MSG(x, ...)               { if(!(!(x))) {LOG_ENGINE_DEBUG_CRITICAL("Assertion Failed : {0}", __VA_ARGS__);oo::Log::ShutdownDebugLogger(); __debugbreak();} }
#define ENGINE_ASSERT_CUSTOM_MSG(x, msg, ...)   { if(!(!(x))) {LOG_ENGINE_DEBUG_CRITICAL("Assertion Failed! " + std::string(msg), __VA_ARGS__);oo::Log::ShutdownDebugLogger(); __debugbreak();} }

#define ENGINE_VERIFY(x)                        { if(!(!(x))) {LOG_ENGINE_DEBUG_CRITICAL("Verification Failed!");oo::Log::ShutdownDebugLogger(); __debugbreak();} }
#define ENGINE_VERIFY_MSG(x, ...)               { if(!(!(x))) {LOG_ENGINE_DEBUG_CRITICAL("Verification Failed : {0}", __VA_ARGS__);oo::Log::ShutdownDebugLogger(); __debugbreak();} }
#define ENGINE_VERIFY_CUSTOM_MSG(x, msg, ...)   { if(!(!(x))) {LOG_ENGINE_DEBUG_CRITICAL("Verification Failed! " + std::string(msg), __VA_ARGS__);oo::Log::ShutdownDebugLogger(); __debugbreak();} }

#else

#define ENGINE_ASSERT(x)                        
#define ENGINE_ASSERT_MSG(x, ...)               
#define ENGINE_ASSERT_CUSTOM_MSG(x, msg, ...)   
#define ENGINE_VERIFY(x)                        x
#define ENGINE_VERIFY_MSG(x, ...)               x
#define ENGINE_VERIFY_CUSTOM_MSG(x, msg, ...)   x

#endif