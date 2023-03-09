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

#include "lights.shader"

layout( push_constant ) uniform lightpc
{
	LightPC PC;
};

#include "lightingEquations.shader"

vec2 GetShadowMapRegion(int gridID, in vec2 uv, in vec2 gridSize)
{
	
	vec2 gridIncrement = vec2(1.0)/gridSize; // size for each cell

	vec2 actualUV = gridIncrement * uv; // uv local to this cell

	// avoid the modolus operator not sure how much that matters
	int y = gridID/int(gridSize.x);
	int x = gridID - int(gridSize.x*y);

	vec2 offset = gridIncrement * vec2(x,y); // offset to our cell

	return offset+actualUV; //sampled position
}

float ShadowCalculation(int lightIndex,int gridID , in vec4 fragPosLightSpace, float NdotL)
{

	// perspective divide
	vec4 projCoords = fragPosLightSpace/fragPosLightSpace.w;
	//normalization [0,1] tex coords only.. FOR VULKAN DONT DO Z
	projCoords.xy = projCoords.xy* 0.5 + 0.5;

	vec2 uvs = vec2(projCoords.x,projCoords.y);
	uvs = GetShadowMapRegion(gridID,uvs,PC.shadowMapGridDim);
	
	// Flip y during sample
	uvs = vec2(uvs.x, 1.0-uvs.y);
	
	// Bounds check for the actual shadow map
	float closestDepth = 1.0;
	if(projCoords.x >1.0 || projCoords.x < 0.0
		|| projCoords.y >1.0 || projCoords.y < 0.0 
		|| projCoords.z>1)
	{
		return 1.0;
	}
	else
	{
		closestDepth = texture(samplerShadows,uvs).r;
	}
	float currDepth = projCoords.z;

	float maxbias =  PC.maxBias;
	float mulBias = PC.mulBias;
	float bias = max(mulBias * (1.0 - NdotL),maxbias);
	float shadow = 1.0;
	if (projCoords.w > 0.0 && currDepth - bias > closestDepth ) 
	{
		if(projCoords.z < 1)
		{
			shadow = 0.0;	
		}
	}

	return shadow;
}

vec3 EvalLight(int lightIndex, in vec3 fragPos, in vec3 normal,float roughness, in vec3 albedo, float specular, out float shadow)
{
	vec3 result = vec3(0.0f, 0.0f, 0.0f);	
	vec3 N = normalize(normal);
	float alpha = roughness;
	vec3 Kd = albedo;
	vec3 Ks = vec3(specular);
	//Ks = vec3(0);
	
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
		vec4 lightColInten	= Lights_SSBO[lightIndex].color;
		vec3 lCol = lightColInten.rgb * lightColInten.w;

    		// Attenuation
		float atten = Lights_SSBO[lightIndex].radius.x / (pow(dist, 2.0) + 1.0);		 
	
		// Diffuse part
		
		vec3 diff = lCol * GGXBRDF(L , V , H , N , alpha , Kd , Ks) * NdotL * atten;


		// Specular part
		// Specular map values are stored in alpha of albedo mrt
		vec3 R = -reflect(L, N);
		float RdotV = max(0.0, dot(R, V));
		vec3 spec = lCol * specular * pow(RdotV, 16.0) * atten;
		//vec3 spec = lCol  * pow(RdotV, 16.0) * atten;
	
		//result = diff;// + spec;	
		result = diff+spec;
	}

	// calculate shadow if this is a shadow light
	shadow = 1.0;
	if(Lights_SSBO[lightIndex].info.x > 0)
	{		
		if(Lights_SSBO[lightIndex].info.x == 1)
		{
			int gridID = Lights_SSBO[lightIndex].info.y;
			for(int i = 0; i < 6; ++i)
			{
				vec4 outFragmentLightPos = Lights_SSBO[lightIndex].projection * Lights_SSBO[lightIndex].view[i] * vec4(fragPos,1.0);
				shadow *= ShadowCalculation(lightIndex,gridID+i,outFragmentLightPos,NdotL);
			}
		}
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
	if(dot(normal,normal) == 0.0)
	{
		outFragcolor = vec4(0);
		return;
	}
	vec4 albedo = texture(samplerAlbedo, inUV);
	vec4 material = texture(samplerMaterial, inUV);
	float SSAO = texture(samplerSSAO, inUV).r;
	float specular = material.g;
	float roughness = 1.0 - material.r;

	// Render-target composition
	float ambient = PC.ambient;
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
	
	// Point Lights
	vec3 lightContribution = vec3(0.0);
	for(int i = 0; i < PC.numLights; ++i)
	{
		float outshadow = 1.0;
		vec3 res = EvalLight(i, fragPos, normal, roughness ,albedo.rgb, specular, outshadow);	

		lightContribution += res;
	}
	
	vec3 ambientContribution = albedo.rgb  * ambient ;
	result =  (ambientContribution * SSAO + lightContribution);

	outFragcolor = vec4(result, albedo.a);	

}
