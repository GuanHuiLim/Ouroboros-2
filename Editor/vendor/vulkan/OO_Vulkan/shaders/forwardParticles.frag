#extension GL_EXT_nonuniform_qualifier : require

#include "material.shader"

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;
layout(location = 3) in flat uvec4 inInstanceData;

layout(location = 0) out vec4 outfragCol;
layout(location = 1) out int outEntityID;

#include "shader_utility.shader"

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};

layout (set = 2, binding = 0) uniform sampler2D textureDescriptorArray[];
//layout(set = 1, binding= 0) uniform sampler2D textureSampler;

vec4 PackPBRMaterialOutputs(in float roughness, in float metallic) // TODO: Add other params as needed
{
    // Precision of 0.03921568627451 for UNORM format, typically should be enough. Try not to change the format.
    const float todo_something = 0.0f;
    return vec4(roughness, metallic, todo_something, 1.0f);
}

// Subject to change
float EncodeFlags(uint flags)
{
    return (flags) / 255.0f;
} 

vec2 GenerateRandom_RoughnessMetallic(in uint seed)
{
    const float roughness = RandomUnsignedNormalizedFloat(seed);
    const float metallic  = RandomUnsignedNormalizedFloat(seed + 0xDEADDEAD);
    return vec2(roughness, metallic);
}

void main()
{
    outEntityID = int(inInstanceData.x);
    outfragCol = vec4(inColor.rgba);
    
    if(inColor.a < 0.0001) discard;
    

    // TODO: We need to use a mask to check whether to use textures or values.
    const bool useAlbedoTexture = true;
    const bool useNormalMapTexture = true;
    const bool useRoughnessTexture = true;
    const bool useMetallicTexture = true;
    const bool useAmbientOcclusionTexture = true;

    // Unpack per instance data
    const uint textureIndex_Albedo    = inInstanceData.z >> 16;
    const uint textureIndex_Normal    = inInstanceData.z & 0xFFFF;
    const uint textureIndex_Roughness = inInstanceData.w >> 16;
    const uint textureIndex_Metallic  = inInstanceData.w & 0xFFFF;
    uint perInstanceData              = inInstanceData.y & 0xFF;
   
    outfragCol.rgba = texture(textureDescriptorArray[textureIndex_Albedo], inUV.xy).rgba;
     if(outfragCol.a < 0.0001) discard;
    outfragCol *= inColor;
	
   
}
