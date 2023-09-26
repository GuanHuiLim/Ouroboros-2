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
layout(location = 3) out flat int outEntityID;
layout(location = 4) out flat vec4 outEmissive;
layout(location = 7) out struct
{
	vec3 b;
	vec3 t;
	vec3 n;
}outLightData;
layout(location = 15) flat out uvec4 outInstanceData;

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

#include "skinning.shader"

layout(std430, set = 0, binding = 5) readonly buffer GPUobject
{
	GPUObjectInformation GPUobjectInfo[];
};

void main()
{

	const uint instanceIndex = inInstanceData.x;

	GPUObjectInformation objectInfo = GPUobjectInfo[inInstanceData.x];
	outEntityID = objectInfo.entityID;
	outEmissive = objectInfo.emissiveColour;
	//decode the matrix into transform matrix
	mat4 dInsMatrix = GPUTransformToMatrix4x4(GPUScene_SSBO[instanceIndex]);
	
	// inefficient
	mat3 L2W = mat3(dInsMatrix);//inverse(dInsMatrix);
	//L2W = mat3(1.0);

	vec3 NN = normalize(inNormal);
	vec3 NT = normalize(inTangent);
	vec3 NB = cross(NN, NT);
	
	mat3 invTranspose = mat3(GPUTransformToInverseTransposeMatrix4x4(GPUScene_SSBO[instanceIndex]));

	vec3 T = normalize(invTranspose * vec3(NT)).xyz;
	vec3 B = normalize(invTranspose * vec3(NB)).xyz;
	vec3 N = normalize(invTranspose * vec3(NN)).xyz;

	outLightData.b = B;
	outLightData.t = T;
	outLightData.n = N;


	bool skinned = UnpackSkinned(inInstanceData.y);
    if(skinned)
	{
        uint thisVert = gl_VertexIndex - gl_BaseVertex;
        BoneWeight boneInfo = GetBoneWeights(thisVert + objectInfo.boneWeightsOffset);
		
        uvec4 boneIndices = UnpackBoneIndices(boneInfo);		
        vec4 boneWeights = UnpackBoneWeights(boneInfo);
		
		mat4x4 boneToModel; // what do i do with this
		outPosition = ComputeSkinnedVertexPosition(dInsMatrix,inPosition
													, boneIndices, boneWeights
													,objectInfo.boneStartIdx,boneToModel);
		mat3 inverseTransformBone = transpose(mat3(inverse(boneToModel)));
		outLightData.n = normalize(inverseTransformBone*NN);
	}
	else
	{
		outPosition = dInsMatrix * vec4(inPosition,1.0);
	}

	gl_Position = uboFrameContext.viewProjection * outPosition;
	
	outUV = inUV;
	outColor = inColor;
	outInstanceData = inInstanceData;
}
