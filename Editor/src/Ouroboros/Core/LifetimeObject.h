#pragma once

// Uses RAII paradigm to make sure program lifetime objects are managed here
class LifetimeObject
{
public:
    LifetimeObject() = default;
    ~LifetimeObject() = default;
};
