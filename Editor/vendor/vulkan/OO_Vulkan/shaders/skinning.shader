#ifndef _SKINNING_SHADER_H_
#define _SKINNING_SHADER_H_

 // TODO: Non-precomputed skinning
layout(std430, set = 0, binding = 4) readonly buffer BoneBuffer
{
    mat4x4 BoneBuffer_SSBO[];
};


mat4x4 GetBoneMatrix(uint boneIndex)
{
    // TODO - Get the bone matrix from some buffer/etc
    // Something like: layout(set = S, binding = B) readonly buffer GlobalBoneBuffer { mat4x4 GlobalBoneBuffer_SSBO[]; };
    return BoneBuffer_SSBO[boneIndex];
}

// Trivial implementation.
vec4 ComputeSkinnedVertexPosition(
    const in mat4x4 modelToWorld,
    const in vec3 vertexPosition,
    const in uvec4 boneIndexes,
    const in vec4 boneWeights,
    const in uint boneOffset,
    out mat4x4 outBoneToModel
    )
{
    mat4x4 bones[4];
    bones[0] = GetBoneMatrix(boneIndexes[0]+ boneOffset);
    bones[1] = GetBoneMatrix(boneIndexes[1]+ boneOffset);
    bones[2] = GetBoneMatrix(boneIndexes[2]+ boneOffset);
    bones[3] = GetBoneMatrix(boneIndexes[3]+ boneOffset);

    // Aks: LBS
    const mat4x4 boneToModel = bones[0] * boneWeights[0]
                             + bones[1] * boneWeights[1]
                             + bones[2] * boneWeights[2]
                             + bones[3] * boneWeights[3];
    outBoneToModel = boneToModel;

    const vec4 result = modelToWorld * boneToModel * vec4(vertexPosition, 1.0);
    return result;
}

// Not implemented: Skinning Transformation of Vertex Normals.

#endif//INCLUDE_GUARD
