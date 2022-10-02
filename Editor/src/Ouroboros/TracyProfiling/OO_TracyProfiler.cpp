/************************************************************************************//*!
\file           OO_TracyProfiler.cpp
\project        Ouroboros
\author         Lim Guan Hui, l.guanhui, 390009020
\par            email: l.guanhui\@digipen.edu
\date           Sept 30, 2022
\brief          Provides customised profiling options using the Tracy Profiler.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "OO_TracyProfiler.h"
#ifdef TRACY_ENABLE
#include <tracy/client/TracyProfiler.hpp>
#endif
#include <filesystem>

namespace oo
{
    STARTUPINFO OO_TracyProfiler::m_si{};
    PROCESS_INFORMATION OO_TracyProfiler::m_pi{};
    bool OO_TracyProfiler::m_server_active = false;
    bool OO_TracyProfiler::m_server_to_be_closed = false;
    bool OO_TracyProfiler::m_server_to_be_opened = false;
    OO_TracyProfiler::PlotContainer  OO_TracyProfiler::m_dataPlots{};

    void OO_TracyProfiler::StartTracyServer()
    {
#ifdef TRACY_ENABLE
        m_server_to_be_opened = true;
#endif
    }

    void OO_TracyProfiler::CloseTracyServer()
    {
#ifdef TRACY_ENABLE
        m_server_to_be_closed = true;
#endif
    }

    void OO_TracyProfiler::TrackTime(const char* name, PlotContainer::value_type::second_type value)
    {
#ifdef TRACY_ENABLE
        if (OO_TracyProfiler::m_server_active == false)
            return;


        if (m_dataPlots.insert_or_assign(name, value).second == true)
        {
            TRACY_PROFILE_CONFIG(name, TRACY_PERCENTAGE);
        }
#else
        UNREFERENCED(name);
        UNREFERENCED(value);
#endif
    }

    void OO_TracyProfiler::PlotPerformance(const char* name)
    {
#ifdef TRACY_ENABLE
        if (m_dataPlots.find(name) == m_dataPlots.end() || OO_TracyProfiler::m_server_active == false)
            return; 
        auto total_time = m_dataPlots[name];
        for (auto& plotdata : m_dataPlots)
        {
            if (plotdata.first == name)
                continue;
            
            auto percentage = plotdata.second / total_time;
            TRACY_PLOT(plotdata.first, percentage * 100.f);
        }
#else
        UNREFERENCED(name);
#endif 

    }

    void OO_TracyProfiler::PlotPerformanceSelective(std::initializer_list<const char*> list)
    {
#ifdef TRACY_ENABLE
        if (list.size() <= 1)
            return;
        if (m_dataPlots.find(*(list.begin())) == m_dataPlots.end() || OO_TracyProfiler::m_server_active == false)
            return;
        auto total_time = m_dataPlots[*(list.begin())];
        int index = 0;
        for (auto& elements : list)
        {
            if (index == 0 ||
                m_dataPlots.find(elements) == m_dataPlots.end())
            {
                ++index;
                continue;
            }
            auto percentage = m_dataPlots[elements] / total_time;
            TRACY_PLOT(elements, percentage * 100.0);

            /*if (plotdata.first == *(list.begin()))
                continue;

            auto percentage = plotdata.second / total_time;
            TRACY_PLOT(plotdata.first, percentage * 100.f);*/
            ++index;
        }
#else
        UNREFERENCED(list);
#endif 

    }

    void OO_TracyProfiler::CheckIfServerToBeClosed()
    {
#ifdef TRACY_ENABLE
        if (m_server_to_be_closed == false)
            return;
        if (m_server_active == false)
            return;
        m_server_active = false;
        TerminateProcess(m_pi.hProcess, 0);

        CloseHandle(m_pi.hProcess);
        CloseHandle(m_pi.hThread);
        //tracy::ShutdownProfiler();
        m_server_to_be_closed = false;
#endif 
    }

    void OO_TracyProfiler::CheckIfServerToBeOpened()
    {
#ifdef TRACY_ENABLE
        if (m_server_to_be_opened == false)
            return;
        if (m_server_active == true)
            return;
        
        //tracy::StartupProfiler();
        // set the size of the structures
        ZeroMemory(&m_si, sizeof(m_si));
        m_si.cb = sizeof(m_si);
        ZeroMemory(&m_pi, sizeof(m_pi));

        std::wstring command{ L"Tracy.exe -a 127.0.0.1" };
        static LPWSTR arguements{ command.data() };
        static const std::wstring path{ std::filesystem::current_path().wstring() + L"/tracy_server/Tracy.exe" };

        // start the program up
        CreateProcess(path.c_str(),   // the path
            arguements,        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &m_si,            // Pointer to STARTUPINFO structure
            &m_pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
        );
        m_server_active = true;

        m_server_to_be_opened = false;
#endif 
    }

    OO_TracyTracker::OO_TracyTracker(const char* name) :
        m_name{ name },
        m_active{ OO_TracyProfiler::m_server_active}
    {
#ifdef TRACY_ENABLE
        if (OO_TracyProfiler::m_server_active == false)
            return;
        m_start = OO_TracyProfiler::GetTime();
#endif
    }
    OO_TracyTracker::~OO_TracyTracker() 
    {
#ifdef TRACY_ENABLE
        m_end = OO_TracyProfiler::GetTime();
        //m_time_spent = m_end - m_start;

        //OO_TracyProfiler::TrackTime(m_name, m_time_spent);

        //auto end = std::chrono::high_resolution_clock::now();
        /*auto total = std::chrono::duration_cast<std::chrono::milliseconds>(m_end - m_start);

        OO_TracyProfiler::TrackTime(m_name, (float)total.count());*/

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start).time_since_epoch();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_end).time_since_epoch();

        auto duration = end - start;
        auto ms = duration.count() * 0.001;

        OO_TracyProfiler::TrackTime(m_name, ms);
#endif
    }
    OO_Tracy_Zone::OO_Tracy_Zone(const char* name) :
        m_name{name},
        m_active{ OO_TracyProfiler::m_server_active}
    {
#ifdef TRACY_ENABLE
        if (m_active == false)
            return;
        TracyCZoneN(info, m_name, true);
        m_ctx = info;
#endif
    }
    OO_Tracy_Zone::~OO_Tracy_Zone()
    {
#ifdef TRACY_ENABLE
        if (m_active == false )
            return;
        TracyCZoneEnd(m_ctx);
#endif
    }
}