/************************************************************************************//*!
\file           Timer.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 24, 2022
\brief          Declares the structures and functions relevant to time that is used
                throughout the project.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <chrono>
#include <vector>

namespace oo 
{
    namespace timer
    {
        struct TimeDebugInfo
        {
            double AvgFPS;
            double AvgDeltaTime;

            double AvgUnscaledFPS;
            double AvgUnscaledDeltaTime;

            double AvgRawFPS;
            double AvgRawDeltaTime;

            double CurrentTimeScale;
            double TimeElapsed;
        };

        class Timestep
        {
        public:
            Timestep();
            ~Timestep();

        private:
            std::chrono::steady_clock::time_point m_startTime;
        };

        void init();
        void terminate();
        void reset();

        double program_elapsed();

        float  dt();
        float  fps();
        double dt_precise();
        double fps_precise();

        float  unscaled_dt();
        float  unscaled_fps();
        double unscaled_dt_precise();
        double unscaled_fps_precise();

        float  raw_dt();
        float  raw_fps();
        double raw_dt_precise();
        double raw_fps_precise();
        

        float  get_timescale();
        double get_timescale_precise();
        void   set_timescale(double newTimeScale);
    }
}