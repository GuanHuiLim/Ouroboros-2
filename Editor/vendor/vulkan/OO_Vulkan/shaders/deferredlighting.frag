layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

 #extension GL_EXT_samplerless_texture_functions : require


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

layout (set = 0, binding = 9) uniform sampler cubeSampler;
layout (set = 0, binding = 10) uniform textureCube irradianceCube;
layout (set = 0, binding = 11)uniform textureCube prefilterCube;
layout (set = 0, binding = 12)uniform texture2D brdfLUT;
layout (set = 0, binding = 13)uniform samplerShadow shadowSampler;


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
	
	// Get G-Buffer values
	float depth = texelFetch(textureDepth,ivec2(gl_FragCoord.xy) , 0).r;

	vec4 albedo = texture(sampler2D(textureAlbedo,basicSampler), inUV);
	float ambient = PC.ambient;
	vec3 fragPos = WorldPosFromDepth(depth.r,inUV,uboFrameContext.inverseProjectionJittered,uboFrameContext.inverseView);
	vec3 normal = DecodeNormalHelper(texture(sampler2D(textureNormal,basicSampler), inUV).rgb);
	normal = normalize(normal);
	
    albedo.rgb = GammaToLinear(albedo.rgb);
	
    //albedo.rgb = vec3(1,1,0);

	vec4 material = texture(sampler2D(textureMaterial,basicSampler), inUV);
	float SSAO = texture(sampler2D(textureSSAO,basicSampler), inUV).r;
    float roughness = clamp(material.r,0.01,1.0);
    float metalness = clamp(material.g,0.01,1.0);

    if (PC.useSSAO == 0)
    {
        SSAO = 1;
    }
	
	// Ambient part
	vec3 emissive = texture(sampler2D(textureEmissive,basicSampler),inUV).rgb;
	//emissive = vec3(0);

    vec4 lightCol = PC.lightColorInten;
	
    vec3 lightDir = -normalize(PC.directionalLight.xyz);
	
    SurfaceProperties surface;
    surface.albedo = albedo.rgb;
    surface.roughness = roughness;
    surface.metalness = metalness;
    surface.lightCol = lightCol.rgb * lightCol.w;
    surface.lightRadius = 0.0f;
    surface.N = normalize(normal);
    surface.V = normalize(uboFrameContext.cameraPosition.xyz - fragPos);
    surface.L = normalize(lightDir);
    surface.H = normalize(surface.L + surface.V);
    surface.dist = length((lightDir) - fragPos);
	
    vec3 irradiance = vec3(1);
    irradiance = texture(samplerCube(irradianceCube, basicSampler), normal).rgb;
    irradiance *= PC.ambient;
    vec3 R = normalize(reflect(-surface.V, surface.N));
	
    const float MAX_REFLECTION_LOD = 6.0;
    vec3 prefilteredColor = textureLod(samplerCube(prefilterCube, basicSampler), R, roughness * MAX_REFLECTION_LOD).rgb;
    prefilteredColor *= PC.ambient * SSAO.rrr;
	
    vec2 lutVal = texture(sampler2D( brdfLUT, basicSampler),vec2(max(dot(surface.N, surface.V), 0.0), roughness)).rg;
	
    //vec3 result = EvalDirectionalLight(surface, irradiance, prefilteredColor,lutVal);
	vec3 result = emissive+ SaschaWillemsDirectionalLight(surface,
													irradiance,
													prefilteredColor,
													lutVal);

    outFragcolor = vec4(result.rgb, albedo.a);
}
