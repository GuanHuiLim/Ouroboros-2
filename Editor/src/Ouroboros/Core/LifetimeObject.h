/************************************************************************************//*!
\file           LifetimeObject.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           May 10, 2022
\brief          Defines an object that'll be created once at the very start of the 
                project and gets destroyed through RAII.
                
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

namespace oo
{
    // Uses RAII paradigm to make sure program lifetime objects are managed here
    class LifetimeObject
    {
    public:
        LifetimeObject()
        {
            log::init();
            LOG_CORE_INFO("Begin loading static lifetime objects");
            timer::init();
        }
    
        ~LifetimeObject()
        {
            timer::terminate();
            LOG_CORE_INFO("Finish unloading static lifetime objects");
            log::shutdown();
        }
    };
}
