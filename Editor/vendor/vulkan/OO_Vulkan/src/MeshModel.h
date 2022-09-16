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

class MeshContainer
{
public:
    MeshContainer() = default;
    MeshContainer(std::vector<Mesh> newMeshList);

    size_t getMeshCount();
    Mesh *getMesh(size_t index);

    const glm::mat4& getModel();
    void setModel(glm::mat4 newModel);

    void destroyMeshModel();

    static std::vector<std::string> LoadMaterials(const aiScene *scene);
    static std::vector<Mesh> LoadNode(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool commandPool,
        aiNode *node, const aiScene *scene, std::vector<int> matToTex);
    static Mesh LoadMesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice, VkQueue transferQueue, VkCommandPool commandPool,
        aiMesh *mesh, const aiScene *scene, std::vector<int> matToTex);

private:
    std::vector<Mesh> meshList;
    glm::mat4 model{ 1.0f };
};

struct BoneInfo
{
    int id;

    /*offset matrix transforms vertex from model space to bone space*/
    glm::mat4 offset;
};

struct Bone
{
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;
};

struct Model
{
    uint32_t gfxIndex{};
    std::string fileName;
    std::vector<oGFX::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<std::string, BoneInfo> strToBone;
    std::vector<Bone> bones;
    uint32_t boneCnt{};
    Sphere s{};
    AABB aabb{};
};

struct gfxModel
{
    struct Vertices
    {
        uint32_t count{};
        uint32_t offset{};
        VkBuffer buffer{};
        VkDeviceMemory memory{};
    } vertices{};

    struct Indices
    {
        uint32_t count{};
        uint32_t offset{};
        VkBuffer buffer{};
        VkDeviceMemory memory{};
    } indices{};

    struct Textures
    {
        uint32_t albedo{};
        uint32_t normal{};
        uint32_t occlusion{};
        uint32_t roughness{};
    }textures{};

    Model* cpuModel{ nullptr };

    void destroy(VkDevice device);

    void loadNode(Node* parent, const aiScene* scene, const aiNode& node, uint32_t nodeIndex,
                 Model& cpumodel);

    void updateOffsets(uint32_t idxOffset, uint32_t vertOffset);

    std::vector<Node*> nodes;
    uint32_t meshCount{};

private:
    oGFX::Mesh* processMesh(aiMesh* mesh, const aiScene* scene, std::vector<oGFX::Vertex>& vertices, std::vector<uint32_t>& indices);

};

