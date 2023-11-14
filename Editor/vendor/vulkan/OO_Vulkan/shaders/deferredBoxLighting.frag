layout (location = 1) in flat int inLightInstance;
layout (location = 0) out vec4 outFragcolor;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};
#include "shared_structs.h"

layout (set = 0, binding = 0) uniform sampler basicSampler; 
layout (set = 0, binding = 1) uniform texture2D textureDepth;
layout (set = 0, binding = 2) uniform texture2D textureNormal;
layout (set = 0, binding = 3) uniform texture2D textureAlbedo;
layout (set = 0, binding = 4) uniform texture2D textureMaterial;
layout (set = 0, binding = 5) uniform texture2D textureEmissive;
layout (set = 0, binding = 6) uniform texture2D textureShadows;
layout (set = 0, binding = 7) uniform texture2D textureSSAO;
// layout 8 taken by buffer
layout (set = 0, binding = 9) uniform sampler cubeSampler;
layout (set = 0, binding = 10) uniform textureCube irradianceCube;
layout (set = 0, binding = 11)uniform textureCube prefilterCube;
layout (set = 0, binding = 12)uniform texture2D brdfLUT;

layout (set = 0, binding = 13) uniform samplerShadow shadowSampler;


#include "lights.shader"

layout( push_constant ) uniform lightpc
{
	LightPC PC;
};

#include "shadowCalculation.shader"
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
	vec4 depth = texture(sampler2D(textureDepth,basicSampler), inUV);
    vec3 fragWorldPos = WorldPosFromDepth(depth.r, inUV, uboFrameContext.inverseProjectionJittered, uboFrameContext.inverseView);

	outFragcolor = vec4(0,0,0,1);
	vec3 normal = DecodeNormalHelper(texture(sampler2D(textureNormal,basicSampler), inUV).rgb);
	if(dot(normal,normal) == 0.0) return;
	normal = normalize(normal);

	vec4 albedo = texture(sampler2D(textureAlbedo,basicSampler), inUV);
    albedo.rgb = GammaToLinear(albedo.rgb);

	vec4 material = texture(sampler2D(textureMaterial,basicSampler), inUV);
	float SSAO = texture(sampler2D(textureSSAO,basicSampler), inUV).r;
	float roughness = material.r;
	float metalness = material.g;
	
	//vec3 emissive = texture(sampler2D(samplerEmissive,basicSampler),inUV).rgb;
    vec3 emissive = vec3(0);

	// remove SSAO if not wanted
	if(PC.useSSAO == 0){
		SSAO = 1.0;
	}
	
	float outshadow = texture(sampler2D(textureShadows,basicSampler),inUV).r;
	
	LocalLightInstance lightInfo = Lights_SSBO[inLightInstance];

	vec3 lightContribution = vec3(0.0);
    vec3 res = EvalLight(lightInfo, fragWorldPos, uboFrameContext.cameraPosition.xyz, normal, roughness, albedo.rgb, metalness);
	lightContribution += res;
	
	// calculate shadow if this is a shadow light
	
    vec3 lightDir = lightInfo.position.xyz - fragWorldPos;
	

    SurfaceProperties surface;
    surface.albedo = albedo.rgb;
    surface.roughness = roughness;
    surface.metalness = metalness;
    surface.lightCol = lightInfo.color.rgb * lightInfo.color.w;
    surface.lightRadius = lightInfo.radius.x;
    surface.N = normalize(normal);
    surface.V = normalize(uboFrameContext.cameraPosition.xyz - fragWorldPos);
    surface.L = normalize(lightDir);
    surface.H = normalize(surface.L + surface.V);
    surface.dist = length(lightDir);
	
    float attenuation = UnrealFalloff(surface.dist, surface.lightRadius);
    //surface.lightCol *= attenuation;
	
    vec3 irradiance = vec3(1);
    irradiance = texture(samplerCube(irradianceCube, basicSampler), normal).rgb;
    vec3 R = normalize(reflect(-surface.V, surface.N));
	
    const float MAX_REFLECTION_LOD = 6.0;
    vec3 prefilteredColor = textureLod(samplerCube(prefilterCube, basicSampler), R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 lutVal = texture(sampler2D( brdfLUT, basicSampler), vec2(max(dot(surface.N, surface.V), 0.0), roughness)).rg;


	vec3 result = SaschaWillemsDirectionalLight(surface,
													irradiance,
													prefilteredColor,
													lutVal);
	
   
	
    float shadowValue = EvalShadowMap(lightInfo, inLightInstance, normal, fragWorldPos);
 
    result *= shadowValue;
    result *= attenuation;
	
	outFragcolor = vec4(result, albedo.a);	

}
