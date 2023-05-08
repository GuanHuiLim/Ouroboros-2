/************************************************************************************//*!
\file           MeshModel.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares a mesh object and resources for use on CPU and GPU

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <vulkan/vulkan.h>
#include "MathCommon.h"
#include "VulkanUtils.h"
#include "Mesh.h"
#include "Geometry.h"

#pragma warning( push )
#pragma warning( disable : 26451 ) // vendor overflow
#include "assimp/scene.h"
#pragma warning( pop )


inline glm::vec3 aiVector3D_to_glm(const aiVector3D& v)
{
    return glm::vec3{ v.x, v.y, v.z };
}

inline glm::mat4 aiMat4_to_glm(const aiMatrix4x4& m)
{
    return glm::mat4{
        { m.a1, m.b1, m.c1, m.d1 },
        { m.a2, m.b2, m.c2, m.d2 },
        { m.a3, m.b3, m.c3, m.d3 },
        { m.a4, m.b4, m.c4, m.d4 },	};
}

namespace oGFX
{

struct BoneNode
{
    ~BoneNode();
    std::string mName{ "BONE_NAME" };
    uint32_t m_BoneIndex{static_cast<uint32_t>(-1)};
    bool mbIsBoneNode{ false }; // Really a bone for skinning.
    BoneNode* mpParent{ nullptr };
    std::vector<BoneNode*> mChildren;

    glm::mat4 mModelSpaceLocal{1.0f};	// Local transformation of the bone in model space
    glm::mat4 mModelSpaceGlobal{1.0f};	// Global transformation of the bone in model space
};

struct BoneInverseBindPoseInfo
{
    uint32_t boneIdx{ 0 };
    glm::mat4 transform{ 1.0f };
};

constexpr uint32_t MAX_BONE_NUM = 4;
struct BoneWeight
{
    uint32_t boneIdx[MAX_BONE_NUM];
    float boneWeights[MAX_BONE_NUM];
};

struct Skeleton
{
    ~Skeleton();
    oGFX::BoneNode* m_boneNodes{ nullptr };
    std::vector<oGFX::BoneInverseBindPoseInfo>inverseBindPose;
    std::vector<oGFX::BoneWeight>boneWeights;
};

struct CPUSkeletonInstance
{
    ~CPUSkeletonInstance();
    oGFX::BoneNode* m_boneNodes{ nullptr };
};

[[nodiscard]] CPUSkeletonInstance* CreateCPUSkeleton(const Skeleton* skeleton);

} // end namespace oGFX

#define MAX_SUBMESH 64 
struct ModelFileResource
{
    ~ModelFileResource();

    std::string fileName;
    uint32_t meshResource{};
    uint32_t numSubmesh{};

    std::vector<oGFX::Vertex> vertices;
    std::vector<uint32_t> indices;

    Node* sceneInfo{ nullptr };
    uint32_t sceneMeshCount{};

    const oGFX::Skeleton* skeleton{ nullptr };
    std::unordered_map<std::string, uint32_t> strToBone;

    void ModelSceneLoad(const aiScene* scene, const aiNode& node, Node* parent,const glm::mat4 accMat);
    void ModelBoneLoad(const aiScene* scene, const aiNode& node, uint32_t vertOffset);
};

struct SubMesh
{
    std::string name;
    // Sub range of mesh
    uint32_t baseVertex{};
    uint32_t vertexCount{};
    uint32_t baseIndices{};
    uint32_t indicesCount{};
    oGFX::Sphere boundingSphere;

    // TODO: Material
};

struct gfxModel
{
    std::string name{"GARBAGE"};

    // Whole range of mesh
    uint32_t baseVertex{};
    uint32_t vertexCount{};
    uint32_t baseIndices{};
    uint32_t indicesCount{};

    std::vector<SubMesh> m_subMeshes;

    ModelFileResource* cpuModel{ nullptr };
    oGFX::Skeleton* skeleton{ nullptr };

    void destroy(VkDevice device);
private:

};

