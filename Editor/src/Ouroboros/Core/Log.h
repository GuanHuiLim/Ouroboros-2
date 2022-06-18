/************************************************************************************//*!
\file           log.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 10, 2022
\brief          Core Logging library using external library spdlog.

\note           using spdlog causes some warnings : they are currently being suppressed

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#if not defined(SPDLOG_ACTIVE_LEVEL) 
#define SPDLOG_ACTIVE_LEVEL 0 
#endif 
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace oo
{
    /********************************************************************************//*!
     @brief     Core Logging library using external library spdlog.
    *//*********************************************************************************/
    namespace log
    {
        /********************************************************************************//*!
         @brief     Initialization of logging library. Performed in main
        *//*********************************************************************************/
        void init();

        /********************************************************************************//*!
         @brief     Releasing of logging library. Performed in main
        *//*********************************************************************************/
        void shutdown();

        void ShutdownDebugLogger();

        /********************************************************************************//*!
         @brief     Retrieve the core logger used to store logging information of
                    Game Engine
        *//*********************************************************************************/
        std::shared_ptr<spdlog::logger>& GetCoreLogger();

        /********************************************************************************//*!
         @brief     Retrieve the client logger used to store logging information of
                    Client Application
        *//*********************************************************************************/
        std::shared_ptr<spdlog::logger>& GetClientLogger();

        /********************************************************************************//*!
         @brief     Retrieve the debug logger used to store logging information of
            Client Application
        *//*********************************************************************************/
        std::shared_ptr<spdlog::logger>& GetDebugLogger();

    }

}


/****************************************************************************//*!
 @brief     Disables all logging information if not in debug mode
*//*****************************************************************************/

// Engine Log macros
#define LOG_CORE_TRACE(...)                 SPDLOG_LOGGER_TRACE(::oo::log::GetCoreLogger(),__VA_ARGS__)
#define LOG_CORE_INFO(...)                  SPDLOG_LOGGER_INFO(::oo::log::GetCoreLogger(),__VA_ARGS__)
#define LOG_CORE_WARN(...)                  SPDLOG_LOGGER_WARN(::oo::log::GetCoreLogger(),__VA_ARGS__)
#define LOG_CORE_ERROR(...)                 SPDLOG_LOGGER_ERROR(::oo::log::GetCoreLogger(),__VA_ARGS__)
#define LOG_CORE_CRITICAL(...)              SPDLOG_LOGGER_CRITICAL(::oo::log::GetCoreLogger(),__VA_ARGS__)

// Client Log macros
#define LOG_TRACE(...)                      SPDLOG_LOGGER_TRACE(::oo::log::GetClientLogger(),__VA_ARGS__)
#define LOG_INFO(...)                       SPDLOG_LOGGER_INFO(::oo::log::GetClientLogger(),__VA_ARGS__)
#define LOG_WARN(...)                       SPDLOG_LOGGER_WARN(::oo::log::GetClientLogger(),__VA_ARGS__)
#define LOG_ERROR(...)                      SPDLOG_LOGGER_ERROR(::oo::log::GetClientLogger(),__VA_ARGS__)
#define LOG_CRITICAL(...)                   SPDLOG_LOGGER_CRITICAL(::oo::log::GetClientLogger(),__VA_ARGS__)

#ifndef OO_EXECUTABLE

//Critical Logs are reserved for assert
#define LOG_CORE_DEBUG_INFO(...)            SPDLOG_LOGGER_INFO(::oo::log::GetDebugLogger(),__VA_ARGS__)
#define LOG_CORE_DEBUG_CRITICAL(...)        SPDLOG_LOGGER_CRITICAL(::oo::log::GetDebugLogger(),__VA_ARGS__)

#else

//Critical Logs are reserved for assert
#define LOG_CORE_DEBUG_INFO(...)          
#define LOG_CORE_DEBUG_CRITICAL(...)      

#endif