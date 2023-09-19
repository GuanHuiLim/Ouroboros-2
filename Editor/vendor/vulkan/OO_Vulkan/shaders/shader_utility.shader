#ifndef _SHADER_UTILITY_SHADER_H_
#define _SHADER_UTILITY_SHADER_H_

#define pi 3.1415
#define EPSILON 0.00001

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308


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

vec4 ViewPosFromDepth(float depth, in vec2 uvCoord, in mat4 projInv) {

    float z = max(EPSILON, depth);
    // skip this step because vulkan
    // z = depth * 2.0 - 1.0;

    vec2 uv = uvCoord * 2.0 - 1.0;
    float x = uvCoord.x * 2.0 - 1.0;
    // flipped y for vulkan
    float y = (1.0 - uvCoord.y) * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(x, y, z, 1.0);
    vec4 viewSpacePosition = projInv * clipSpacePosition;

    // Perspective division
    viewSpacePosition /=  viewSpacePosition.w;

    return viewSpacePosition;
}

vec3 WorldPosFromDepth(float depth, in vec2 uvCoord, in mat4 projInv, in mat4 viewInv) {

    vec4 worldSpacePosition = viewInv * ViewPosFromDepth(depth,uvCoord,projInv);

    return worldSpacePosition.xyz;
}



#endif//INCLUDE_GUARD
