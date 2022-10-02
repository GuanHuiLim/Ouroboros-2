#pragma once
#include "Ouroboros/Asset/Asset.h"
#include "OO_Vulkan/src/MeshModel.h"
#include <bitset>
struct MeshInfo
{
	oo::Asset mesh_handle;
	std::bitset<MAX_SUBMESH> submeshBits;
};