/************************************************************************************//*!
\file           GraphicsWorld.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Declares graphics world, a wrapper for objects that require to be rendered.
    This is used as the main tnerface between the renderer and external engine

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "MathCommon.h"
#include "../shaders/shared_structs.h"
#include "BitContainer.h"
#include "MeshModel.h"
#include "Camera.h"
#include "VulkanTexture.h"

#include "imgui/imgui.h"
#include <vector>
#include <array>

// pos windows
#undef TRANSPARENT 
enum ObjectInstanceFlags : uint32_t // fuck enum class
{
    RENDER_ENABLED   = 0x1,  // Object will never change after initialization
    STATIC_INSTANCE  = 0x2,  // Object is dynamic (spatial/property)
    DYNAMIC_INSTANCE = 0x4,  // Object is inactive, skip for all render pass
    ACTIVE_FLAG      = 0x8,  // Object casts shadows (put it into shadow render pass)
    SHADOW_CASTER    = 0x10, // Object receives shadows (a mask for lighting pass)
    SHADOW_RECEIVER  = 0x20, // Object is added to Z-Prepass
    ENABLE_ZPREPASS  = 0x40, // Object is added to forward pass
    TRANSPARENT      = 0x80, // Object is an emitter ??s
    EMITTER          = 0x100, // Object is added to skinned pass
    SKINNED          = 0x200, // Object can project shadows
    SHADOW_ENABLED   = 0x400, // Object is rendered
                                // etc
};


inline ObjectInstanceFlags operator|(ObjectInstanceFlags a, ObjectInstanceFlags b)
{
    return static_cast<ObjectInstanceFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline ObjectInstanceFlags operator&(ObjectInstanceFlags a, ObjectInstanceFlags b)
{
    return static_cast<ObjectInstanceFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline ObjectInstanceFlags operator~(ObjectInstanceFlags a)
{
    return static_cast<ObjectInstanceFlags>(~static_cast<uint32_t>(a));
}

//CHAR_BIT * sizeof(uint64_t)
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

    uint8_t instanceData{ 0 }; // Per Instance unique data (not to be in material)
    glm::mat4x4 localToWorld{ 1.0f };
    ObjectInstanceFlags flags{static_cast<ObjectInstanceFlags>(RENDER_ENABLED | SHADOW_RECEIVER | SHADOW_CASTER)};

    // helper functions
    void SetShadowCaster(bool s);
    void SetShadowReciever(bool s);
    void SetSkinned(bool s);
    void SetShadowEnabled(bool s);
    void SetRenderEnabled(bool s);

    bool isSkinned();
    bool isShadowEnabled();
    bool isRenderable();

    std::vector<glm::mat4> bones;

    uint32_t modelID{}; // Index for the mesh
    std::bitset<MAX_SUBMESH>submesh;// submeshes to draw
    uint32_t entityID{}; // Unique ID for this entity instance
};

struct ParticleData
{
    glm::mat4 transform{1.0f};
    glm::vec4 colour{1.0f};
    glm::ivec4 instanceData; // EntityID, flags  ,abledo norm, roughness metal
};

struct EmitterInstance
{
    uint32_t bindlessGlobalTextureIndex_Albedo{ 0xFFFFFFFF };
    uint32_t bindlessGlobalTextureIndex_Normal{ 0xFFFFFFFF };
    uint32_t bindlessGlobalTextureIndex_Roughness{ 0xFFFFFFFF };
    uint32_t bindlessGlobalTextureIndex_Metallic{ 0xFFFFFFFF };

    glm::mat4x4 localToWorld{ 1.0f };

    uint32_t modelID{}; // Index for the mesh
    std::bitset<MAX_SUBMESH>submesh;// submeshes to draw
    uint32_t entityID{}; // Unique ID for this entity instance

    std::vector<ParticleData> particles;
};

void SetCastsShadows(LocalLightInstance& l, bool s);
bool GetCastsShadows(LocalLightInstance& l);
void SetCastsShadows(OmniLightInstance& l, bool s);
bool GetCastsShadows(OmniLightInstance& l);
void SetCastsShadows(SpotLightInstance& l, bool s);
bool GetCastsShadows(SpotLightInstance& l);


struct DecalInstance
{
    glm::mat4x4 decalViewProjection{ 1.0f };
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 direction{ 0.0f, -1.0f, 0.0f };
    float rotation{ 0.0f };
    float projectorSize{ 1.0f };
    float testVar0{ 1.0f };
    float testVar1{ 1.0f };
    float nearZ{ -1.0f };
};

// TODO: Move all object storage here...
class GraphicsWorld
{
public:
    
    // Call this at the beginning of the frame
    void BeginFrame();
    // Call this at the end of the frame
    void EndFrame();

    auto& GetAllObjectInstances() { return m_ObjectInstances; }
    auto& GetAllOmniLightInstances() { return m_OmniLightInstances; }
    auto& GetAllEmitterInstances() { return m_EmitterInstances; }

    int32_t CreateObjectInstance();
    int32_t CreateObjectInstance(ObjectInstance obj);
    ObjectInstance& GetObjectInstance(int32_t id);
    void DestroyObjectInstance(int32_t id);
    void ClearObjectInstances();

    int32_t CreateLightInstance();
    int32_t CreateLightInstance(OmniLightInstance obj);
    OmniLightInstance& GetLightInstance(int32_t id);
    void DestroyLightInstance(int32_t id);
    void ClearLightInstances();

    int32_t CreateEmitterInstance();
    int32_t CreateEmitterInstance(EmitterInstance obj);
    EmitterInstance& GetEmitterInstance(int32_t id);
    void DestroyEmitterInstance(int32_t id);
    void ClearEmitterInstances();

    void SubmitParticles(std::vector<ParticleData>& particleData, uint32_t cnt, int32_t modelID);

    uint32_t numCameras = 1;
    std::array<Camera, 2>cameras;
    std::array<int32_t, 2>targetIDs{ -1,-1 };
    std::array<ImTextureID, 2>imguiID{};

    // TODO: Fix Me ! This is for testing
    DecalInstance m_HardcodedDecalInstance;

    struct SSAOSettings
    {
        float radius = 0.5f;
        float bias = 0.025f;
        uint32_t samples = 8;
    }ssaoSettings{};

    struct LightingSettings
    {
        float ambient = 0.002f;
        float maxBias = 0.0001f;
        float biasMultiplier = 0.002f;
    }lightSettings{};

    friend class VulkanRenderer;
private:
    int32_t m_entityCount{};
    BitContainer<ObjectInstance> m_ObjectInstances;
    int32_t m_lightCount{};
    BitContainer<OmniLightInstance> m_OmniLightInstances;
    int32_t m_emitterCount{};
    BitContainer<EmitterInstance> m_EmitterInstances;
    bool initialized = false;

    //etc

    // + Spatial Acceleration Structures
    // + Culling object BV against frustum
};
