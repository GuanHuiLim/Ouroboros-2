layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};
#include "shared_structs.h"

layout (set = 0, binding = 0) uniform sampler basicSampler; 
layout (set = 0, binding = 1) uniform texture2D samplerDepth;
layout (set = 0, binding = 2) uniform texture2D samplerNormal;
layout (set = 0, binding = 3) uniform texture2D samplerAlbedo;
layout (set = 0, binding = 4) uniform texture2D samplerMaterial;
layout (set = 0, binding = 5) uniform texture2D samplerEmissive;
layout (set = 0, binding = 6) uniform texture2D samplerShadows;
layout (set = 0, binding = 7) uniform texture2D samplerSSAO;

#include "lights.shader"

layout( push_constant ) uniform lightpc
{
	LightPC PC;
};

#include "lightingEquations.shader"


uint DecodeFlags(in float value)
{
    return uint(value * 255.0f);
}

#include "shader_utility.shader"

void main()
{
	// just perform ambient
	vec4 albedo = texture(sampler2D(samplerAlbedo,basicSampler), inUV);
	float ambient = PC.ambient;
	
	const float gamma = 2.2;
	albedo.rgb =  pow(albedo.rgb, vec3(1.0/gamma));
	// Ambient part
	vec3 emissive = texture(sampler2D(samplerEmissive,basicSampler),inUV).rgb;
	vec3 result = albedo.rgb  * ambient + emissive;
	outFragcolor = vec4(result, albedo.a);	

}
