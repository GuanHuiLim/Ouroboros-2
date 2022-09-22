#pragma once
#include "DebugDraw.h"
#include "VulkanRenderer.h"
#include "IcoSphereCreator.h"
#include <cmath>

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
        auto [sphVertices, spfIndices] = icosahedron::make_icosphere(2, false);
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
        vert.pos = vert.pos * sphere.radius + sphere.center;
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

void DebugDraw::AddDisc(const glm::vec3& center, float radius, const glm::vec3& basis0, const glm::vec3& basis1, const oGFX::Color& color)
{
    constexpr unsigned k_segments = 32;
    constexpr float k_increment = 2.0f * 3.1415f / (float)k_segments;
    static const std::array<glm::vec2, 32> s_UnitCircleVertices =
    {
        glm::vec2{ sinf(0 * k_increment), cosf(0 * k_increment)},
        glm::vec2{ sinf(1 * k_increment), cosf(1 * k_increment)},
        glm::vec2{ sinf(2 * k_increment), cosf(2 * k_increment)},
        glm::vec2{ sinf(3 * k_increment), cosf(3 * k_increment)},
        glm::vec2{ sinf(4 * k_increment), cosf(4 * k_increment)},
        glm::vec2{ sinf(5 * k_increment), cosf(5 * k_increment)},
        glm::vec2{ sinf(6 * k_increment), cosf(6 * k_increment)},
        glm::vec2{ sinf(7 * k_increment), cosf(7 * k_increment)},
        glm::vec2{ sinf(8 * k_increment), cosf(8 * k_increment)},
        glm::vec2{ sinf(9 * k_increment), cosf(9 * k_increment)},
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

    for (int i = 0; i < k_segments - 1; ++i)
    {
        DebugDraw::AddLine(center + s_UnitCircleVertices[i    ].x * b0 + s_UnitCircleVertices[i    ].y * b1,
                           center + s_UnitCircleVertices[i + 1].x * b0 + s_UnitCircleVertices[i + 1].y * b1,
                           color);
    }
    DebugDraw::AddLine(center + s_UnitCircleVertices[k_segments - 1].x * b0 + s_UnitCircleVertices[k_segments - 1].y * b1,
		               center + s_UnitCircleVertices[0             ].x * b0 + s_UnitCircleVertices[0             ].y * b1,
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
