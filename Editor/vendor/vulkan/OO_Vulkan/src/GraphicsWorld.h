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
#include "VulkanUtils.h"
#include "Font.h"

#include "imgui/imgui.h"
#include <vector>
#include <array>

// pos windows
#undef TRANSPARENT 
enum class ObjectInstanceFlags : uint32_t 
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
ENUM_OPERATORS_GEN(ObjectInstanceFlags, uint32_t)

enum class UIInstanceFlags : uint32_t
{
    RENDER_ENABLED   = 0x1,  // Object will never change after initialization
    WORLD_SPACE_UI   = 0x2,  // Object is worldspace
    TEXT_INSTANCE    = 0x4,  // Object is inactive, skip for all render pass
    SPRITE_INSTANCE  = 0x8,  // Object casts shadows (put it into shadow render pass)  
    SCREEN_SPACE     = 0x10, // Object rendered without depth    
};
ENUM_OPERATORS_GEN(UIInstanceFlags, uint32_t)


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
    uint32_t bindlessGlobalTextureIndex_Emissive{ 0xFFFFFFFF };
    // End temp stuff

    glm::vec4 emissiveColour{};
    uint8_t instanceData{ 0 }; // Per Instance unique data (not to be in material)
    glm::mat4x4 localToWorld{ 1.0f };
    ObjectInstanceFlags flags{static_cast<ObjectInstanceFlags>(ObjectInstanceFlags::RENDER_ENABLED 
        | ObjectInstanceFlags::SHADOW_RECEIVER 
        | ObjectInstanceFlags::SHADOW_CASTER)};

    // helper functions
    void SetShadowCaster(bool s);
    void SetShadowReciever(bool s);
    void SetSkinned(bool s);
    void SetShadowEnabled(bool s);
    void SetRenderEnabled(bool s);

    bool isSkinned();
    bool isShadowEnabled();
    bool isShadowCaster();
    bool isRenderable();
    bool isDynamic();
    bool isTransparent();

    std::vector<glm::mat4> bones;

    uint32_t modelID{}; // Index for the mesh
    std::bitset<MAX_SUBMESH>submesh;// submeshes to draw
    uint32_t entityID{}; // Unique ID for this entity instance
};

struct UIInstance
{
    std::string name;

    // Begin These are temp until its fully integrated 
    uint32_t bindlessGlobalTextureIndex_Albedo{ 0xFFFFFFFF }; // waiting for material system xd..
    // End temp stuff
    std::string textData{"SAMPLE TEXT"};
    glm::vec4 colour{1.0f};

    oGFX::FontFormatting format;
    oGFX::Font* fontAsset{ nullptr };

    uint8_t instanceData{ 0 }; // Per Instance unique data (not to be in material)
    glm::mat4x4 localToWorld{ 1.0f };
    UIInstanceFlags flags{static_cast<UIInstanceFlags>(
        UIInstanceFlags::RENDER_ENABLED 
        | UIInstanceFlags::WORLD_SPACE_UI)};

    void SetText(bool s);
    bool isText();
    void SetScreenSpace(bool s);
    bool isScreenSpace();

    void SetRenderEnabled(bool s);
    bool isRenderable();

    uint32_t entityID{}; // Unique ID for this entity instance
};

struct ParticleData
{
    glm::mat4 transform{1.0f};
    glm::vec4 colour{1.0f};
    glm::ivec4 instanceData; // EntityID, flags  ,abledo norm, roughness metal
};

struct UIData
{
    glm::mat4 transform{1.0f};
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
bool GetCastsShadows(const LocalLightInstance& l);
void SetCastsShadows(OmniLightInstance& l, bool s);
bool GetCastsShadows(const OmniLightInstance& l);
void SetCastsShadows(SpotLightInstance& l, bool s);
bool GetCastsShadows(const SpotLightInstance& l);

template <typename T>
inline void SetLightEnabled(T& l, bool s) {
    reinterpret_cast<LocalLightInstance*>(&l)->info.z = s ? 1 : -1;
}
template <typename T>
inline bool GetLightEnabled(const T& l) {
   return reinterpret_cast<const LocalLightInstance*>(&l)->info.z == 1 ? true : false;
}


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
    auto& GetAllUIInstances() { return m_UIInstances; }

    int32_t CreateObjectInstance();
    int32_t CreateObjectInstance(ObjectInstance obj);
    ObjectInstance& GetObjectInstance(int32_t id);
    void DestroyObjectInstance(int32_t id);
    void ClearObjectInstances();

    int32_t CreateUIInstance();
    int32_t CreateUIInstance(UIInstance obj);
    UIInstance& GetUIInstance(int32_t id);
    void DestroyUIInstance(int32_t id);
    void ClearUIInstances();

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
    std::array<bool, 2> shouldRenderCamera{ true, false };
    std::array<Camera, 2>cameras;
    std::array<int32_t, 2>targetIDs{ -1,-1 };
    std::array<ImTextureID, 2>imguiID{};

    // TODO: Fix Me ! This is for testing
    DecalInstance m_HardcodedDecalInstance;

    struct SSAOSettings
    {
        float radius = 0.5f;
        float bias = 0.025f;
        float intensity = 1.0f;
        uint32_t samples = 8;
    }ssaoSettings{};

    struct LightingSettings
    {
        float ambient = 0.002f;
        float maxBias = 0.0001f;
        float biasMultiplier = 0.002f;
        float specularModifier = 16.0f;
    }lightSettings{};

    struct BloomSettings
    {
        float threshold = 10.0f;
        float softThreshold = 0.01f;
    }bloomSettings{};

    struct ColourCorrectionSettings
    {
        float highlightThreshold = 1.0f;
        float shadowThreshold = 0.0f;
        glm::vec4 shadowColour{};
        glm::vec4 midtonesColour{};
        glm::vec4 highlightColour{};
    }colourSettings{};

    struct VignetteSettings
    {
        vec4 colour;
        float innerRadius;
        float outerRadius;
    }vignetteSettings{};

    friend class VulkanRenderer;
private:
    int32_t m_entityCount{};
    BitContainer<ObjectInstance> m_ObjectInstances;
    int32_t m_uiCount{};
    BitContainer<UIInstance> m_UIInstances;
    int32_t m_lightCount{};
    BitContainer<OmniLightInstance> m_OmniLightInstances;
    int32_t m_emitterCount{};
    BitContainer<EmitterInstance> m_EmitterInstances;
    bool initialized = false;

    //etc

    // + Spatial Acceleration Structures
    // + Culling object BV against frustum
};
