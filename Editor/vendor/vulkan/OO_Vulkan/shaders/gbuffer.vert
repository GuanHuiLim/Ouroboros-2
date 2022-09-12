#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require

#include "shared_structs.h"
#include "instancing.shader"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;
layout(location = 15) in uvec4 inInstanceData;

// Note: Sending too much stuff from VS to FS can result in bottleneck...
layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec2 outUV;
layout(location = 2) out vec3 outColor;
layout(location = 15) flat out uvec4 outInstanceData;
layout(location = 7) out struct
{
	mat3 btn;
}outLightData;

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

/* // TODO: Non-precomputed skinning
layout(std430, set = 0, binding = 4) readonly buffer BoneBuffer
{
	mat4x4 BoneBuffer_SSBO[];
};
*/

layout(push_constant) uniform PushLight
{
	mat4 instanceMatrix;
	vec3 pos;
}pushLight;

void main()
{
	const uint localToWorldMatrixIndex = inInstanceData.x;

	//decode the matrix into transform matrix
	const mat4 dInsMatrix = GPUTransformToMatrix4x4(GPUScene_SSBO[localToWorldMatrixIndex]);
	// inefficient
	const mat4 inverseMat = inverse(dInsMatrix);

	vec3 binormal = normalize(cross(inTangent, inNormal));
	
	outLightData.btn = mat3(inTangent, binormal, mat3(transpose(inverseMat))*inNormal);

	outPosition = dInsMatrix * vec4(inPosition,1.0);
	gl_Position = uboFrameContext.viewProjection * outPosition;
	
	outUV = inUV;
	outColor = inColor;
	outInstanceData = inInstanceData;
}
