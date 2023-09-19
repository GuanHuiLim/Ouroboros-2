layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outFragcolor;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};

#include "shared_structs.h"
layout (set = 0, binding = 0) uniform sampler basicSampler; 
layout (set = 0, binding = 1) uniform texture2D samplerDepth; // also constructs position
layout (set = 0, binding = 2) uniform texture2D samplerNormal;
layout (set = 0, binding = 3) uniform texture2D samplerNoise;

layout(std430, set = 0, binding = 4) readonly buffer RandomVectors
{
	vec3 randomVectors[];
};

layout( push_constant ) uniform pc
{
	SSAOPC PC;
};

#include "shader_utility.shader"

void main()
{
	// Get G-Buffer values
	vec4 depth = texture(sampler2D(samplerDepth,basicSampler), inUV);
	vec3 fragPos = ViewPosFromDepth(depth.r,inUV,uboFrameContext.inverseProjection).xyz;

	vec3 normal = texture(sampler2D(samplerNormal,basicSampler), inUV).rgb;

	vec2 noiseScale = vec2(float(PC.screenDim.x)/PC.sampleDim.x, float(PC.screenDim.y)/PC.sampleDim.y);
	vec3 randomVec = texture(sampler2D(samplerNoise,basicSampler), inUV * noiseScale).xyz;

	vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN       = mat3(tangent, bitangent, normal);
	
	float occlusion = 0.0;
	float radius = PC.radius;
	float bias = PC.bias;
	int kernelSize = randomVectors.length();
	kernelSize = int(PC.numSamples);
	for(int i = 0; i < kernelSize; ++i)
	{
	    // get sample position
	    vec3 samplePos = TBN * randomVectors[i].xyz; // from tangent to view-space
	    samplePos = fragPos + samplePos * radius; 
	    
	    vec4 offset = vec4(samplePos, 1.0);
		offset      = uboFrameContext.projection * offset;    // from view to clip-space
		offset.xyz /= offset.w;               // perspective divide
		offset.xy  = offset.xy * 0.5 + 0.5; // transform to range 0.0 - 1.0
		offset.y = 1.0 - offset.y;
		// once again we ignore z because vulkan
		float sampleDepth = texture(sampler2D(samplerDepth,basicSampler), offset.xy).r;
		vec3 world = ViewPosFromDepth(sampleDepth,offset.xy,uboFrameContext.inverseProjection).xyz;
		
		sampleDepth = world.z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion       += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	} 

	// normalize
	occlusion = (occlusion / kernelSize);
	occlusion = 1.0 - occlusion;
	occlusion = pow(occlusion,PC.intensity);
	//occlusion = smoothstep(occlusion,0.0,PC.intensity.x);
	
	outFragcolor = vec4(occlusion);

}
