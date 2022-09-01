#pragma once

// Note: This header file only wraps the C++ Macros needed for external profiling tools.

//#define USE_PROFILING_OPTICK

#if defined(USE_PROFILING_OPTICK)
    #include "optick/optick.h"
    #define PROFILE_SCOPED(...)              OPTICK_EVENT(__VA_ARGS__);
    #define PROFILE_FRAME(...)               OPTICK_FRAME(__VA_ARGS__);
    #define PROFILE_THREAD(...)              OPTICK_THREAD(__VA_ARGS__);
    #define PROFILE_INIT_VULKAN(q,w,e,r,t,y) OPTICK_GPU_INIT_VULKAN(q,w,e,r,t,y);
    #define PROFILE_GPU_CONTEXT(...)         OPTICK_GPU_CONTEXT(__VA_ARGS__);
    #define PROFILE_GPU_EVENT(...)           OPTICK_GPU_EVENT(__VA_ARGS__);
    #define PROFILE_GPU_PRESENT(...)         OPTICK_GPU_FLIP(__VA_ARGS__);
#else
    #define PROFILE_SCOPED(...)
    #define PROFILE_FRAME(...)
    #define PROFILE_THREAD(...)
    #define PROFILE_INIT_VULKAN(q,w,e,r,t,y)
    #define PROFILE_GPU_CONTEXT(...)
    #define PROFILE_GPU_EVENT(...)
    #define PROFILE_GPU_PRESENT(...)
#endif
