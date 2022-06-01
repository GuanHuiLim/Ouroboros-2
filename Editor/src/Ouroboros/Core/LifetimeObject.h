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
            timer::init();
        }
    
        ~LifetimeObject()
        {
            timer::terminate();
            log::shutdown();
        }
    };
}
