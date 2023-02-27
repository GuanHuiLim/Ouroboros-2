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


const float pxRange = 2.0; // set to distance field's pixel range

float screenPxRange() {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(textureDescriptorArray[inInstanceData.x], 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(inUV.xy);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    outEntityID = int(inInstanceData.y);
    outfragCol = vec4(inColor.rgba);
    
    uint isSDFFont = inInstanceData.z;

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
   
    outfragCol.rgba = texture(textureDescriptorArray[inInstanceData.x], inUV.xy).rgba;
     if(outfragCol.a < 0.0001) discard;
    
    if(isSDFFont == 1)
    {
        float sd = median(outfragCol.r, outfragCol.g, outfragCol.b);
        float screenPxDistance = screenPxRange()*(sd - 0.5);
        float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
        outfragCol = mix(vec4(0),inColor,opacity);
        
        if(outfragCol.a < 0.0001) discard; // this is bad and broken
    }else
{
outfragCol = outfragCol * inColor.rgba;
outfragCol.rgb = pow(outfragCol.rgb,vec3(2.2));
}

    // hardcode red
    //outfragCol = vec4(1.0,0.0,0.0,1.0);
   
}
