/************************************************************************************//*!
\file           Profiling.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Profile defines for optick

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

// Note: This header file only wraps the C++ Macros needed for external profiling tools.

//#define USE_PROFILING_OPTICK

#pragma warning( push )
#pragma warning( disable : 26819 ) // fallthrough
#pragma warning( disable : 26495 ) // uninitialized
#pragma warning( disable : 26451 ) // arithmetic overflow
#pragma warning( disable : 6387 ) // temp value
#pragma warning( disable : 6385 ) // invalid data

#if defined(USE_PROFILING_OPTICK)
    #include "optick/optick.h"
    #define PROFILE_SCOPED(...)              OPTICK_EVENT(__VA_ARGS__);
    #define PROFILE_FRAME(...)               OPTICK_FRAME(__VA_ARGS__);
    #define PROFILE_THREAD(...)              OPTICK_THREAD(__VA_ARGS__);
    #define PROFILE_INIT_VULKAN(q,w,e,r,t,y) OPTICK_GPU_INIT_VULKAN(q,w,e,r,t,y);
    #define PROFILE_GPU_CONTEXT(...)         OPTICK_GPU_CONTEXT(__VA_ARGS__);
    #define PROFILE_GPU_EVENT(...)           OPTICK_GPU_EVENT(__VA_ARGS__);
    #define PROFILE_GPU_PRESENT(...)         OPTICK_GPU_FLIP(__VA_ARGS__);
    #define PROFILE_GPU_SHUTDOWN(...)         OPTICK_SHUTDOWN();
#else
    #define PROFILE_SCOPED(...)
    #define PROFILE_FRAME(...)
    #define PROFILE_THREAD(...)
    #define PROFILE_INIT_VULKAN(q,w,e,r,t,y)
    #define PROFILE_GPU_CONTEXT(...)
    #define PROFILE_GPU_EVENT(...)
    #define PROFILE_GPU_PRESENT(...)
    #define PROFILE_GPU_SHUTDOWN(...) 
#endif

#pragma warning( pop )
