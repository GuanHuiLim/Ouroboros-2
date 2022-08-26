#pragma once

#include "VulkanUtils.h"

#include <vector>
#include <cstdint>

// Just a dummy struct to hold Vertex and 32-bit Index Buffers.
struct DefaultMesh
{
    std::vector<oGFX::Vertex> m_VertexBuffer;
    std::vector<uint32_t> m_IndexBuffer;
};

DefaultMesh CreateDefaultCubeMesh();

// We must be explicit by saying this is a XZ-plane, because a plane can mean anything, is it XY, XZ, or YZ...?
DefaultMesh CreateDefaultPlaneXZMesh();
