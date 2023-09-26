#ifndef _SHADER_UTILITY_SHADER_H_
#define _SHADER_UTILITY_SHADER_H_

#define PI 3.14159265359
#define EPSILON 0.00001

#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

// Based omn http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
float random(vec2 co)
{
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(co.xy, vec2(a, b));
    float sn = mod(dt, 3.14);
    return fract(sin(sn) * c);
}

vec3 EncodeNormalHelper(vec3 SrcNormal)
{
    return SrcNormal * .5f + .5f;
}

vec3 DecodeNormalHelper(vec3 SrcNormal)
{
    return SrcNormal * 2.0f - 1.0f;
}

float RGBtoLuminance(vec3 srcRGB)
{
    return dot(srcRGB, vec3(0.2126, 0.7152, 0.0722));
}

vec3 GammaToLinear(vec3 srcRGB)
{
    const float gamma = 2.2;
    return pow(srcRGB, vec3(gamma));
}

vec3 LinearToGamma(vec3 srcRGB)
{
    const float gamma = 2.2;
    return pow(srcRGB, 1.0/vec3(gamma));
}

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

//// Cube functions
vec3 cubeCoordToWorld(ivec3 cubeCoord, vec2 cubemapSize)
{
    vec2 texCoord = vec2(cubeCoord.xy) / cubemapSize;
    texCoord = texCoord * 2.0 - 1.0; // -1..1
    switch (cubeCoord.z)
    {
        case 0: return vec3(        1.0 , -texCoord.yx             ); // posx
        case 1: return vec3(       -1.0 , -texCoord.y ,  texCoord.x); //negx
        case 2: return vec3( texCoord.x ,          1.0,  texCoord.y); // posy
        case 3: return vec3( texCoord.x ,         -1.0, -texCoord.y); //negy
        case 4: return vec3( texCoord.x , -texCoord.y ,         1.0); // posz
        case 5: return vec3(-texCoord.xy,                      -1.0); // negz
    }
    return vec3(0.0);
}

ivec3 texCoordToCube(vec3 texCoord, vec2 cubemapSize)
{
    vec3 abst = abs(texCoord);
    texCoord /= max3(abst);

    float cubeFace;
    vec2 uvCoord;
    if (abst.x > abst.y && abst.x > abst.z)
    {
          // x major
        float negx = step(texCoord.x, 0.0);
        uvCoord = mix(-texCoord.zy, vec2(texCoord.z, -texCoord.y), negx);
        cubeFace = negx;
    }
    else if (abst.y > abst.z)
    {
          // y major
        float negy = step(texCoord.y, 0.0);
        uvCoord = mix(texCoord.xz, vec2(texCoord.x, -texCoord.z), negy);
        cubeFace = 2.0 + negy;
    }
    else
    {
          // z major
        float negz = step(texCoord.z, 0.0);
        uvCoord = mix(vec2(texCoord.x, -texCoord.y), -texCoord.xy, negz);
        cubeFace = 4.0 + negz;
    }
    uvCoord = (uvCoord + 1.0) * 0.5; // 0..1
    uvCoord = uvCoord * cubemapSize;
    uvCoord = clamp(uvCoord, vec2(0.0), cubemapSize - vec2(1.0));

    return ivec3(ivec2(uvCoord), int(cubeFace));
}
//// end cube functions

#endif//INCLUDE_GUARD
