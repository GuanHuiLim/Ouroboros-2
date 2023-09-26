#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : require

#include "shared_structs.h"
#include "instancing.shader"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inCol;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

layout(location = 15) in uvec4 inInstanceData;


#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
	FrameContext uboFrameContext;
};


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
	const uint localToWorldMatrixIndex = inInstanceData.x;

	//decode the matrix into transform matrix
	const mat4 dInsMatrix = GPUTransformToMatrix4x4(GPUScene_SSBO[localToWorldMatrixIndex]);
	GPUObjectInformation objectInfo = GPUobjectInfo[inInstanceData.x];
	// inefficient

	vec4 outPosition;
	bool skinned = UnpackSkinned(inInstanceData.y);
    if(skinned)
	{
        uint thisVert = gl_VertexIndex - gl_BaseVertex;
        BoneWeight boneInfo = GetBoneWeights(thisVert + objectInfo.boneWeightsOffset);
        uvec4 boneIndices = UnpackBoneIndices(boneInfo);
        vec4 boneWeights = UnpackBoneWeights(boneInfo);
		
		mat4x4 boneToModel; // what do i do with this
		outPosition = ComputeSkinnedVertexPosition(dInsMatrix,inPos
		, boneIndices, boneWeights
		,objectInfo.boneStartIdx,boneToModel);
	}
	else
	{
		outPosition = dInsMatrix * vec4(inPos,1.0);
	}

	gl_Position = uboFrameContext.viewProjection * outPosition;
	
}
