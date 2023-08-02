/************************************************************************************//*!
\file           EntryPoint.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 05, 2022
\brief          Main Entry point to the program.
                Will hide away this from the Sandbox and they just have to implement
                a version of the CreateApplication function.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Ouroboros/Core/Application.h"
#include "Ouroboros/Core/StaticRuntime.h"
//#include "Ouroboros/Core/Base.h"
//#include "Ouroboros/Core/JobSystem/JobSystem.h"

#include "Ouroboros/Debug/MiniDumpHelper.h"

#include "Accessibility.h"

#include <Windows.h>
// Hints enable Nvidia optimus on laptop systems
extern "C" 
{
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

#ifdef OO_PLATFORM_WINDOWS

extern oo::Application* oo::CreateApplication(oo::CommandLineArgs args);

int main(int argc, char** argv)
{
    MiniDumpHelper::Init();
    __try
    {

    #ifdef OO_PRODUCTION
    // destroy and hide console 
    FreeConsole();
    #endif


    // Memory Leak Checker in Debug builds
    #if not defined (OO_PRODUCTION)
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
        // Uncomment to cause a break on allocation for debugging
        //_CrtSetBreakAlloc(/*Allocation Number here*/);
    #endif
        
        //RaiseException(
        //    1,                    // exception code 
        //    1,                    // non continuable exception 
        //    0, NULL);             // no arguments

        {
            oo::static_runtime::init();

            auto app = oo::CreateApplication({ argc, argv });
            
            app->Run();

            delete app;

            oo::static_runtime::terminate();
        }
    }
    __except (MiniDumpHelper::DumpMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
    {
        // Restore back when going to windowed or shutting down
        oo::accessibility::allow_accessibility_shortcut_keys(true);
    }

}

#endif