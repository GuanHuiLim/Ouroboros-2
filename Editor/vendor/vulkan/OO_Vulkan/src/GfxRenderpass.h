/************************************************************************************//*!
\file           GfxRenderpass.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              A renderpass interface object

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include "Profiling.h"
#include "rhi/CommandList.h"
#include "FramebufferBuilder.h"
#include "VulkanRenderpass.h"

#include <vector>
#include <memory>
#include <cassert>
#include <iostream>
#include <type_traits>

enum GBufferAttachmentIndex
{
    //POSITION = 0,
    NORMAL ,
    ALBEDO ,
    MATERIAL ,
    EMISSIVE ,
    ENTITY_ID,
    DEPTH ,

    MAX_ATTACHMENTS,
    TOTAL_COLOR_ATTACHMENTS = MAX_ATTACHMENTS - 1
};

class GfxRenderpass
{
public:
    virtual ~GfxRenderpass() = default;
    virtual void Init() = 0;
    virtual void Draw(const VkCommandBuffer cmdlist) = 0;
    virtual void Shutdown() = 0;
    // Called once upon init    
    virtual void CreatePSO() {};
    // Called once per context, for render graph. "false" means this render pass is unused/skipped.
    virtual bool SetupDependencies() = 0;
    uint8_t m_Index{ 0xFF };
    VkCommandBuffer lastCmd{VK_NULL_HANDLE};
};

class RenderPassDatabase
{
public:
    static RenderPassDatabase* Get();
    static void Shutdown();
    void RegisterRenderPass(std::unique_ptr<GfxRenderpass>&& renderPass);
    void RegisterRenderPass(GfxRenderpass* renderPass);
    static void ReloadAllShaders();

    // Call this once to call "Init()" on all registered render passes.
    // Take note the order of initialization is undefined.
    static void InitAllRegisteredPasses();

    static void ShutdownAllRegisteredPasses();
    // TODO: Proper C++ Fix needed.
    template<typename T_PASS>
    static inline T_PASS* GetRenderPass()
    {
        for (auto& pass : Get()->m_AllRawRenderPasses)
        {
            GfxRenderpass* base = pass;
            if constexpr (true) // TODO FIX ME
            {
                // This is bad ! Leaving this here as a fallback in case shit happens
                if (T_PASS* derived = dynamic_cast<T_PASS*>(base))
                {
                    return derived;
                }
            }
        }
        return nullptr;
    }

private:
    static RenderPassDatabase* ms_renderpass;
    std::vector<GfxRenderpass*> m_AllRawRenderPasses;
    uint8_t m_RegisteredRenderPasses{ 0 };
};


#define DECLARE_RENDERPASS_SINGLETON(pass)\
static pass* Get()\
{\
    assert(m_pass);\
    return m_pass;\
}\
inline static pass* m_pass{nullptr};


// function declares and creates a renderpass automatically at runtime
#define DECLARE_RENDERPASS(pass)\
pass g_gfx##pass;\
GfxRenderpass* g_##pass{ &g_gfx##pass };
//namespace DeclareRenderPass_ns\
//{\
//struct DeclareRenderPass_##pass\
//     {\
//            DeclareRenderPass_##pass()\
//            {\
//                auto ptr = new pass;\
//                auto rdb =  RenderPassDatabase::Get();\
//                rdb->RegisterRenderPass(ptr);\
//            }\
//    }g_DeclareRenderPass_##pass;\
//}

#define RENDERPASS_USAGE(pass)\
extern GfxRenderpass* g_##pass


#define BIGCANCER(pass) DeclareRenderPass_ns::g_DeclareRenderPass_##pass
