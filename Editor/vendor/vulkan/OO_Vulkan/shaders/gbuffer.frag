#extension GL_EXT_nonuniform_qualifier : require

#include "material.shader"

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inColor;
layout(location = 3) in flat int inEntityID;
layout(location = 4) in flat vec4 inEmissiveColour;

layout(location = 15) in flat uvec4 inInstanceData;
layout(location = 7) in struct
{
    mat3 btn;
}inLightData;

//layout(location = 0) out vec4 outPosition; // Optimization for space reconstructed from depth.
layout(location = 0) out vec4 outNormal;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outMaterial;
layout(location = 3) out vec4 outEmissive;
layout(location = 4) out int outEntityID;

#include "shader_utility.shader"

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};

layout (set = 2, binding = 0) uniform sampler2D textureDescriptorArray[];
//layout(set = 1, binding= 0) uniform sampler2D textureSampler;

vec4 PackPBRMaterialOutputs(in float roughness, in float metallic,in float emissive) // TODO: Add other params as needed
{
    // Precision of 0.03921568627451 for UNORM format, typically should be enough. Try not to change the format.
    return vec4(roughness, metallic, emissive, 1.0f);
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

// !! DANGER !! - Dont ever learn this... Look at assembly/performance first.
void Jump(inout uint perInstanceData, inout float3 albedo)
{
    switch (perInstanceData)
    {
        case 1:
        {
            albedo.rgb *= (0.5f * uboFrameContext.renderTimer.y + 0.5f);
            break;
        }
        case 2:
        {
            const float sine_normalized = 0.5f * sin(uboFrameContext.renderTimer.x * 3.14159f * 15.0f) + 0.5f;
            albedo.rgb *= float3(1.0f * sine_normalized);
            perInstanceData = 1; // Hack
            break;
        }
        case 3:
        {
            albedo.rgb = main_voronoi(uboFrameContext.renderTimer.x, inUV.xy).xyz;
            perInstanceData = 1; // Hack
            break;
        }
    }
}

void main()
{
    outEntityID = inEntityID;
    outAlbedo = vec4(inColor, 1.0);
    
    //outPosition = inPosition;
    // implicit depthOut will reconstruct the position

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
    const uint textureIndex_Emissive  = inInstanceData.y >> 16;
   

    {
        outAlbedo.rgb = texture(textureDescriptorArray[textureIndex_Albedo], inUV.xy).rgb;
        
        // !! DANGER !! - Dont ever learn this... Look at assembly/performance first.
        Jump(perInstanceData, outAlbedo.rgb);
    }
	
    {
        outNormal = vec4(inLightData.btn[2], 0.0);
    }
	if(textureIndex_Normal != 1)
	{        
        vec3 texNormal = texture(textureDescriptorArray[textureIndex_Normal], inUV.xy).xyz;
        //float gamma = 2.2;
        //texNormal = pow(texNormal,vec3(gamma));
        texNormal = texNormal  * 2.0 - 1.0;
        //texNormal = normalize(max(vec3(0),texNormal));

        //texNormal = texNormal * 2.0 - 1.0;
		outNormal.rgb = normalize(inLightData.btn * texNormal);
		//outNormal.rgb = texNormal;
       // outNormal.rgb = vec3(0.0,1.0,0.0);
	}

    {
        // Commented out because unused.
        //const vec2 roughness_metallic = GenerateRandom_RoughnessMetallic(inInstanceData.x);

        // TODO Optimization: Bake these kinds of individual material textures together. Reduce the number of texture samples.
        const float roughness = texture(textureDescriptorArray[textureIndex_Roughness], inUV.xy).r;
        const float metallic = texture(textureDescriptorArray[textureIndex_Metallic], inUV.xy).r;

        outMaterial = PackPBRMaterialOutputs(roughness, metallic, 1.0);
        uint flags = perInstanceData;
        outMaterial.z = EncodeFlags(flags);
    }

    {
        outEmissive.rgb = texture(textureDescriptorArray[textureIndex_Emissive], inUV.xy).rrr * inEmissiveColour.rgb * inEmissiveColour.a;
    }

}
