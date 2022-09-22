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


struct ModelData
{
    ~ModelData();

    std::string fileName;
    std::vector<uint32_t> gfxMeshIndices;

    std::vector<oGFX::Vertex> vertices;
    std::vector<uint32_t> indices;

    Node* sceneInfo{ nullptr };
    uint32_t sceneMeshCount{};


    std::unordered_map<std::string, BoneInfo> strToBone;
    std::vector<Bone> bones;
    uint32_t boneCnt{};

    void ModelSceneLoad(const aiScene* scene, const aiNode& node, Node* parent,const glm::mat4 accMat);
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

    ModelData* cpuModel{ nullptr };
    std::vector<Node*> nodes;
    std::string name;   
    oGFX::Mesh* mesh{ nullptr };

    void destroy(VkDevice device);

    void loadNode(Node* parent, const aiScene* scene, const aiNode& node, uint32_t nodeIndex,
        ModelData& cpumodel);
    void loadNode(const aiScene* scene,const aiNode& node, Node* parent, ModelData& cpuModel, glm::mat4 accMat);

    void updateOffsets(uint32_t idxOffset, uint32_t vertOffset);
    oGFX::Mesh* processMesh(aiMesh* mesh, const aiScene* scene, std::vector<oGFX::Vertex>& vertices, std::vector<uint32_t>& indices);
private:

};

