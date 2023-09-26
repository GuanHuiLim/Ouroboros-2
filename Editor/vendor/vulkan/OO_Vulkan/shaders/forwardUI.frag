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

layout (set = 0, binding = 0) uniform sampler basicSampler;
layout (set = 2, binding = 0) uniform texture2D textureDescriptorArray[];


const float pxRange = 2.0; // set to distance field's pixel range

float screenPxRange() {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(sampler2D(textureDescriptorArray[inInstanceData.x],basicSampler), 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(inUV.xy);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void Remap_float(float In, vec2 InMinMax, vec2 OutMinMax, out float Out)
{
    Out = OutMinMax.x + (In - InMinMax.x) * (OutMinMax.y - OutMinMax.x) / (InMinMax.y - InMinMax.x);
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
   
    outfragCol.rgba = texture(sampler2D(textureDescriptorArray[inInstanceData.x],basicSampler), inUV.xy).rgba;
    
    if(isSDFFont == 1)
    {
        float sd = median(outfragCol.r, outfragCol.g, outfragCol.b);
        float screenPxDistance = screenPxRange()*(sd - 0.5);
        float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
        outfragCol = mix(vec4(0),inColor,opacity);
        //Remap_float(outfragCol.a,vec2(0.0,1.0),vec2(0,0.20),outfragCol.a);
        
        
    }else
    {
        vec4 tempcol = inColor.rgba;
        //Remap_float(tempcol.a,vec2(0.0,1.0),vec2(0.0,0.20),tempcol.a);
        //tempcol.a = min(50.0/255.0 * tempcol.a, 1.0);
        outfragCol.rgb = outfragCol.rgb * inColor.rgb;
        outfragCol.a = outfragCol.a * tempcol.a;

        // done after tonemapping so correct here
        outfragCol.rgb = GammaToLinear(outfragCol.rgb);
    }

	if(outfragCol.a < 0.0001) discard; // this is bad and broken
    // hardcode red
    //outfragCol = vec4(1.0,0.0,0.0,1.0);
   
}
