#pragma once

#include "MathCommon.h"
#include "../shaders/shared_structs.h"
#include "BitContainer.h"

#include <vector>


struct ObjectInstance
{
    std::string name;

    // Begin These are temp until its fully integrated
    glm::vec3 position{};
    glm::vec3 scale{1.0f};
    float rot{};
    glm::vec3 rotVec{0.0f,1.0f,0.0f};

    uint32_t bindlessGlobalTextureIndex_Albedo{ 0xFFFFFFFF };
    uint32_t bindlessGlobalTextureIndex_Normal{ 0xFFFFFFFF };
    uint32_t bindlessGlobalTextureIndex_Roughness{ 0xFFFFFFFF };
    uint32_t bindlessGlobalTextureIndex_Metallic{ 0xFFFFFFFF };
    // End temp stuff

    glm::mat4x4 localToWorld{ 1.0f };

    uint32_t modelID{}; // Index for the mesh
    uint32_t entityID{}; // Unique ID for this entity instance
};

struct DecalInstance
{

};

enum class ObjectInstanceFlags : uint32_t
{
    STATIC_INSTANCE  = 0x1,  // Object will never change after initialization
    DYNAMIC_INSTANCE = 0x2,  // Object is dynamic (spatial/property)
    ACTIVE_FLAG      = 0x4,  // Object is inactive, skip for all render pass
    SHADOW_CASTER    = 0x8,  // Object casts shadows (put it into shadow render pass)
    SHADOW_RECEIVER  = 0x10, // Object receives shadows (a mask for lighting pass)
    ENABLE_ZPREPASS  = 0x20, // Object is added to Z-Prepass
    // etc
};

// TODO: Move all object storage here...
class GraphicsWorld
{
public:

    auto& GetAllObjectInstances() { return m_ObjectInstances; }
    auto& GetAllOmniLightInstances() { return m_OmniLightInstances; }

    int32_t CreateObjectInstance();
    int32_t CreateObjectInstance(ObjectInstance obj);
    ObjectInstance& GetObjectInstance(int32_t id);

private:
    BitContainer<ObjectInstance> m_ObjectInstances;
    BitContainer<OmniLightInstance> m_OmniLightInstances;
    //etc

    // + Spatial Acceleration Structures
};
