#pragma once

#include <chrono>

#include <vector>

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
    Timestep()
        : m_startTime(std::chrono::steady_clock::now())
    {
    }

    ~Timestep()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto deltaTime = std::chrono::duration<double, std::chrono::seconds::period>(endTime - m_startTime).count();
        Time::SetDT(deltaTime);
    }

private:
    std::chrono::steady_clock::time_point m_startTime;
};

class Time
{
public:
    static void init()
    {
        s_debugInfo.reserve(500);
        s_deltas.reserve(500);
        s_controlled_deltas.reserve(500);
        s_raw_deltas.reserve(500);

        reset();
    }

    static void terminate()
    {
        auto program_lifetime = program_elapsed();
        
        reset();
    }

    static void reset()
    {
        s_timeStart = std::chrono::high_resolution_clock::now();
        s_raw_dt    = 0.0;
        s_timescale = 1.0;

        s_debugInfo.clear();
        s_deltas.clear();
        s_controlled_deltas.clear();
        s_raw_deltas.clear();
        s_frequency = 0.0;
    }

    static double program_elapsed()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto program_delta = std::chrono::duration<double, std::chrono::seconds::period>(endTime - s_timeStart).count();
        return program_delta;
    }

    //private
    static void SetDT(double value)
    {
        s_controlled_dt = s_raw_dt = value;
        std::clamp(s_controlled_dt, s_lower_limit, s_upper_limit);
        
        if (s_frequency > 0.0)
        {
            s_raw_deltas.emplace_back(s_raw_dt);
            s_controlled_deltas.emplace_back(s_controlled_dt);
            s_frequency -= s_raw_dt;
        }
        else
        {
            TimeDebugInfo info;

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

    static float  DeltaTime()                   { return static_cast<float>(s_controlled_dt * s_timescale); }
    static float  FPS()                         { return static_cast<float>(1.0 / DeltaTime()); }
    static double DeltaTime_Precise()           { return s_controlled_dt * s_timescale; }
    static double FPS_Precise()                 { return 1.0 / DeltaTime_Precise(); }

    static float  UnscaledDeltaTime()           { return static_cast<float>(s_controlled_dt); }
    static float  UnscaledFPS()                 { return static_cast<float>(1.0 / UnscaledDeltaTime()); }
    static double UnscaledDeltaTime_Precise()   { return s_controlled_dt; }
    static double UnscaledFPS_Precise()         { return 1.0 / UnscaledDeltaTime_Precise(); }

    static float  RawDeltaTime()                { return static_cast<float>(s_raw_dt); }
    static float  RawFPS()                      { return static_cast<float>(1.0 / DeltaTime()); }
    static double RawDeltaTime_Precise()        { return s_raw_dt; }
    static double RawFPS_Precise()              { return 1.0 / DeltaTime_Precise(); }

    static double s_timescale;
    
    static double s_debugTrackingDuration;
    static double s_lower_limit, s_upper_limit;

private:
    static std::chrono::steady_clock::time_point s_timeStart;
    static double s_raw_dt, s_controlled_dt;

    static std::vector<TimeDebugInfo> s_debugInfo;
    static std::vector<double> s_deltas, s_controlled_deltas, s_raw_deltas;
    static double s_frequency;
};
