/************************************************************************************//*!
\file           DefaultMeshCreator.h
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              helper to Creates default meshes

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
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

DefaultMesh CreateDefaultPlaneXYMesh();
