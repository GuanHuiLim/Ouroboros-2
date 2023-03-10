/************************************************************************************//*!
\file           DebugDraw.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines a debug drawing class that allows external engine to interafce with debug drawing

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once
#include "DebugDraw.h"
#include "VulkanRenderer.h"
#include "IcoSphereCreator.h"
#include <cmath>

namespace oGFX {


template<typename T>
void CalculateTangentSpace(const T& normal, T& out0, T& out1)
{
	if (fabsf(normal[2]) > 0.7071067811865475244008443621048490f)
	{
		// out0 in y-z plane
		const float a = normal[1] * normal[1] + normal[2] * normal[2];
        const float k = 1.0f / sqrtf(a);
		out0 = { 0.0f, -normal[2] * k, normal[1] * k };
		// out1 = n x out1
		out1 = { a * k, -normal[0] * out0[2], normal[0] * out0[1] };
	}
	else
	{
		// out0 in x-y plane
		const float a = normal[0] * normal[0] + normal[1] * normal[1];
		const float k = 1.0f / sqrtf(a);
		out0 = { -normal[1] * k, normal[0] * k, 0.0f };
		// out1 = n x out1
		out1 = { -normal[2] * out0[1], normal[2] * out0[0], a * k };
	}
}

static void CalculateHorizonDisc(const glm::vec3& cameraPosition, const glm::vec3& sphereCenter, float sphereRadius, glm::vec3& outPosition, glm::vec3& outNormal)
{
	const float radius = sphereRadius * 0.5f;
	const float d = glm::distance(sphereCenter, cameraPosition);
	const float l = glm::sqrt(d * d - radius * radius);
	outNormal = glm::normalize(cameraPosition - sphereCenter);
	const float h = (radius * l) / d;
	const float s = glm::sqrt(radius * radius - h * h);
	outPosition = sphereCenter + outNormal * s;
}

void DebugDraw::AddLine(const glm::vec3& p0, const glm::vec3& p1, const oGFX::Color& col)
{
    VulkanRenderer* vr = VulkanRenderer::get();
    auto sz = vr->g_DebugDrawVertexBufferCPU.size();
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ p0, col });
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ p1, col });
    vr->g_DebugDrawIndexBufferCPU.emplace_back(0 + static_cast<uint32_t>(sz));
    vr->g_DebugDrawIndexBufferCPU.emplace_back(1 + static_cast<uint32_t>(sz));
}

void DebugDraw::AddAABB(const AABB& aabb, const oGFX::Color& col)
{
    VulkanRenderer* vr = VulkanRenderer::get();

    static std::array<uint32_t, 24> boxindices
    {
        0,1,
        0,2,
        0,3,
        1,4,
        1,5,
        3,5,
        3,6,
        2,6,
        2,4,
        6,7,
        5,7,
        4,7
    };

    auto sz = vr->g_DebugDrawVertexBufferCPU.size();
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{ -aabb.halfExt[0], -aabb.halfExt[1], -aabb.halfExt[2] },col }); //0
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{ -aabb.halfExt[0],  aabb.halfExt[1], -aabb.halfExt[2] },col }); // 1
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{ -aabb.halfExt[0], -aabb.halfExt[1],  aabb.halfExt[2] },col }); // 2
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{  aabb.halfExt[0], -aabb.halfExt[1], -aabb.halfExt[2] },col }); // 3
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{ -aabb.halfExt[0],  aabb.halfExt[1],  aabb.halfExt[2] },col }); // 4
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{  aabb.halfExt[0],  aabb.halfExt[1], -aabb.halfExt[2] },col }); // 5
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{  aabb.halfExt[0], -aabb.halfExt[1],  aabb.halfExt[2] },col }); // 6
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ aabb.center + Point3D{  aabb.halfExt[0],  aabb.halfExt[1],  aabb.halfExt[2] },col }); // 7
    for (auto x : boxindices)
    {
        vr->g_DebugDrawIndexBufferCPU.emplace_back(x + static_cast<uint32_t>(sz));
    }
}

void DebugDraw::AddSphere(const Sphere& sphere, const oGFX::Color& col)
{
    VulkanRenderer* vr = VulkanRenderer::get();
    
    static std::vector<oGFX::Vertex> vertices;
    static std::vector<uint32_t> indices;
    static bool once = [&]() {
        auto [sphVertices, spfIndices] = icosahedron::make_icosphere(1, false);
        vertices.reserve(sphVertices.size());
        for (auto&& v : sphVertices)
        {
            vertices.emplace_back(oGFX::Vertex{ v });
        }
        indices.reserve(spfIndices.size() * 3);
        for (auto&& ind : spfIndices)
        {
            indices.emplace_back(ind.vertex[0]);
            indices.emplace_back(ind.vertex[1]);
            indices.emplace_back(ind.vertex[0]);
            indices.emplace_back(ind.vertex[2]);
            indices.emplace_back(ind.vertex[2]);
            indices.emplace_back(ind.vertex[1]);
        }
        return true;
    }();

    auto currsz = vr->g_DebugDrawVertexBufferCPU.size();
    vr->g_DebugDrawVertexBufferCPU.reserve(vr->g_DebugDrawVertexBufferCPU.size() + vertices.size());
    oGFX::DebugVertex vert;
    for (const auto& v : vertices)
    {
        vert.pos = v.pos * sphere.radius + sphere.center;
        vert.col = col;
        vr->g_DebugDrawVertexBufferCPU.push_back(vert);
    }
    
    vr->g_DebugDrawIndexBufferCPU.reserve(vr->g_DebugDrawIndexBufferCPU.size() + indices.size());
    for (const auto ind : indices)
    {
        vr->g_DebugDrawIndexBufferCPU.emplace_back(ind + static_cast<uint32_t>(currsz));
    }
}

void DebugDraw::AddTriangle(const Triangle& tri, const oGFX::Color& col)
{
    VulkanRenderer* vr = VulkanRenderer::get();

    uint32_t sz = (uint32_t)vr->g_DebugDrawVertexBufferCPU.size();
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ tri.v0, col }); //0
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ tri.v1, col }); //1
    vr->g_DebugDrawVertexBufferCPU.emplace_back(oGFX::DebugVertex{ tri.v2, col }); //2
    
    vr->g_DebugDrawIndexBufferCPU.emplace_back(0 + sz); // E0
    vr->g_DebugDrawIndexBufferCPU.emplace_back(1 + sz); // E0
    vr->g_DebugDrawIndexBufferCPU.emplace_back(1 + sz); // E1
    vr->g_DebugDrawIndexBufferCPU.emplace_back(2 + sz); // E1
    vr->g_DebugDrawIndexBufferCPU.emplace_back(2 + sz); // E2
    vr->g_DebugDrawIndexBufferCPU.emplace_back(0 + sz); // E2
}

void DebugDraw::AddArrow(const glm::vec3& p0, const glm::vec3& p1, const oGFX::Color& col)
{
    auto p0v4 = glm::vec4{ p0 ,1.0f };
    auto p1v4 = glm::vec4{ p1 ,1.0f };

    auto mag = glm::length(p0v4 - p1v4);
    auto revDir = glm::normalize(p0v4 - p1v4);

    //main line
    AddLine(p0v4, p1v4, col);

    {
        static const auto  posRot = glm::rotate(glm::mat4(1.0f),glm::radians( 30.0f),glm::vec3(1.0f,0.0f,0.0f));
        static const auto  negRot = glm::rotate(glm::mat4(1.0f),glm::radians(-30.0f),glm::vec3(1.0f,0.0f,0.0f));

        //head
        AddLine(p1v4, posRot * revDir * (mag / 4.0f) + p1v4, col);
        AddLine(p1v4, negRot * revDir * (mag / 4.0f) + p1v4, col);
    }

    static const auto  posRot = glm::rotate(glm::mat4(1.0f),glm::radians( 30.0f),glm::vec3(0.0f,0.0f,1.0f));
    static const auto  negRot = glm::rotate(glm::mat4(1.0f),glm::radians(-30.0f),glm::vec3(0.0f,0.0f,1.0f));
    //head
    AddLine(p1v4, posRot * revDir * (mag / 4.0f) + p1v4, col);
    AddLine(p1v4, negRot * revDir * (mag / 4.0f) + p1v4, col);

}

void DebugDraw::DrawCameraFrustrum(const Camera& camera, const oGFX::Color& col)
{
    auto m_forward = camera.GetFront();
    auto m_up = camera.GetUp();
    auto m_right= camera.GetRight();

    auto fov = camera.GetFov();
    auto ar = camera.GetAspectRatio();
    auto znear = camera.GetNearClip();
    auto zfar = camera.GetFarClip();

    glm::vec3 near_center = camera.m_position + m_forward * znear;
    glm::vec3 far_center = camera.m_position + m_forward * zfar;

    float near_half_height = tan(glm::radians(fov )/ 2) * znear;
    float near_half_width = near_half_height * ar;
    float far_half_height = tan(glm::radians(fov )/ 2) * zfar;
    float far_half_width = far_half_height * ar;

    glm::vec3 ntl = near_center + m_up * near_half_height - m_right * near_half_width;
    glm::vec3 ntr = near_center + m_up * near_half_height + m_right * near_half_width;
    glm::vec3 nbl = near_center - m_up * near_half_height - m_right * near_half_width;
    glm::vec3 nbr = near_center - m_up * near_half_height + m_right * near_half_width;
    glm::vec3 ftl = far_center  + m_up * far_half_height  - m_right * far_half_width;
    glm::vec3 ftr = far_center  + m_up * far_half_height  + m_right * far_half_width;
    glm::vec3 fbl = far_center  - m_up * far_half_height  - m_right * far_half_width;
    glm::vec3 fbr = far_center  - m_up * far_half_height  + m_right * far_half_width;

    // near plane
    AddLine(ntl,ntr, col); // top
    AddLine(ntr,nbr, oGFX::Colors::BLUE); // right
    AddLine(ntl,nbl, oGFX::Colors::RED); // left
    AddLine(nbl,nbr, col); // bottom

    // far plane
    AddLine(ftl,ftr, col); // top
    AddLine(ftr,fbr, col); // right
    AddLine(ftl,fbl, col); // left
    AddLine(fbl,fbr, col); // bottom

                              // connectors
    AddLine(ftr,ntr,col); // TR
    AddLine(ftl,ntl,col); // TL
    AddLine(fbl,nbl,col); // BL
    AddLine(fbr,nbr,col); // BR

    return;

    DrawCameraFrustrum(camera.m_position, camera.matrices.view, camera.GetAspectRatio() , camera.GetFov(), camera.GetNearClip(), camera.GetFarClip(), col);
}

void DebugDraw::DrawCameraFrustrum(const glm::vec3& position, const glm::mat4& view, float ar, float fov,float znear, float zfar,const oGFX::Color& col)
{
    mat4 inv = glm::inverse(view);

    float halfHeight = tanf(glm::radians(fov / 2.f));
    float halfWidth = halfHeight * ar;

    float xn = halfWidth * znear + EPSILON;
    float xf = halfWidth * zfar + EPSILON;
    float yn = halfHeight * znear + EPSILON;
    float yf = halfHeight * zfar + EPSILON;

    glm::vec4 f[8u] =
    {
        // znear face
        {xn, yn,  -znear, 1.f},
        {-xn, yn, -znear, 1.f},
        {xn, -yn, -znear, 1.f},
        {-xn, -yn,-znear , 1.f},

        // zfar face
        {xf, yf, -zfar, 1.f},
        {-xf, yf,-zfar , 1.f},
        {xf, -yf,-zfar , 1.f},
        {-xf, -yf,-zfar, 1.f},
    };

    glm::vec3 v[8];
    for (int i = 0; i < 8; i++)
    {
        glm::vec4 ff = inv * f[i];
        v[i].x = ff.x / ff.w;
        v[i].y = ff.y / ff.w;
        v[i].z = ff.z / ff.w;
    }

  


    // near plane
    AddLine(v[0], v[1], col); // top
    AddLine(v[0], v[2], col); // right
    AddLine(v[3], v[1], col); // left
    AddLine(v[3], v[2], col); // bottom

    // far plane
    AddLine(v[4], v[5], col); // top
    AddLine(v[4], v[6], col); // right
    AddLine(v[7], v[5], col); // left
    AddLine(v[7], v[6], col); // bottom
    
    // connectors
    AddLine(v[0], v[4],col); // TR
    AddLine(v[1], v[5],col); // TL
    AddLine(v[3], v[7],col); // BL
    AddLine(v[2], v[6],col); // BR
}

void DebugDraw::DrawCameraFrustrumDebugArrows(const Camera& c, const oGFX::Color& col)
{
    auto frust = c.GetFrustum();
    DrawCameraFrustrumDebugArrows(frust, col);

}

void DebugDraw::DrawCameraFrustrumDebugArrows(const Frustum& frust, const oGFX::Color& col)
{
    const float arrowLen = 5.0f;
    {
        glm::vec3 start = glm::vec3(frust.top.normal)* frust.top.normal.w;
        glm::vec3 end = start + glm::vec3(frust.top.normal) * arrowLen;
        oGFX::DebugDraw::AddArrow(start,end , col);
    }

    {
        glm::vec3 start = glm::vec3(frust.bottom.normal)* frust.bottom.normal.w;
        glm::vec3 end = start + glm::vec3(frust.bottom.normal) * arrowLen;
        oGFX::DebugDraw::AddArrow(start,end , col);
    }

    {
        glm::vec3 start = glm::vec3(frust.right.normal)* frust.right.normal.w;
        glm::vec3 end = start + glm::vec3(frust.right.normal) * arrowLen;
        oGFX::DebugDraw::AddArrow(start,end , col);
    }

    {
        glm::vec3 start = glm::vec3(frust.left.normal) * frust.left.normal.w;
        glm::vec3 end = start + glm::vec3(frust.left.normal) * arrowLen;
        oGFX::DebugDraw::AddArrow(start, end, col); 
    }

    {
        glm::vec3 start = glm::vec3(frust.planeFar.normal)* frust.planeFar.normal.w;
        glm::vec3 end = start + glm::vec3(frust.planeFar.normal) * arrowLen;
        oGFX::DebugDraw::AddArrow(start,end , col);
    }

    {
        glm::vec3 start = glm::vec3(frust.planeNear.normal)* frust.planeNear.normal.w;
        glm::vec3 end = start + glm::vec3(frust.planeNear.normal) * arrowLen;
        oGFX::DebugDraw::AddArrow(start,end , col);
    }

}

void DebugDraw::AddDisc(const glm::vec3& center, float radius, const glm::vec3& basis0, const glm::vec3& basis1, const oGFX::Color& color)
{
    constexpr unsigned k_segments = 32;
    constexpr float k_increment = 2.0f * 3.1415f / (float)k_segments;
    static const std::array<glm::vec2, 32> s_UnitCircleVertices =
    {
        glm::vec2{ sinf( 0 * k_increment), cosf( 0 * k_increment)},
        glm::vec2{ sinf( 1 * k_increment), cosf( 1 * k_increment)},
        glm::vec2{ sinf( 2 * k_increment), cosf( 2 * k_increment)},
        glm::vec2{ sinf( 3 * k_increment), cosf( 3 * k_increment)},
        glm::vec2{ sinf( 4 * k_increment), cosf( 4 * k_increment)},
        glm::vec2{ sinf( 5 * k_increment), cosf( 5 * k_increment)},
        glm::vec2{ sinf( 6 * k_increment), cosf( 6 * k_increment)},
        glm::vec2{ sinf( 7 * k_increment), cosf( 7 * k_increment)},
        glm::vec2{ sinf( 8 * k_increment), cosf( 8 * k_increment)},
        glm::vec2{ sinf( 9 * k_increment), cosf( 9 * k_increment)},
        glm::vec2{ sinf(10 * k_increment), cosf(10 * k_increment)},
        glm::vec2{ sinf(11 * k_increment), cosf(11 * k_increment)},
        glm::vec2{ sinf(12 * k_increment), cosf(12 * k_increment)},
        glm::vec2{ sinf(13 * k_increment), cosf(13 * k_increment)},
        glm::vec2{ sinf(14 * k_increment), cosf(14 * k_increment)},
        glm::vec2{ sinf(15 * k_increment), cosf(15 * k_increment)},
        glm::vec2{ sinf(16 * k_increment), cosf(16 * k_increment)},
        glm::vec2{ sinf(17 * k_increment), cosf(17 * k_increment)},
        glm::vec2{ sinf(18 * k_increment), cosf(18 * k_increment)},
        glm::vec2{ sinf(19 * k_increment), cosf(19 * k_increment)},
        glm::vec2{ sinf(20 * k_increment), cosf(20 * k_increment)},
        glm::vec2{ sinf(21 * k_increment), cosf(21 * k_increment)},
        glm::vec2{ sinf(22 * k_increment), cosf(22 * k_increment)},
        glm::vec2{ sinf(23 * k_increment), cosf(23 * k_increment)},
        glm::vec2{ sinf(24 * k_increment), cosf(24 * k_increment)},
        glm::vec2{ sinf(25 * k_increment), cosf(25 * k_increment)},
        glm::vec2{ sinf(26 * k_increment), cosf(26 * k_increment)},
        glm::vec2{ sinf(27 * k_increment), cosf(27 * k_increment)},
        glm::vec2{ sinf(28 * k_increment), cosf(28 * k_increment)},
        glm::vec2{ sinf(29 * k_increment), cosf(29 * k_increment)},
        glm::vec2{ sinf(30 * k_increment), cosf(30 * k_increment)},
        glm::vec2{ sinf(31 * k_increment), cosf(31 * k_increment)}
    };

    const glm::vec3 b0 = glm::normalize(basis0) * radius;
    const glm::vec3 b1 = glm::normalize(basis1) * radius;

    for (size_t i = 0; i < k_segments - 1; ++i)
    {
        DebugDraw::AddLine(center + s_UnitCircleVertices[i       ].x * b0 + s_UnitCircleVertices[i       ].y * b1,
                           center + s_UnitCircleVertices[i + 1ull].x * b0 + s_UnitCircleVertices[i + 1ull].y * b1,
                           color);
    }
    DebugDraw::AddLine(center + s_UnitCircleVertices[k_segments - 1ull].x * b0 + s_UnitCircleVertices[k_segments - 1ull].y * b1,
		               center + s_UnitCircleVertices[0                ].x * b0 + s_UnitCircleVertices[0                ].y * b1,
		               color);
}

void DebugDraw::AddDisc(const glm::vec3& center, float radius, const glm::vec3& normal, const oGFX::Color& color)
{
    glm::vec3 basis0;
	glm::vec3 basis1;
    CalculateTangentSpace(normal, basis0, basis1);
    DebugDraw::AddDisc(center, radius, basis0, basis1, color);
}

void DebugDraw::AddSphereAs3Disc1HorizonDisc(const glm::vec3& center, float radius, const glm::vec3& cameraPosition, const oGFX::Color& color)
{
    DebugDraw::AddDisc(center, radius, { 1.0f, 0.0f, 0.0f }, color);
    DebugDraw::AddDisc(center, radius, { 0.0f, 1.0f, 0.0f }, color);
    DebugDraw::AddDisc(center, radius, { 0.0f, 0.0f, 1.0f }, color);
    glm::vec3 position;
    glm::vec3 normal;
    CalculateHorizonDisc(cameraPosition, center, radius, position, normal);
    DebugDraw::AddDisc(position, radius, normal, color);
}

void DebugDraw::DrawYGrid(float gridSize, float gapSize, const oGFX::Color & col)
{
    const float halfGrid = gridSize/2.0f;
    const float numLines = gridSize / gapSize;

    const Point3D bottomLeft{ -halfGrid,0.0f,-halfGrid };
    const Point3D topRight{ halfGrid ,0.0f, halfGrid };

    const auto iters = numLines;
    for (size_t x = 0; x < iters; x++)
    {
        DebugDraw::AddLine(bottomLeft + Point3D{ gapSize * x,0.0f,0.0f }, topRight - Point3D{ gapSize * (iters - x),0.0f,0.0f }, oGFX::Colors::LIGHT_GREY);
    }
    for (size_t z = 0; z< iters; z++)
    {
        DebugDraw::AddLine(bottomLeft + Point3D{ 0.0f,0.0f, gapSize*z }, topRight - Point3D{ 0.0f,0.0f ,gapSize * (iters - z)}, oGFX::Colors::LIGHT_GREY);
    }
    DebugDraw::AddLine(bottomLeft + Point3D{ gridSize ,0.0f,0.0f }, topRight , oGFX::Colors::LIGHT_GREY);
    DebugDraw::AddLine(bottomLeft + Point3D{ 0.0f,0.0f, gridSize }, topRight , oGFX::Colors::LIGHT_GREY);

}

} // end namesace oGFX