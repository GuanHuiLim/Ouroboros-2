layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inCol;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

// Instanced attributes		
//layout (location = 4) in vec4 instancePos;
//layout (location = 5) in vec4 instanceRot;
//layout (location = 6) in vec4 instanceScale;
//layout (location = 5) in mat4 instanceMatrix; // eats location 5-8
//
//layout (location = 9) in int instanceTexIndex;
//layout (location = 10) in int instanceNormalTexIndex;
//layout (location = 11) in int instanceOcclusionTexIndex;
//layout (location = 12) in int instanceRoughnessTexIndex;
layout(location = 15) in uvec4 inInstanceData;

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};

layout(push_constant)uniform PushLight
{
	vec3 pos;
}pushLight;


layout(location = 0) out vec2 outUV;

layout(location = 1) out flat  struct
{
	ivec4 maps;
	//int albedo;	
	//int normal;	
	//int occlusion;
	//int roughness;
}outTexIndex;

layout(location = 4)out vec3 outViewVec;

layout(location = 7) out struct 
{
	mat3 btn;
	vec3 vertCol;
	vec3 localVertexPos;
	vec3 localLightPos;
	vec3 localEyePos;
}outLightData;

void main()
{
	// inefficient
	mat4 instanceMatrix = mat4(1.0);
	mat4 inverseMat = inverse(instanceMatrix);
	
	outLightData.localEyePos = vec3(inverseMat* vec4(vec3(uboFrameContext.cameraPosition),1.0));
	
	outLightData.localLightPos = vec3(inverseMat * vec4(pushLight.pos, 1.0));

	vec3 binormal = normalize(cross(inTangent, inNormal));
	outLightData.btn = mat3(inTangent, binormal, inNormal);


	//outViewVec = -vec3(uboFrameContext.view[3]);	
	outLightData.localVertexPos = inPos;

	vec4 pos = instanceMatrix * vec4(inPos,1.0);
	gl_Position = uboFrameContext.projection * uboFrameContext.view * pos;

	//outTexIndex.maps.x = instanceTexIndex;
	//outTexIndex.maps.y = instanceNormalTexIndex;
	//outTexIndex.maps.z = instanceOcclusionTexIndex;
	//outTexIndex.maps.w = instanceRoughnessTexIndex;
	outUV = inUV;
}
