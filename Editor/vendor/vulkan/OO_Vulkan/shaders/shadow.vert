#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require

#include "shared_structs.h"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inCol;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

layout(location = 15) in uvec4 inInstanceData; // (id, material, unused, unused)

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};

//layout (set = 2, binding = 0) uniform sampler2D textureDesArr[];

layout(std430, set = 0, binding = 3) readonly buffer GPUScene
{
	GPUTransform GPUScene_SSBO[];
};

layout(push_constant) uniform PushLight
{
	mat4 instanceMatrix;
	vec3 pos;
}pushLight;

mat4 GPUTransformToMatrix4x4(const in GPUTransform m)
{
    return mat4(m.row0.x, m.row1.x, m.row2.x, 0.0,
              m.row0.y, m.row1.y, m.row2.y, 0.0,
              m.row0.z, m.row1.z, m.row2.z, 0.0,
              m.row0.w, m.row1.w, m.row2.w, 1.0);
}

layout(location = 0) out vec4 outPos;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outCol;

layout (location = 15)flat out uvec4 outInstanceData;

//layout(location = 1) out flat  struct
//{
//  ivec4 maps;
//  //int albedo;	
// //int normal;	
// //int occlusion;
// //int roughness;
//}outTexIndex;

// WR Note: Sending too much stuff from VS to FS can result in bottleneck...

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
	//decode the matrix into transform matrix
	const mat4 dInsMatrix = GPUTransformToMatrix4x4(GPUScene_SSBO[inInstanceData.x]);
	// inefficient
	const mat4 inverseMat = inverse(dInsMatrix);
	
	outLightData.localEyePos = vec3(inverseMat* vec4(vec3(uboFrameContext.cameraPosition),1.0));
	
	outLightData.localLightPos = vec3(inverseMat * vec4(pushLight.pos, 1.0));

	vec3 binormal = normalize(cross(inTangent, inNormal));
	
	outLightData.btn = mat3(inTangent, binormal, mat3(transpose(inverseMat))*inNormal);

	//outViewVec = -vec3(uboFrameContext.view[3]);	
	outLightData.localVertexPos = inPos;

	outPos = dInsMatrix * vec4(inPos,1.0);
	gl_Position = pushLight.instanceMatrix * outPos;

	//outTexIndex.maps.x = instanceTexIndex;
	//outTexIndex.maps.y = instanceNormalTexIndex;
	//outTexIndex.maps.z = instanceOcclusionTexIndex;
	//outTexIndex.maps.w = instanceRoughnessTexIndex;
	outUV = inUV;
	outCol = inCol;

	outInstanceData = inInstanceData;
}
