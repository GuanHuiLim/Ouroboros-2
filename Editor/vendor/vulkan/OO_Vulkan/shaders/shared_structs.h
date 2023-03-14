#ifndef COMMON_HOST_DEVICE
#define COMMON_HOST_DEVICE


#define BIND_POINT_VERTEX_BUFFER_ID  0
#define BIND_POINT_INSTANCE_BUFFER_ID  2
#define BIND_POINT_WEIGHTS_BUFFER_ID  1
#define BIND_POINT_GPU_SCENE_BUFFER_ID  3


#ifdef __cplusplus
#include "MathCommon.h"
// GLSL Type
using vec4 = glm::vec4;
using vec3 = glm::vec3;
using vec2 = glm::vec2;
using uvec4 = glm::uvec4;
using uvec3 = glm::uvec3;
using uvec2 = glm::uvec2;
using ivec4 = glm::ivec4;
using ivec3 = glm::ivec3;
using ivec2 = glm::ivec2;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

struct LocalLightInstance
{
    // x:1? cast shadow:dont cast , y: 0? render : dont render
    ivec4 info;// TODO: does this take up too much space?
    vec4 position;
    vec4 color;
    vec4 radius;
    mat4 projection;
    mat4 view[6];
};

struct OmniLightInstance
{
    ivec4 info; 
    vec4 position; // XYZ
    vec4 color; // RGB Intensity
    vec4 radius; // Inner rad outer rad etc..
    mat4 projection;
    mat4 view[6];
};

struct SpotLightInstance
{
    ivec4 info;
    vec4 position;
    vec3 color;
    vec4 radius; // x inner, y outer
    mat4 projection;
    mat4 view;
};

struct LightPC
{
    uint numLights;
    uint useSSAO;
    vec2 shadowMapGridDim;
    float ambient;
    float maxBias;
    float mulBias;
    uint PADDING;
};

struct SSAOPC
{
    vec2 screenDim;
    vec2 sampleDim; // screenDim_sampleDim
    float radius;
    float bias;
    float intensity;
    uint numSamples;
};

struct BloomPC
{
    vec4 threshold;
};

struct ColourCorrectPC
{
    vec4 shadowCol;
    vec4 midCol;
    vec4 highCol;
    vec2 threshold;

};

struct VignettePC
{
    vec4 colour;
    vec4 vignetteValues;

};

struct GPUTransform
{
	vec4 row0;
	vec4 row1;
	vec4 row2;
	vec4 colour; // temp
};

// struct represents perobject information in gpu
struct GPUObjectInformation
{
    uint boneStartIdx;
    int entityID;
    uint materialIdx;
    uint unused;
    vec4 emissiveColour;
};

#endif //! COMMON_HOST_DEVICE
