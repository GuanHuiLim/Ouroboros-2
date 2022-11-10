layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};
#include "shared_structs.h"

layout (set = 0, binding = 1) uniform sampler2D samplerDepth;
// layout (set = 0, binding = 1) uniform sampler2D samplerposition; we construct position using depth
layout (set = 0, binding = 2) uniform sampler2D samplerNormal;
layout (set = 0, binding = 3) uniform sampler2D samplerAlbedo;
layout (set = 0, binding = 4) uniform sampler2D samplerMaterial;
layout (set = 0, binding = 5) uniform sampler2D samplerShadows;
layout (set = 0, binding = 6) uniform sampler2D samplerSSAO;


layout(std430, set = 0, binding = 7) readonly buffer Lights
{
	SpotLightInstance Lights_SSBO[];
};

layout( push_constant ) uniform lightpc
{
	LightPC PC;
};

#include "lightingEquations.shader"

float ShadowCalculation(in vec4 fragPosLightSpace, float NdotL)
{

	// perspective divide
	vec4 projCoords = fragPosLightSpace/fragPosLightSpace.w;
	//normalization [0,1] tex coords only.. FOR VULKAN DONT DO Z
	projCoords.xy = projCoords.xy* 0.5 + 0.5;

	float maxbias =  PC.maxBias;
	float mulBias = PC.mulBias;
	float bias = max(mulBias * (1.0 - NdotL),maxbias);
	// Flip y during sample
	vec2 uvs = vec2(projCoords.x,1.0-projCoords.y);
	float closestDepth = texture(samplerShadows,uvs).r;
	float currDepth = projCoords.z;

	float shadow = 1.0;
	if (projCoords.w > 0.0 && currDepth - bias > closestDepth ) 
	{
		if(projCoords.z < 1)
		{
			shadow = 0.0;		
		}
	}
	//shadow = currDepth - bias > closestDepth ? 1.0 : 0.0;

	return shadow;
}

vec3 EvalLight(int lightIndex, in vec3 fragPos, in vec3 normal,float roughness, in vec3 albedo, float specular, float shadow)
{
	vec3 result = vec3(0.0f, 0.0f, 0.0f);
	vec3 N = normalize(normal);
	float alpha = roughness;
	vec3 Kd = albedo;
	vec3 Ks = vec3(specular);
	
	// Vector to light
	vec3 L = Lights_SSBO[lightIndex].position.xyz - fragPos;

	// Distance from light to fragment position
	float dist = length(L);
	
	// Viewer to fragment
	vec3 V = uboFrameContext.cameraPosition.xyz - fragPos;
	
	// Light to fragment
	L = normalize(L);	
	V = normalize(V);
	vec3 H = normalize(L+V);
	float NdotL = max(0.0, dot(N, L));

	//if(dist < Lights_SSBO[lightIndex].radius.x)
	{
		//SpotLightInstance light = SpotLightInstance(Omni_LightSSBO[lightIndex]); 
	    
		float r1 = Lights_SSBO[lightIndex].radius.x * 0.9;
		float r2 = Lights_SSBO[lightIndex].radius.x;

    		// Attenuation
		float atten = Lights_SSBO[lightIndex].radius.x / (pow(dist, 2.0) + 1.0);		 
	
		// Diffuse part
		
		vec3 diff = Lights_SSBO[lightIndex].color.xyz * GGXBRDF(L , V , H , N , alpha , Kd , Ks) * NdotL * atten;


		// Specular part
		// Specular map values are stored in alpha of albedo mrt
		//vec3 R = -reflect(L, N);
		//float RdotV = max(0.0, dot(R, V));
		//vec3 spec = ubo.lights[lightIndex].color.xyz * specular * pow(RdotV, 16.0) * atten;
	
		//result = diff;// + spec;	
		result = diff;
	}

	if(lightIndex == 0)
	{
		vec4 outFragmentLightPos = Lights_SSBO[lightIndex].projection * Lights_SSBO[lightIndex].view * vec4(fragPos,1.0);
		float shadow = ShadowCalculation(outFragmentLightPos,NdotL);
		result *= shadow;
	}

	return result;
//	return fragPos;
}


uint DecodeFlags(in float value)
{
    return uint(value * 255.0f);
}

#include "shader_utility.shader"

void main()
{
	// Get G-Buffer values
	vec4 depth = texture(samplerDepth, inUV);
	vec3 fragPos = WorldPosFromDepth(depth.r,inUV,uboFrameContext.inverseProjection,uboFrameContext.inverseView);
	//fragPos.z = depth.r;
	vec3 normal = texture(samplerNormal, inUV).rgb;
	vec4 albedo = texture(samplerAlbedo, inUV);
	vec4 material = texture(samplerMaterial, inUV);
	float SSAO = texture(samplerSSAO, inUV).r;
	float specular = material.b;
	float roughness = material.r;

	// Render-target composition
	float ambient = PC.ambient;
	if (DecodeFlags(material.z) == 0x1)
	{
		ambient = 1.0;
	}
	float gamma = 2.2;
	
	albedo.rgb =  pow(albedo.rgb, vec3(gamma));

	// Ambient part
	vec3 result = albedo.rgb  * ambient;	
	if(PC.useSSAO != 0){
		result *=  SSAO;
	}
	
	// Point Lights
	for(int i = 0; i < PC.numLights; ++i)
	{
			result += EvalLight(i, fragPos, normal, roughness ,albedo.rgb, specular, 0.0);
	}
	

	result = pow(result, vec3(1.0/gamma));
	outFragcolor = vec4(result, 1.0);	
}
