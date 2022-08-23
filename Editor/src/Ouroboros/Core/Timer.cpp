/************************************************************************************//*!
\file           Timer.cpp
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 24, 2022
\brief          Defines the structures and functions relevant to time that is used
                throughout the project.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "pch.h"
#include "Timer.h"

namespace oo
{
    namespace timer
    {
        double s_timescale;

        double s_debugTrackingDuration;
        double s_lower_limit = 0.f, s_upper_limit = 1.0f;

        std::chrono::steady_clock::time_point s_timeStart;
        double s_raw_dt, s_controlled_dt;

        std::vector<TimeDebugInfo> s_debugInfo;
        std::vector<double> s_deltas, s_controlled_deltas, s_raw_deltas;
        double s_frequency;

        //private
        void set_dt(double value)
        {
            s_controlled_dt = s_raw_dt = value;
            s_controlled_dt = std::clamp(s_controlled_dt, s_lower_limit, s_upper_limit);

            if (s_frequency > 0.0)
            {
                s_raw_deltas.emplace_back(s_raw_dt);
                s_controlled_deltas.emplace_back(s_controlled_dt);
                s_frequency -= s_raw_dt;
            }
            else
            {
                timer::TimeDebugInfo info;

                double totalDelta = 0.0;
                for (auto& timesteps : s_deltas)
                {
                    totalDelta += timesteps;
                }

                double totalUnScaledDelta = 0.0;
                for (auto& timesteps : s_controlled_deltas)
                {
                    totalUnScaledDelta += timesteps;
                }

                double totalRawDelta = 0.0;
                for (auto& timesteps : s_raw_deltas)
                {
                    totalRawDelta += timesteps;
                }

                info.AvgDeltaTime = totalDelta / s_deltas.size();
                info.AvgFPS = 1.0 / info.AvgDeltaTime;

                info.AvgUnscaledDeltaTime = totalUnScaledDelta / s_controlled_deltas.size();
                info.AvgUnscaledFPS = 1.0 / info.AvgUnscaledDeltaTime;

                info.AvgRawDeltaTime = totalRawDelta / s_raw_deltas.size();
                info.AvgRawFPS = 1.0 / info.AvgDeltaTime;

                info.CurrentTimeScale = s_timescale;
                info.TimeElapsed = program_elapsed();

                s_deltas.clear();
                s_raw_deltas.clear();
                s_controlled_deltas.clear();

                s_frequency = s_debugTrackingDuration;
            }
        }

        Timestep::Timestep()
            : m_startTime(std::chrono::steady_clock::now())
        {
        }

        Timestep::~Timestep()
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto deltaTime = std::chrono::duration<double, std::chrono::seconds::period>(endTime - m_startTime).count();
            set_dt(deltaTime);
        }

        void init()
        {
            s_debugInfo.reserve(500);
            s_deltas.reserve(500);
            s_controlled_deltas.reserve(500);
            s_raw_deltas.reserve(500);

            reset();
        }

        void terminate()
        {
            auto program_lifetime = program_elapsed();
            UNREFERENCED(program_lifetime);
            reset();
        }

        void reset()
        {
            s_timeStart = std::chrono::high_resolution_clock::now();
            s_raw_dt = 0.0;
            s_timescale = 1.0;

            s_debugInfo.clear();
            s_deltas.clear();
            s_controlled_deltas.clear();
            s_raw_deltas.clear();
            s_frequency = 0.0;
        }

        double program_elapsed()
        {
            auto endTime = std::chrono::high_resolution_clock::now();
            auto program_delta = std::chrono::duration<double, std::chrono::seconds::period>(endTime - s_timeStart).count();
            return program_delta;
        }


        float dt() { return static_cast<float>(s_controlled_dt * s_timescale); }

        float fps() { return static_cast<float>(1.0 / dt()); }

        double dt_precise() { return s_controlled_dt * s_timescale; }

        double fps_precise() { return 1.0 / dt_precise(); }

        float unscaled_dt() { return static_cast<float>(s_controlled_dt); }

        float unscaled_fps() { return static_cast<float>(1.0 / unscaled_dt()); }

        double unscaled_dt_precise() { return s_controlled_dt; }

        double unscaled_fps_precise() { return 1.0 / unscaled_dt_precise(); }

        float raw_dt() { return static_cast<float>(s_raw_dt); }

        float raw_fps() { return static_cast<float>(1.0 / dt()); }

        double raw_dt_precise() { return s_raw_dt; }

        double raw_fps_precise() { return 1.0 / dt_precise(); }

        float get_timescale()
        {
            return static_cast<float>(s_timescale);
        }

        double get_timescale_precise()
        {
            return s_timescale;
        }
        
        void set_timescale(double newTimeScale)
        {
            s_timescale = std::clamp(newTimeScale, 0.0, newTimeScale);
        }
    }
}
