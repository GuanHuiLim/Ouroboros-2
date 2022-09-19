#ifndef _DEFERRED_DECAL_SHADER_H_
#define _DEFERRED_DECAL_SHADER_H_

#define float2 vec2
#define float3 vec3
#define float4 vec4

#include "frame.shader"
layout(set = 1, binding = 0) uniform UboFrameContext
{
    FrameContext uboFrameContext;
};

// TODO: Remove this, and use a proper instancing buffer.
layout(push_constant) uniform PC
{
    mat4 g_InvDecalViewProjection;
    mat4 g_DecalViewProjection;
};

//----------------------------------------------------------------------------------------------------
#if defined(VERTEX_SHADER)

layout(location = 0) in float3 inPosition;
layout(location = 0) out float4 outClipPosition;

#define BOUND 0.5f
// JESAS......
float3 vertices[36] = 
{
    {-BOUND,-BOUND,-BOUND}, // -X
    {-BOUND,-BOUND, BOUND},
    {-BOUND, BOUND, BOUND},
    {-BOUND, BOUND, BOUND},
    {-BOUND, BOUND,-BOUND},
    {-BOUND,-BOUND,-BOUND},
    {-BOUND,-BOUND,-BOUND}, // -Z
    { BOUND, BOUND,-BOUND},
    { BOUND,-BOUND,-BOUND},
    {-BOUND,-BOUND,-BOUND},
    {-BOUND, BOUND,-BOUND},
    { BOUND, BOUND,-BOUND},
    {-BOUND,-BOUND,-BOUND}, // -Y
    { BOUND,-BOUND,-BOUND},
    { BOUND,-BOUND, BOUND},
    {-BOUND,-BOUND,-BOUND},
    { BOUND,-BOUND, BOUND},
    {-BOUND,-BOUND, BOUND},
    {-BOUND, BOUND,-BOUND}, // +Y
    {-BOUND, BOUND, BOUND},
    { BOUND, BOUND, BOUND},
    {-BOUND, BOUND,-BOUND},
    { BOUND, BOUND, BOUND},
    { BOUND, BOUND,-BOUND},
    { BOUND, BOUND,-BOUND}, // +X
    { BOUND, BOUND, BOUND},
    { BOUND,-BOUND, BOUND},
    { BOUND,-BOUND, BOUND},
    { BOUND,-BOUND,-BOUND},
    { BOUND, BOUND,-BOUND},
    {-BOUND, BOUND, BOUND}, // +Z
    {-BOUND,-BOUND, BOUND},
    { BOUND, BOUND, BOUND},
    {-BOUND,-BOUND, BOUND},
    { BOUND,-BOUND, BOUND},
    { BOUND, BOUND, BOUND},
};
#undef BOUND

void main()
{
    const float4 worldPosition = g_InvDecalViewProjection * float4(vertices[gl_VertexIndex], 1.0f);
    gl_Position = uboFrameContext.viewProjection * worldPosition;
    outClipPosition = gl_Position;
}

#endif//VERTEX_SHADER

//----------------------------------------------------------------------------------------------------
#if defined(PIXEL_SHADER)

//#define DECAL_SUPPORT_NORMALS

layout(location = 0) in float4 inClipPosition;

#if defined (DECALS_DEFERRED)
layout(location = 0) out float3 outAlbedo;
#if defined(DECAL_SUPPORT_NORMALS)
layout(location = 1) out float3 outNormal;
#endif
// Writing to more render targets? (TODO)
#elif defined (DECALS_FORWARD)
layout(location = 0) out vec4 outColor;
#endif

// TODO: Fix this problem
//layout(set = 2, binding = 0) uniform sampler2D textureDesArr[]; // Bindless textures for decals
// From GBuffer
//layout(binding = 1) uniform sampler2D s_GBufferNormal;
//layout(set = 0, binding = 3) uniform sampler2D s_GBufferDepth;

// TODO: Piggyback the deferred lighting descriptor set for now...
layout(set = 0, binding = 1) uniform sampler2D samplerposition;
layout(set = 0, binding = 2) uniform sampler2D samplerNormal;
layout(set = 0, binding = 3) uniform sampler2D samplerAlbedo;
layout(set = 0, binding = 4) uniform sampler2D samplerMaterial;
layout(set = 0, binding = 5) uniform sampler2D samplerDepth;

// TODO: Fix me
// Per Decal Instance
float3 s_DecalAlbedo = float3(0.0, 1.0, 0.0);
float3 s_DecalNormal = float3(0.5, 0.5, 1.0);
float4 u_DecalOverlayColor = float4(1.0,1.0,1.0,1.0); // Something like tint
float2 u_DecalAspectRatio = float2(1.0, 1.0);

// Not needed for now, since we have gbuffer position as RT...
float3 CalcuilateWorldPositionFromDepth(float2 screen_pos, float ndc_depth)
{
    const float depth = ndc_depth * 2.0 - 1.0; // Remap to [-1.0, 1.0]
    const float4 ndcPosition = float4(screen_pos, depth, 1.0);
    float4 worldPosition = uboFrameContext.inverseViewProjection *ndcPosition;
    worldPosition = worldPosition / worldPosition.w;
    return worldPosition.xyz;
}

void main()
{
    const float2 screenPosition = inClipPosition.xy / inClipPosition.w;
    const float2 texCoords = screenPosition * 0.5f + 0.5f;

#if 0
    // Reconstruct world position from depth
    const float gbufferDepth = texture(s_GBufferDepth, texCoords).x;
    const float3 worldPosition = CalcuilateWorldPositionFromDepth(screenPosition, gbufferDepth);
#else
    // Since we already write world position as a gbuffer RT, we can just use it for now...
    const float3 worldPosition = texture(samplerposition, texCoords).xyz;
#endif

    float4 ndcPosition = g_DecalViewProjection * float4(worldPosition, 1.0f);
    ndcPosition.xyz /= ndcPosition.w;
    ndcPosition.xy *= u_DecalAspectRatio;

    if (ndcPosition.x < -1.0f || ndcPosition.x > 1.0f || ndcPosition.y < -1.0f || ndcPosition.y > 1.0f)
        discard;

    float2 decalTexCoord = ndcPosition.xy * 0.5f + 0.5f;

    float4 albedo = float4(s_DecalAlbedo, 1.0) * u_DecalOverlayColor;
    albedo.xy = decalTexCoord; // For now just color the UV to check for decal correctness.
    albedo.z = 0.0;

    // TODO: Handle alpha clip?
    //if (albedo.a < 0.1)
        //discard;

#if defined (DECALS_DEFERRED)
    outAlbedo.rgb = albedo.rgb;
    // TODO: Writing to other gbuffer render targets
#elif defined (DECALS_FORWARD)
    // TODO: Forward lit/unlit?
    outColor.xyz = albedo.rgb;
#endif
}

#endif//FRAGMENT_SHADER

#endif//INCLUDE_GUARD
