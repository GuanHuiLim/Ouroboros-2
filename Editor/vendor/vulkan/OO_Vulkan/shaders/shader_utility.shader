#ifndef _SHADER_UTILITY_SHADER_H_
#define _SHADER_UTILITY_SHADER_H_

uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

float RandomUnsignedNormalizedFloat(uint seed)
{
    return wang_hash(seed) / 4294967295.0;
}

#endif//INCLUDE_GUARD
