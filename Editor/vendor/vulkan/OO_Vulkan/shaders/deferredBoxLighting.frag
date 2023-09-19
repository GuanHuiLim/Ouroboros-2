layout (location = 0) in vec2 inUVo2;
layout (location = 1) in flat int inLightInstance;
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

	vec2 inUV = gl_FragCoord.xy / PC.resolution.xy;

	
	// Get G-Buffer values
	vec4 depth = texture(sampler2D(samplerDepth,basicSampler), inUV);
	vec3 fragPos = WorldPosFromDepth(depth.r,inUV,uboFrameContext.inverseProjection,uboFrameContext.inverseView);
	//fragPos.z = depth.r;
	vec3 normal = texture(sampler2D(samplerNormal,basicSampler), inUV).rgb;
	if(dot(normal,normal) == 0.0)
	{
		outFragcolor = vec4(0);
	//	outFragcolor = vec4(0.0,0.0,1.0,1.0);
		return;
	}
	vec4 albedo = texture(sampler2D(samplerAlbedo,basicSampler), inUV);
	vec4 material = texture(sampler2D(samplerMaterial,basicSampler), inUV);
	float SSAO = texture(sampler2D(samplerSSAO,basicSampler), inUV).r;
	float specular = material.g;
	float roughness = 1.0 - material.r;

	// Render-target composition
	//float ambient = PC.ambient;
	float ambient = 0.0;
	//if (DecodeFlags(material.z) == 0x1)
	//{
	//	ambient = 1.0;
	//}
	
	const float gamma = 2.2;
	albedo.rgb =  pow(albedo.rgb, vec3(1.0/gamma));

	// Ambient part
	vec3 result = albedo.rgb  * ambient;

	// remove SSAO if not wanted
	if(PC.useSSAO == 0){
		SSAO = 1.0;
	}
	
	float outshadow = texture(sampler2D(samplerShadows,basicSampler),inUV).r;
	
	// Point Lights
	vec3 lightContribution = vec3(0.0);
	/////////for(int i = 0; i < PC.numLights; ++i)
	{
		vec3 res = EvalLight(inLightInstance, fragPos, normal, roughness ,albedo.rgb, specular);	

		lightContribution += res;
	}

	//lightContribution *= outshadow;
	
	vec3 ambientContribution = albedo.rgb  * ambient;
	//vec3 emissive = texture(sampler2D(samplerEmissive,basicSampler),inUV).rgb;
	vec3 emissive = vec3(0);
	result =  (ambientContribution * SSAO + lightContribution) + emissive;

	outFragcolor = vec4(result, albedo.a);	

}
