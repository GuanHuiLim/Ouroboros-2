#extension GL_EXT_nonuniform_qualifier : require

layout (set = 2, binding = 0) uniform sampler2D textureDesArr[];
//layout(set = 1, binding= 0) uniform sampler2D textureSampler;

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inCol;

layout(location = 15) in flat uvec4 inInstanceData;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};

//layout(location = 1) in flat struct
//{
// ivec4 maps;
// //int albedo;	
// //int normal;	
// //int occlusion;
// //int roughness;
//}inTexIndex;

layout(location = 7) in struct 
{
	mat3 btn;
	vec3 vertCol;
	vec3 localVertexPos;
	vec3 localLightPos;
	vec3 localEyePos;
}inLightData;

layout (location = 0) out vec4 outPosition; // TODO: Optimization for space, not necessary as position can be reconstructed from depth.
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMaterial;

#include "shader_utility.shader"

float RandomUnsignedNormalizedFloat(uint x)
{
    return wang_hash(x) / 4294967295.0;
}

void PackPBRMaterialOutputs(in float roughness, in float metallic) // TODO: Add other params as needed
{
	// Precision of 0.03921568627451 for UNORM format, typically should be enough. Try not to change the format.
	const float todo_something = 1.0f;
	outMaterial = vec4(roughness, metallic, todo_something, todo_something);
}

void main()
{
	// implictly writes to depth buffer
	// gl_FragDepth = gl_FragCoord.z;
}
