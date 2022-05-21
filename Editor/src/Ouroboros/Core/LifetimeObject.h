#pragma once

#include "Timer.h"

// Uses RAII paradigm to make sure program lifetime objects are managed here
class LifetimeObject
{
public:
    LifetimeObject()
    {
        timer::init();
    }
    
    ~LifetimeObject()
    {
        timer::terminate();
    }
};
