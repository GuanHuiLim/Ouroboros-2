/************************************************************************************//*!
\file           OO_TracyProfiler.h
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 390009020
\par            email: l.guanhui\@digipen.edu
\date           October 6, 2021
\brief          Provides customised profiling options using the Tracy Profiler.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#pragma warning( push )
#pragma warning( disable : 26495 )
#pragma warning( disable : 6201 )
#include <Tracy/Tracy.hpp>
#include <Tracy/TracyC.h>
#pragma warning( pop )
#include <windows.h>
#include <chrono>
#include "Ouroboros/Core/Log.h"
//#include "tracy/Tracy.hpp"

namespace oo
{
    struct OO_TracyTracker;
    class OO_TracyProfiler
    {
        using Duration = double;
        using Time = std::chrono::steady_clock::time_point;
        using PlotContainer = std::unordered_map<const char*, Duration>;
        friend struct OO_TracyTracker;
        static STARTUPINFO m_si;
        static PROCESS_INFORMATION m_pi;
        static PlotContainer m_dataPlots;
    public:
        static bool m_server_active;
        static bool m_server_to_be_closed;
        static bool m_server_to_be_opened;

        static void StartTracyServer();
        static void CloseTracyServer();
        static Time GetTime() { return std::chrono::steady_clock::now(); };
        static void TrackTime(const char* name, PlotContainer::value_type::second_type value);
        static void PlotPerformance(const char* name);
        static void PlotPerformanceSelective(std::initializer_list<const char*> list);
        static void CheckIfServerToBeClosed();
        static void CheckIfServerToBeOpened();
    };


    struct OO_TracyTracker
    {
        OO_TracyProfiler::Duration m_time_spent{};
        OO_TracyProfiler::Time m_start{};
        OO_TracyProfiler::Time m_end{};
        const char* m_name{};
        const bool m_active{};
        OO_TracyTracker() = delete;
        OO_TracyTracker(const char* name);
        ~OO_TracyTracker();


    };

    struct OO_Tracy_Zone
    {
        const char* m_name{};
        const bool m_active{};
        TracyCZoneCtx m_ctx{};
        OO_Tracy_Zone(const char* name);
        ~OO_Tracy_Zone();

        inline static std::stack<TracyCZoneCtx> m_ctxStack{};
        static void ProfileZone(TracyCZoneCtx ctx)
        {
            m_ctxStack.emplace(ctx);
        }
        static void ProfileZoneEnd()
        {
            TracyCZoneEnd(m_ctxStack.top());
            m_ctxStack.pop();
        }
    };

    
}

//toggle macro to disable tracy related functionality when TRACY_ENABLE is not defined
#ifdef TRACY_ENABLE
#define TRACY_TOGGLE(var) var
#else
#define TRACY_TOGGLE(var)
#endif
/**
 * profiles a scope.
 */
#define TRACY_PROFILE_FUNCTION TRACY_TOGGLE(ZoneScoped)
#define TRACY_PROFILE_FUNCTION_NAMED TRACY_TOGGLE(ZoneScoped())
/**
 * profiles a scope using RAII to create an object that tracks from when it is created till the end of its lifetime.
 * name provided is to be manually given without any strings as such TRACY_PROFILE_SCOPE(input_name_here)
 */
//#define TRACY_PROFILE_SCOPE(name) TRACY_TOGGLE(oo::OO_Tracy_Zone OO_tracy_##name(name))
#define TRACY_PROFILE_SCOPE(name) TRACY_TOGGLE(if (oo::OO_TracyProfiler::m_server_active){TracyCZoneN(info,name,true); oo::OO_Tracy_Zone::ProfileZone(info);})
#define TRACY_PROFILE_SCOPE_N(name) TRACY_PROFILE_SCOPE(name)
#define TRACY_PROFILE_SCOPE_NC(name, color) TRACY_TOGGLE(if (oo::OO_TracyProfiler::m_server_active){TracyCZoneNC(info,name,color, true); oo::OO_Tracy_Zone::ProfileZone(info);})
#define TRACY_PROFILE_SCOPE_END() TRACY_TOGGLE(if (oo::OO_TracyProfiler::m_server_active) oo::OO_Tracy_Zone::ProfileZoneEnd();)
/** ************************************************** */
//tracking of frames
/** ************************************************** */
/**
 * profile end of a repeating loop. it is given a default name internally by tracy api
 */
#define TRACY_PROFILE_END_OF_LOOP TRACY_TOGGLE(FrameMark)
/**
 * profile end of a repeating loop. typically used for update loops. name is required to be given to represent this.
 * name provided needs to be a const char*
 */
#define TRACY_PROFILE_END_OF_FRAME() TRACY_TOGGLE(if (oo::OO_TracyProfiler::m_server_active)FrameMark;)
//#define TRACY_PROFILE_END_OF_FRAME(name) TRACY_TOGGLE(if (oo::OO_TracyProfiler::m_server_active)FrameMarkNamed(name);)
//discontinuous frames
#define TRACY_PROFILE_FRAME_START(name) TRACY_TOGGLE(FrameMarkStart(name))
#define TRACY_PROFILE_FRAME_END(name) TRACY_TOGGLE(FrameMarkEnd(name))
//set plotting configurations
#define TRACY_PERCENTAGE TRACY_TOGGLE(tracy::PlotFormatType::Percentage)
#define TRACY_NUMBER TRACY_TOGGLE(tracy::PlotFormatType::Number)
#define TRACY_MEMORY TRACY_TOGGLE(tracy::PlotFormatType::Memory)
#define TRACY_PROFILE_CONFIG(name, type) TRACY_TOGGLE(TracyPlotConfig(name,type))
#define TRACY_PLOT(name, value) TRACY_TOGGLE(TracyPlot(name,value))
//tracy helper functions
#define TRACY_GET_TIME TRACY_TOGGLE(std::chrono::high_resolution_clock::now())
#define TRACY_TRACK_PERFORMANCE(name) TRACY_TOGGLE(oo::OO_TracyTracker profile_##name{name};)
#define TRACY_DISPLAY_PERFORMANCE(name) TRACY_TOGGLE(oo::OO_TracyProfiler::PlotPerformance (name);)
#define TRACY_DISPLAY_PERFORMANCE_SELECTED(...) TRACY_TOGGLE(oo::OO_TracyProfiler::PlotPerformanceSelective ({__VA_ARGS__});)