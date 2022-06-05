/************************************************************************************//*!
\file           log.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 10, 2022
\brief          Core Logging library using external library spdlog.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Log.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#pragma warning(pop)

#include "LogCallbackSink.h"

#include <filesystem>

namespace oo
{
    namespace log
    {
        //static defines
        static std::shared_ptr<spdlog::logger> s_coreLogger;
        static std::shared_ptr<spdlog::logger> s_clientLogger;
//#ifndef OO_EXECUTABLE
        static std::shared_ptr<spdlog::logger> s_debuggerLogger;
//#endif
        void init()
        {
            // Create logging directory if it doesn't exist.
            std::string logsDirectory = "logs";
            if (!std::filesystem::exists(logsDirectory))
                std::filesystem::create_directories(logsDirectory);

            std::vector<spdlog::sink_ptr> coreSinks =
            {
                // Creates a multi-threaded, Colored std out console
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
                std::make_shared<CallbackSink_mt>(),
                std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Engine.log", true)
            };

            std::vector<spdlog::sink_ptr> clientSinks =
            {
                // Creates a multi-threaded, Colored std out console
                std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
                std::make_shared<CallbackSink_mt>(),
                std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Client.log", true)
            };

            std::vector<spdlog::sink_ptr> debugLoggerSinks =
            {
                std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Debug.log", true)
            };

            // %^   == Color
            // [%T] == Timestamp
            // [%l] == Message Level
            // %n   == Name of Logger
            // %v%$ == Log Message
            coreSinks[0]->set_pattern("%^ [%T] %n: %v%$");
            //coreSinks[1]->set_pattern("[%T] [%l] %n: %v");

            clientSinks[0]->set_pattern("%^ [%T] %n: %v%$");
            //clientSinks[1]->set_pattern("[%T] [%l] %n: %v");

            debugLoggerSinks[0]->set_pattern("[%T] File:[%@] Function:[%!] : %v");

            // Create core logger
            s_coreLogger = std::make_shared<spdlog::logger>("ENGINE", coreSinks.begin(), coreSinks.end());
            // Set logging level to trace : lowest level therefore traces everything
            s_coreLogger->set_level(spdlog::level::trace);

            // Create client logger
            s_clientLogger = std::make_shared<spdlog::logger>("CLIENT", clientSinks.begin(), clientSinks.end());
            // Set logging level to trace : lowest level therefore traces everything
            s_clientLogger->set_level(spdlog::level::trace);

            // Create debug logger
            s_debuggerLogger = std::make_shared<spdlog::logger>("DEBUG", debugLoggerSinks.begin(), debugLoggerSinks.end());
            // Set logging level to trace : lowest level therefore traces everything
            s_debuggerLogger->set_level(spdlog::level::trace);
            s_debuggerLogger->enable_backtrace(512);
        }

        std::shared_ptr<spdlog::logger>& GetCoreLogger()
        {
            // TODO: insert return statement here
            return s_coreLogger;
        }

        std::shared_ptr<spdlog::logger>& GetClientLogger()
        {
            // TODO: insert return statement here
            return s_clientLogger;
        }

        std::shared_ptr<spdlog::logger>& GetDebugLogger()
        {
            return s_debuggerLogger;
        }

        void shutdown()
        {
            s_coreLogger.reset();
            s_clientLogger.reset();
            
            ShutdownDebugLogger();

            spdlog::drop_all();
        }

        void ShutdownDebugLogger()
        {
            s_debuggerLogger->dump_backtrace();
            s_debuggerLogger.reset();
        }

    }
    
}