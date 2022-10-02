#ifndef COMMON_HOST_DEVICE
#define COMMON_HOST_DEVICE


#define BIND_POINT_VERTEX_BUFFER_ID  0
#define BIND_POINT_INSTANCE_BUFFER_ID  2
#define BIND_POINT_WEIGHTS_BUFFER_ID  1
#define BIND_POINT_GPU_SCENE_BUFFER_ID  3


#ifdef __cplusplus
#include "MathCommon.h"
// GLSL Type
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using uvec4 = glm::uvec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

struct LocalLightInstance
{
    vec4 position;
    vec4 color;
    vec4 radius;
    mat4 projection;
    mat4 view[6];
};

struct OmniLightInstance
{
    vec4 position;
    vec4 color;
    vec4 radius;
    mat4 projection;
    mat4 view[6];
};

struct SpotLightInstance
{
    vec4 position;
    vec3 color;
    vec4 radius; // x inner, y outer
    mat4 projection;
    mat4 view;
};

struct LightPC
{
    uvec4 numLights;
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
    uint boneCnt;
    uint materialIdx;
    uint unused;
};

#endif //! COMMON_HOST_DEVICE
