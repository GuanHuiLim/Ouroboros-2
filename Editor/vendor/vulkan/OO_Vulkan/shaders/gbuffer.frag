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
    vec3 b;
	vec3 t;
	vec3 n;
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

layout (set = 0, binding = 0) uniform sampler basicSampler;
layout (set = 2, binding = 0) uniform texture2D textureDescriptorArray[];


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


mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
    {
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float val = max( dot(T,T), dot(B,B) );
    val = max(FLT_MIN, val); // to ensure no degenerate stuff
    float invmax = inversesqrt( val );
    return mat3( T * invmax, B * invmax, N );
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
   
    vec3 normalInfo = vec3(0.0);
    {
        outAlbedo.rgb = texture(sampler2D(textureDescriptorArray[textureIndex_Albedo],basicSampler), inUV.xy).rgb;
    }
	
    {
        normalInfo = inLightData.n;
    }

	if(textureIndex_Normal != 1)
	{        
        vec3 N = normalize(inLightData.n);   
         
        vec3 V = normalize(inPosition.xyz - uboFrameContext.cameraPosition.xyz);    
  
        vec3 map = texture(sampler2D(textureDescriptorArray[textureIndex_Normal],basicSampler), inUV.xy).xyz*2.0-1.0;
        
        // new method
        mat3 TBN = cotangent_frame( N, V,  inUV.xy );
        vec3 genNorms = normalize( TBN * map );
 
        mat3 inTBN = mat3(inLightData.t,inLightData.b,inLightData.n);
            
        // old method
        normalInfo.rgb = normalize(inTBN * map);
       
        normalInfo.rgb = genNorms;
    }
    
    outNormal.rgb = EncodeNormalHelper(normalInfo);

    {
        // Commented out because unused.
        //const vec2 roughness_metallic = GenerateRandom_RoughnessMetallic(inInstanceData.x);

        // TODO Optimization: Bake these kinds of individual material textures together. Reduce the number of texture samples.
        const float roughness = texture(sampler2D(textureDescriptorArray[textureIndex_Roughness],basicSampler), inUV.xy).r;
        const float metallic = texture(sampler2D(textureDescriptorArray[textureIndex_Metallic],basicSampler), inUV.xy).r;

        outMaterial = PackPBRMaterialOutputs(roughness, metallic, 1.0);
        uint flags = perInstanceData;
        outMaterial.z = EncodeFlags(flags);
    }

    {
        outEmissive.rgb = texture(sampler2D(textureDescriptorArray[textureIndex_Emissive],basicSampler), inUV.xy).rrr * inEmissiveColour.rgb * inEmissiveColour.a;
    }

}
