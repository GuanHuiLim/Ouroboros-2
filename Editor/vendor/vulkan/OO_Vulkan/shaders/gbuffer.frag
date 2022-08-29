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
	const float todo_something = 0.0f;
	outMaterial = vec4(roughness, metallic, todo_something, 1.0f);
}

void main()
{
	outAlbedo = vec4(inCol, 1.0);

	const uint textureIndex = inInstanceData.y;

	if(textureIndex > uint(0))
	{
		outAlbedo.rgb = texture(textureDesArr[textureIndex], inUV.xy).rgb;
	}

	outNormal = vec4(inLightData.btn[2], 1.0f);
	outPosition = inPos;

	// TODO: Fix this hardcoded value properly...
	{
		const uint seed = inInstanceData.x; // Generate random PBR params based on object ID or something.
		//const float roughness = RandomUnsignedNormalizedFloat(seed);
		//const float metallic  = RandomUnsignedNormalizedFloat(seed + 0xDEADDEAD);

		const float roughness = texture(textureDesArr[inInstanceData.w >> 16], inUV.xy).r;
		const float metallic = texture(textureDesArr[inInstanceData.w & 0xFFFF], inUV.xy).r;

		PackPBRMaterialOutputs(roughness, metallic);
	}

	outAlbedo.rgb = texture(textureDesArr[inInstanceData.z >> 16], inUV.xy).rgb;
}
