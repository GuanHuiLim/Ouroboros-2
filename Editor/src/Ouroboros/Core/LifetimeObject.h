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
