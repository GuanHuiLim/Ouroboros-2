#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inCol;
layout(location = 15) in flat uvec4 inInstanceData;
layout(location = 7) in struct 
{
	mat3 btn;
	vec3 vertCol;
	vec3 localVertexPos;
	vec3 localLightPos;
	vec3 localEyePos;
}inLightData;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};

layout (set = 2, binding = 0) uniform sampler2D textureDesArr[];

#include "shader_utility.shader"

void main()
{
	// implictly writes to depth buffer
	// gl_FragDepth = gl_FragCoord.z;
}
