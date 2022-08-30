#ifndef COMMON_HOST_DEVICE
#define COMMON_HOST_DEVICE


#define VERTEX_BUFFER_ID  0
#define INSTANCE_BUFFER_ID  1
#define GPU_SCENE_BUFFER_ID  3


#ifdef __cplusplus
#include <glm/glm.hpp>
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
    float innerRadius;
    float outerRadius;
    mat4 projection;
    mat4 view;
};


struct GPUTransform
{
	vec4 row0;
	vec4 row1;
	vec4 row2;
	vec4 colour; // temp
};

#endif //! COMMON_HOST_DEVICE
