/************************************************************************************//*!
\file           StaticRuntime.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 10, 2022
\brief          Defines a global namespace that should only be initialized and
                terminated once
                
                Should only put stateful functions that are required at the start
                of the project and hangs around until when the program shutsdown.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Timer.h"
#include "Log.h"
#include <Physics/Source/phy.h>
#include "Ouroboros/Audio/Audio.h"
//#include <JobSystem/src/final/jobs.h>

namespace oo
{
    namespace static_runtime
    {
        void init()
        {
            //jobsystem::initialize();
            log::init();
            LOG_CORE_INFO("Begin loading static lifetime objects");
            timer::init();
            myPhysx::physx_system::init();
            audio::Init(2048);
        }
    
        void terminate()
        {
            timer::terminate();
            LOG_CORE_INFO("Finish unloading static lifetime objects");
            log::shutdown();
            myPhysx::physx_system::shutdown();
            audio::ShutDown();
            //jobsystem::shutdown();
        }
    };
}
